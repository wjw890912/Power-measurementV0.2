

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "udp_echoserver-netconn.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define UDPECHOSERVER_THREAD_PRIO    ( tskIDLE_PRIORITY + 2UL )
#define UDPECHOSERVER_THREAD_STACKSIZE  200
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u32_t nPageHits = 0;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  http server thread 
  * @param arg: pointer on argument(not used here) 
  * @retval None
  */
static void udp_echoserver_netconn_thread(void *arg)
{ 
  struct netconn *conn;
  err_t err;
  char msg[] = "Hello World!!";
  struct netbuf *buf, *buf_send;
  char *data; 
  static ip_addr_t *addr;
  static unsigned short port;
    
  /* Create a new UDP connection handle */  
  conn = netconn_new(NETCONN_UDP);
  if (conn!= NULL)
  {
    /* Bind to port 80 (HTTP) with default IP address */
    err = netconn_bind(conn, NULL, 80);
    
    if (err == ERR_OK)
    {
        while(1) {  
            printf("Wait for UDP data ...");
            while(netconn_recv(conn, &buf) != ERR_OK);
            printf(" [OK] ...\n");
            
            /* Get destination ip address and port*/
            addr = netbuf_fromaddr(buf);
            port = netbuf_fromport(buf);            
            
            /* Prepare data */          
            buf_send = netbuf_new();
            data = netbuf_alloc(buf_send, sizeof(msg)); 
            memcpy (data, msg, sizeof (msg));
            
            /* Send the packet */
            netconn_sendto(conn, buf_send, addr, port);
            
            /* Free the buffer */
            netbuf_delete(buf_send);
            netbuf_delete(buf);
        }
     }
  }
}

/**
  * @brief  Initialize the UDP echo server (start its thread) 
  * @param  none
  * @retval None
  */
void udp_echoserver_netconn_init()
{
  sys_thread_new("UDPECHO", udp_echoserver_netconn_thread, NULL, UDPECHOSERVER_THREAD_STACKSIZE, UDPECHOSERVER_THREAD_PRIO);
}

