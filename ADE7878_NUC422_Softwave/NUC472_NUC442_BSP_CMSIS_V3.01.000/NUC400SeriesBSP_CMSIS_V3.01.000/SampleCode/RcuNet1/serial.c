#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "function.h"
#include "btrcu.h"
#include "NUC472_442.h"

#define SERIAL_THREAD_PRIO    ( tskIDLE_PRIORITY + 2UL )
#define SERIAL_THREAD_STACKSIZE  300

// External variable
//defined in main.c
extern uint8_t g_u8SendData[TXBUFSIZE];
extern uint8_t g_u8RecvData[RXBUFSIZE];
extern volatile uint32_t g_u32comRhead;
extern volatile uint32_t g_u32comRtail;
extern volatile uint32_t g_u32comShead;
extern volatile uint32_t g_u32comStail;

extern uint8_t g_u8SerialBuf[RXBUFSIZE];
extern volatile uint16_t g_u16SerialLen;

// defined in server.c
extern struct netconn *connServer;
extern volatile uint8_t servConnected;

// defined in cfgtool.c
extern uint8_t sys_Settings[OPT_MAX_LEN];
extern struct netconn *connTool;
extern ip_addr_t RcuToolAddr;
extern unsigned short RcuToolPort;

// External functions
void ConnectToServer(void);

void WDT_IRQHandler(void)
{
    // Clear WDT interrupt flag
    WDT_CLEAR_TIMEOUT_INT_FLAG();
	
	// kick dog
	SYS_UnlockReg();	
	WDT_RESET_COUNTER();
	SYS_LockReg();
}

void SerialSend(char *buf, u16_t len)
{
	u16_t i;
	
	for (i = 0; i < len; i ++)
	{
		if (g_u32comShead != g_u32comStail)
		{
			g_u8SendData[g_u32comShead] = buf[i];
			g_u32comShead ++;
			if (g_u32comShead == TXBUFSIZE)
			{
				g_u32comShead = 0;
			}
		}
	}
}

void _SerialSaveRecv(uint8_t iVal)
{
	if (g_u16SerialLen < RXBUFSIZE-1)
	{
		g_u8SerialBuf[g_u16SerialLen] = iVal;
		g_u16SerialLen ++;
	}
}

void _InputNetVersion(uint8_t *pBuf, uint16_t sLen)
{
	uint8_t *pCur;

	pCur = pBuf + sLen - 4;
	pCur[0] = SOFT_VER_HIGH+'0';
	pCur[1] = '.';
	pCur[2] = SOFT_VER_LOW+'0';
	pCur[3] = SOFT_VER_BUILD+'0';
	pCur[4] = '.';
	pCur[5] = ENG_BUILD_HIGH+'0';
	pCur[6] = ENG_BUILD_MID+'0';
	pCur[7] = ENG_BUILD_LOW+'0';
}

uint8_t _InputNetConfig(uint8_t *pBuf, uint16_t sLen)
{
	uint8_t *pCur;
	uint8_t cLen;

	pCur = pBuf + sLen - 4 + 8;
	cLen = strlen((char*)(sys_Settings+OPT_ROOM_NUMBER));

	memcpy(pCur, (sys_Settings+OPT_IP_ADDRESS), 4);
	memcpy(pCur+4, (sys_Settings+OPT_MAC_ADDRESS), 6);
	pCur[10] = sys_Settings[OPT_IP_TYPE];
	memcpy(pCur+11, (sys_Settings+OPT_SUBNET_MASK), 4);
	memcpy(pCur+15, (sys_Settings+OPT_ROUTER_IP), 4);
	memcpy(pCur+19, (sys_Settings+OPT_SERVER_IP), 4);
	if (cLen != 0)
	{
		memcpy(pCur+23, (sys_Settings+OPT_ROOM_NUMBER), cLen);
	}
	pCur[23+cLen] = 0;			//CheckSum 
	pCur[24+cLen] = 0;		
	pCur[25+cLen] = 0x0d;
	pCur[26+cLen] = 0x0a;	

	return (cLen + 27);
}

void _RcuToolSendConnect(uint8_t *pBuf, uint16_t sLen)
{
	uint8_t cLen;
	uint16_t sTotalLen;
	struct netbuf *outbuf;
	char *data;
	
	_InputNetVersion(pBuf, sLen);
	cLen = _InputNetConfig(pBuf, sLen);
	pBuf[2] = sLen - 4 + 8 + cLen - 7; 
	sTotalLen = sLen - 4 + 8 + cLen;

	outbuf = netbuf_new();
    data = netbuf_alloc(outbuf, sTotalLen); 
    memcpy(data, pBuf, sTotalLen);
	
	netconn_sendto(connTool, outbuf, &RcuToolAddr, RcuToolPort);

	netbuf_delete(outbuf);
}

