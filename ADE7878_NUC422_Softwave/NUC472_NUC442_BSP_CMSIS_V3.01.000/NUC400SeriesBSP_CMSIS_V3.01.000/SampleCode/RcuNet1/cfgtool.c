#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "function.h"
#include "btrcu.h"
#include "NUC472_442.h"

#define CFGTOOL_THREAD_PRIO    ( tskIDLE_PRIORITY + 2UL )
#define CFGTOOL_THREAD_STACKSIZE  300

uint8_t gNetSendBuf[50];
uint16_t gNetSendLength;
uint32_t gReadyToSend;

// External function
void SerialSend(char *buf, u16_t len);

// External variable

// Golobal variable
struct netconn *connTool;
uint8_t sys_Settings[OPT_MAX_LEN];
ip_addr_t RcuToolAddr;
unsigned short RcuToolPort;

void RcuToolSend(uint8_t *pBuf, uint16_t sLen)
{
	struct netbuf *outbuf;
	char *data;

	if (RcuToolPort == 0)	return;
	
	outbuf = netbuf_new();
    data = netbuf_alloc(outbuf, sLen); 
    memcpy(data, pBuf, sLen);

	netconn_sendto(connTool, outbuf, &RcuToolAddr, RcuToolPort);

	netbuf_delete(outbuf);
}

void DefaultSettings(void)
{
	u8_t i;
	
	sys_Settings[OPT_IP_ADDRESS] = 192;
	sys_Settings[OPT_IP_ADDRESS+1] = 168;
	sys_Settings[OPT_IP_ADDRESS+2] = 0;
	sys_Settings[OPT_IP_ADDRESS+3] = 3;
	
	sys_Settings[OPT_SUBNET_MASK] = 255;
	sys_Settings[OPT_SUBNET_MASK+1] = 255;
	sys_Settings[OPT_SUBNET_MASK+2] = 255;
	sys_Settings[OPT_SUBNET_MASK+3] = 0;
	
	sys_Settings[OPT_ROUTER_IP] = 192;
	sys_Settings[OPT_ROUTER_IP+1] = 168;
	sys_Settings[OPT_ROUTER_IP+2] = 0;
	sys_Settings[OPT_ROUTER_IP+3] = 1;
	
	sys_Settings[OPT_SERVER_IP] = 192;
	sys_Settings[OPT_SERVER_IP+1] = 168;
	sys_Settings[OPT_SERVER_IP+2] = 0;
	sys_Settings[OPT_SERVER_IP+3] = 103;
	
	for (i = 0; i < 16; i ++)
	{
		sys_Settings[OPT_ROOM_NUMBER+i] = 0;
	}
	sys_Settings[OPT_ROOM_NUMBER] = '1';
	sys_Settings[OPT_ROOM_NUMBER+1] = '0';
	sys_Settings[OPT_ROOM_NUMBER+2] = '1';
	sys_Settings[OPT_ROOM_NUMBER+3] = '8';	

	sys_Settings[OPT_IP_TYPE] = 0; // 0 ~ static ip, 1 ~ dhcp
	
	sys_Settings[OPT_MAC_ADDRESS] = 0x0;
	sys_Settings[OPT_MAC_ADDRESS+1] = 0x0;
	sys_Settings[OPT_MAC_ADDRESS+2] = 0x0;
	sys_Settings[OPT_MAC_ADDRESS+3] = 0x11;
	sys_Settings[OPT_MAC_ADDRESS+4] = 0x22;
	sys_Settings[OPT_MAC_ADDRESS+5] = 0x33;	
}

void LoadSysSettings(void)
{
	u8_t i;
	
	/* Enable FMC ISP function */
    SYS_UnlockReg();
    FMC_Open();

	for (i = 0; i < OPT_MAX_LEN; i += 4)
	{
		*((uint32_t*)(sys_Settings+i)) = FMC_Read((uint32_t)(DFLASH_BASE + OPT_BASE + i));
	}
	
    /* Disable FMC ISP function */
    FMC_Close();
    SYS_LockReg();	
}

void SaveSysSettings(void)
{
	u8_t i;
	
	/* Enable FMC ISP function */
    SYS_UnlockReg();
    FMC_Open();

	FMC_Erase(DFLASH_BASE);
	
	for (i = 0; i < OPT_MAX_LEN; i += 4)
	{
		FMC_Write((uint32_t)(DFLASH_BASE + OPT_BASE + i), *((uint32_t*)(sys_Settings+i)));
	}
	
    /* Disable FMC ISP function */
    FMC_Close();
    SYS_LockReg();	
}

