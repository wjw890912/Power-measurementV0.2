#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "function.h"
#include "btrcu.h"

#define RCUSERVER_THREAD_PRIO    ( tskIDLE_PRIORITY + 2UL )
#define RCUSERVER_THREAD_STACKSIZE  300

// External function
void SerialSend(char *buf, u16_t len);
// External variable
extern uint8_t sys_Settings[OPT_MAX_LEN];

// Golobal variable
struct netconn *connServer;
volatile uint8_t servConnected = 0;

void ConnectToServer(void)
{
	err_t err;

	/* Create a new TCP connection handle */
    connServer = netconn_new(NETCONN_TCP);
    if (connServer == NULL) 
	{
        printf("can not create netconn");
		return;
	}
	
	/* Bind to port 21221 with default IP address */
	err = netconn_bind(connServer, NULL, RCU_LOCAL_PORT);
	if (err != ERR_OK) 
	{
		printf("can not bind netconn");
		return;
	}
	
	err = netconn_connect(connServer, (ip_addr_t*)(sys_Settings+OPT_SERVER_IP), SERVER_LISTEN_PORT);
	if (err != ERR_OK)
	{
		printf("connect to server error\r\n");
		servConnected = 0;
	}
	else
	{
		servConnected = 1;
	}
}

void HandleServerData(char *buf, u16_t len)
{
	SerialSend(buf, len);
}

static void server_thread(void *arg)
{
    struct netbuf *inbuf;
    char* buf;
    u16_t buflen;
	
	ConnectToServer();

	while(1) 
	{
		if (servConnected)
		{
			netconn_recv(connServer, &inbuf);

/*			if (inbuf == NULL) 
			{
				printf("NULL data received\r\n");
			}
*/			
			if (netconn_err(connServer) == ERR_OK) 
			{
				netbuf_data(inbuf, (void**)&buf, &buflen);
				HandleServerData(buf, buflen);		
				netbuf_delete(inbuf);	
			}
		}
		else
		{
			sys_msleep(10);
		}
			
    }
}

/**
  * @brief  Initialize the HTTP server (start its thread)
  * @param  none
  * @retval None
  */
void server_init()
{
    sys_thread_new("SERVER", server_thread, NULL, RCUSERVER_THREAD_STACKSIZE, RCUSERVER_THREAD_PRIO);
}