void HandleSerialData(uint8_t *buf, uint16_t len)
{
	uint8_t iVal, iNextVal;
	uint8_t bBrokenLine;
	uint8_t i;

	while (len)
	{
		bBrokenLine = FALSE;
		if (g_u16SerialLen && g_u8SerialBuf[g_u16SerialLen-1] == 0x0d)
		{
			bBrokenLine = TRUE;
		}
		
		iVal = *buf;
		buf ++;
		len --;
		if (iVal == 0x0d)
		{
			if (len)
			{
				iNextVal = *buf;
				buf ++;
				len --;
				_SerialSaveRecv(iVal);
				_SerialSaveRecv(iNextVal);
				if (iNextVal == 0x0a)
				{
/*					printf("\r\nRecv serial data!\r\n");
					for (i = 0; i < g_u16SerialLen; i ++)
					{
						printf("0x%x ", g_u8SerialBuf[i]);
					}
					netconn_write(connServer, g_u8SerialBuf, g_u16SerialLen, NETCONN_NOCOPY);  
*/					if ((g_u8SerialBuf[0] > 0xc0 && g_u8SerialBuf[0] < 0xd0) ||  g_u8SerialBuf[0] == SCENE_ACK || g_u8SerialBuf[0] == SEQUENCE_ACK)
					{
						if (g_u8SerialBuf[0] == TOOL_CONNECT)
						{
							_RcuToolSendConnect(g_u8SerialBuf, g_u16SerialLen);						
						}
						else
						{
							RcuToolSend(g_u8SerialBuf, g_u16SerialLen);						
						}
					}
					
					if (g_u8SerialBuf[0] > 0x80 && g_u8SerialBuf[0] < 0xa0)
					{
						if (servConnected)
						{
							netconn_write(connServer, g_u8SerialBuf, g_u16SerialLen, NETCONN_NOCOPY);  
						}
						else
						{
//							ConnectToServer();
						}
					}					
					g_u16SerialLen = 0;
			}
			continue;
			}
		}
		else if (iVal == 0x0a)
		{
			if (bBrokenLine)
			{
				_SerialSaveRecv(iVal);
/*				printf("\r\nRecv serial data!\r\n");
				for (i = 0; i < g_u16SerialLen; i ++)
				{
					printf("0x%x ", g_u8SerialBuf[i]);
				}
				netconn_write(connServer, g_u8SerialBuf, g_u16SerialLen, NETCONN_NOCOPY);  
*/				if ((g_u8SerialBuf[0] > 0xc0 && g_u8SerialBuf[0] < 0xd0) ||  g_u8SerialBuf[0] == SCENE_ACK || g_u8SerialBuf[0] == SEQUENCE_ACK)
				{
					if (g_u8SerialBuf[0] == TOOL_CONNECT)
					{
						_RcuToolSendConnect(g_u8SerialBuf, g_u16SerialLen);						
					}
					else
					{
						RcuToolSend(g_u8SerialBuf, g_u16SerialLen);						
					}
				}
				
				if (g_u8SerialBuf[0] > 0x80 && g_u8SerialBuf[0] < 0xa0)
				{
					if (servConnected)
					{
						netconn_write(connServer, g_u8SerialBuf, g_u16SerialLen, NETCONN_NOCOPY);  
					}
					else
					{
//						ConnectToServer();
					}
				}
				g_u16SerialLen = 0;
				bBrokenLine = FALSE;
				continue;
			}
		}
		_SerialSaveRecv(iVal);
	}
}

void SerialSendData()
{
	uint32_t tail;

	tail = g_u32comStail + 1;
	if (tail == TXBUFSIZE)
	{
		tail = 0;
	}

	while(tail != g_u32comShead)
	{
		if (UART_Write(UART0, &(g_u8SendData[tail]), 1))
		{
			g_u32comStail = tail;
			tail ++;
			if (tail == TXBUFSIZE)
			{
				tail = 0;
			}
		}
		else
		{
			break;
		}
	}
}

void SerialRun( void )
{
	uint16_t tail, len;
	uint8_t	buf[RXBUFSIZE];
	
	SerialSendData();
	
	len = 0;
	NVIC_DisableIRQ(UART0_IRQn);	

	tail = g_u32comRtail + 1;
	if (tail == RXBUFSIZE)
	{
		tail = 0;
	}
	
	while (tail != g_u32comRhead)
	{
		buf[len] = g_u8RecvData[tail];
		len ++;
		g_u32comRtail = tail;
		tail ++;
		if (tail == RXBUFSIZE)
		{
			tail = 0;
		}
	}

	NVIC_EnableIRQ(UART0_IRQn);	
	if (len != 0)
	{
		HandleSerialData(buf, len);
	}
	SerialSendData();
}

void serial_thread(void *arg)
{
	// Enable Watchdog
    SYS_UnlockReg();	
    WDT_Open(WDT_TIMEOUT_2POW14, WDT_RESET_DELAY_130CLK, TRUE, FALSE);
//    WDT_EnableInt();
    SYS_LockReg();
	
	while (1)
	{
		sys_msleep(100);
		// kick dog
		SYS_UnlockReg();	
		WDT_RESET_COUNTER();
		SYS_LockReg();
	
		SerialRun();
	}
}

void serial_init()
{
    sys_thread_new("SERIAL", serial_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);	
/*	while (1)
	{
		sys_msleep(10);
		SerialRun();
	}
*/	
}