void HandleToolData(char *buf, u16_t len)
{
	u8_t l, clen;
	char *data;
	
	if (buf[0] == TOOL_SET_CONFIG)
	{
		// Write Config to Network Module
		data = buf;
		data += 3;
		memcpy(sys_Settings+OPT_IP_ADDRESS, data, 4);				
		data += 4;
		l = *data;
		data ++;
		memset(sys_Settings+OPT_ROOM_NUMBER, 0, 16);
		if (l != 0)
		{
			memcpy(sys_Settings+OPT_ROOM_NUMBER, data, l);
		}
		data += l;
		sys_Settings[OPT_IP_TYPE] = *data;
		data ++;
		memcpy(sys_Settings + OPT_SUBNET_MASK, data, 4);				
		data += 4;
		memcpy(sys_Settings + OPT_ROUTER_IP, data, 4);				
		data += 4;
		memcpy(sys_Settings + OPT_SERVER_IP, data, 4);				
		SaveSysSettings();
		
		// Write Config to RCU Module
		data = buf;
		data += 7;		// 3+4
		l = *data;
		data ++;
		if (l == 0)
		{
			memcpy(data, "118", 3);
			l = 3;
		}
		buf[2] = l+5;
		buf[7] = l;
		buf[l+8] = 0;
		buf[l+9] = 0;
		buf[l+10] = 0x0d;
		buf[l+11] = 0x0a;
		SerialSend(buf, l+12);	
	}
	else if (buf[0] == TOOL_RCU_FIND)
	{
		clen = strlen((char*)(sys_Settings+OPT_ROOM_NUMBER));
		gNetSendBuf[0] = TOOL_CONNECT;
		gNetSendBuf[1] = 0;			//Length
		gNetSendBuf[2] = 23 + clen;
		memcpy(gNetSendBuf+3, sys_Settings+OPT_IP_ADDRESS, 4);
		memcpy(gNetSendBuf+7, sys_Settings+OPT_MAC_ADDRESS, 6);
		gNetSendBuf[13] = sys_Settings[OPT_IP_TYPE];
		memcpy(gNetSendBuf+14, sys_Settings+OPT_SUBNET_MASK, 4);
		memcpy(gNetSendBuf+18, sys_Settings+OPT_ROUTER_IP, 4);
		memcpy(gNetSendBuf+22, sys_Settings+OPT_SERVER_IP, 4);
		if (clen != 0)
		{
			memcpy(gNetSendBuf+26, sys_Settings+OPT_ROOM_NUMBER, clen);
		}
		gNetSendBuf[26+clen] = 0;			//CheckSum 
		gNetSendBuf[27+clen] = 0;		
		gNetSendBuf[28+clen] = 0x0d;
		gNetSendBuf[29+clen] = 0x0a;	
		gNetSendLength = 30 + clen;
		gReadyToSend = 1;
	}
	else
	{
		SerialSend(buf, len);
	}
}

static void cfgtool_thread(void *arg)
{
	err_t err;
    struct netbuf *inbuf;
    char* buf;
    u16_t buflen;
	
	gReadyToSend = 0;
	RcuToolPort = 0;
	
    /* Create a new TCP connection handle */
    connTool = netconn_new(NETCONN_UDP);
    if (connTool == NULL) 
	{
        printf("can not create netconn");
	}
	
	/* Bind to port 21221 with default IP address */
	err = netconn_bind(connTool, NULL, CFGTOOL_LOCAL_PORT);
	if (err != ERR_OK) 
	{
		printf("can not bind netconn");
	}
	
	while(1) 
	{
		if (gReadyToSend == 1)
		{
			gReadyToSend = 0;
			RcuToolSend(gNetSendBuf, gNetSendLength);
		}
		
		netconn_recv(connTool, &inbuf);

		/* Get RCUTool ip address and port*/
		RcuToolAddr = *(netbuf_fromaddr(inbuf));
		RcuToolPort = netbuf_fromport(inbuf);            
            			
		if (inbuf == NULL) 
		{
			printf("NULL data received\r\n");
			return;
		}
			
		if (netconn_err(connTool) == ERR_OK) 
		{
			netbuf_data(inbuf, (void**)&buf, &buflen);
			HandleToolData(buf, buflen);		
		    netbuf_delete(inbuf);	
		}
    }
}

void cfgtool_init()
{
    sys_thread_new("CFGTOOL", cfgtool_thread, NULL, CFGTOOL_THREAD_STACKSIZE, CFGTOOL_THREAD_PRIO);
}
