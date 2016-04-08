#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "function.h"
#include "btrcu.h"


#define APP_THREAD_PRIO    ( tskIDLE_PRIORITY + 2UL )
#define APP_THREAD_STACKSIZE  200

struct netconn *connApp;
ip_addr_t AppAddr;
unsigned short AppPort;

void HandleAppData(char *buf, u16_t len)
{
}

static void app_thread(void *arg)
{
	err_t err;
    struct netbuf *inbuf;
    char* buf;
    u16_t buflen;
	
    /* Create a new TCP connection handle */
    connApp = netconn_new(NETCONN_UDP);
    if (connApp == NULL) 
	{
        printf("can not create netconn");
	}
	
	/* Bind to port 21221 with default IP address */
	err = netconn_bind(connApp, NULL, APP_LOCAL_PORT);
	if (err != ERR_OK) 
	{
		printf("can not bind netconn");
	}
	
	while(1) 
	{
		netconn_recv(connApp, &inbuf);

		/* Get RCUTool ip address and port*/
		AppAddr = *(netbuf_fromaddr(inbuf));
		AppPort = netbuf_fromport(inbuf);            
            			
		if (inbuf == NULL) 
		{
			printf("NULL data received\r\n");
			return;
		}
			
		if (netconn_err(connApp) == ERR_OK) 
		{
			netbuf_data(inbuf, (void**)&buf, &buflen);
			HandleAppData(buf, buflen);		
		    netbuf_delete(inbuf);	
		}
    }
}

void app_init()
{
    sys_thread_new("APP", app_thread, NULL, APP_THREAD_STACKSIZE, APP_THREAD_PRIO);
}
