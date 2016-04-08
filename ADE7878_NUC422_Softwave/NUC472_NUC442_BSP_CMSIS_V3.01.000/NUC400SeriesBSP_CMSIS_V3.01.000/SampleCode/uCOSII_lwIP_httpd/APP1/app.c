/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/
/* A simple HTTP server to demonstrate LwIP under £gC/OS-II. 
   The server¡¦s IP address could be configured statically to 
   192.168.0.2, or assigned by DHCP server. */ 

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdio.h>
#include  <string.h>
#include  <app_cfg.h>
#include  <includes.h>
#include "NUC472_442.h"
#include "lwip/api.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "ethernetif.h"
#include "httpserver-netconn.h"

unsigned char my_mac_addr[6] = {0x00, 0x00, 0x00, 0x55, 0x66, 0x77};

extern void EMAC_TX_IRQHandler(void);
extern void EMAC_RX_IRQHandler(void);

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#define TASK_STACK_SIZE            256

#define TASK_1_PRIO                10


/* ----------------- APPLICATION GLOBALS ------------------ */
static  OS_STK          StackOfTask_1[TASK_STACK_SIZE];
struct netif netif;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTask_1                 (void        *p_arg);

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int main(void)
{
#if (OS_TASK_NAME_EN > 0)
    CPU_INT08U  err;
#endif

    SYS_UnlockReg();

    BSP_Init();                                                 /* Initialize BSP functions                             */

    BSP_Set_PLL_Freq(CLK_XTL_12M, PLL_DEFAULT_HZ);                /* Configure PLL                                        */
    BSP_Set_System_Clock(CLK_PLL, 0);                              /* Select HCLK source clock as PLL                      */

    SYS_LockReg();

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"          */

    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    Mem_Init();                                                 /* Initialize memory managment module                   */
    Math_Init();                                                /* Initialize mathematical module                       */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                               */
#endif

    OSTaskCreateExt((void (*)(void *)) AppTask_1,            /* Create the test task                                */
                    (void           *) 0,
                    (OS_STK         *)&StackOfTask_1[TASK_STACK_SIZE - 1],
                    (INT8U           ) TASK_1_PRIO,
                    (INT16U          ) TASK_1_PRIO,
                    (OS_STK         *)&StackOfTask_1[0],
                    (INT32U          ) TASK_STACK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(TASK_1_PRIO, "Task 1", &err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    return (1);
}

void SYS_Init(void)
{

    // Enable EMAC clock
    CLK->AHBCLK |= CLK_AHBCLK_EMACCKEN_Msk;
    // Configure MDC clock rate to HCLK / (127 + 1) = 656 kHz if system is running at 84 MHz
    CLK->CLKDIV3 = (CLK->CLKDIV3 & ~CLK_CLKDIV3_EMACDIV_Msk) | (0x7F << CLK_CLKDIV3_EMACDIV_Pos);

    // Configure RMII pins
    SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC0MFP_Msk |
                                       SYS_GPC_MFPL_PC1MFP_Msk |
                                       SYS_GPC_MFPL_PC2MFP_Msk |
                                       SYS_GPC_MFPL_PC3MFP_Msk |
                                       SYS_GPC_MFPL_PC4MFP_Msk |
                                       SYS_GPC_MFPL_PC6MFP_Msk |
                                       SYS_GPC_MFPL_PC7MFP_Msk)) |
                    SYS_GPC_MFPL_PC0MFP_EMAC_REFCLK |
                    SYS_GPC_MFPL_PC1MFP_EMAC_MII_RXERR |
                    SYS_GPC_MFPL_PC2MFP_EMAC_MII_RXDV |
                    SYS_GPC_MFPL_PC3MFP_EMAC_MII_RXD1 |
                    SYS_GPC_MFPL_PC4MFP_EMAC_MII_RXD0 |
                    SYS_GPC_MFPL_PC6MFP_EMAC_MII_TXD0 |
                    SYS_GPC_MFPL_PC7MFP_EMAC_MII_TXD1;


    SYS->GPC_MFPH = (SYS->GPC_MFPH & ~SYS_GPC_MFPH_PC8MFP_Msk) | SYS_GPC_MFPH_PC8MFP_EMAC_MII_TXEN;
    // Enable high slew rate on all RMII pins
    PC->SLEWCTL |= 0x1DF;

    // Configure MDC, MDIO at PB14 & PB15
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB14MFP_Msk | SYS_GPB_MFPH_PB15MFP_Msk)) |
                    SYS_GPB_MFPH_PB14MFP_EMAC_MII_MDC |
                    SYS_GPB_MFPH_PB15MFP_EMAC_MII_MDIO;


}

/*
*********************************************************************************************************
*                                          TEST TASK
*
* Description : A test task
*
* Arguments   : p_arg   is the argument passed to 'AppTask_2()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTask_1(void *p_arg)
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;


    APP_TRACE_INFO(("AppTask_1\n\r"));

    IP4_ADDR(&gw, 192,168,0,1);
    IP4_ADDR(&ipaddr, 192,168,0,2);
    IP4_ADDR(&netmask, 255,255,255,0);


    tcpip_init(NULL, NULL);

    SYS_Init();


    netif_add(&netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);

    netif_set_default(&netif);

    BSP_IntVectSet(BSP_INT_ID_EMAC_TX, EMAC_TX_IRQHandler);
    BSP_IntEn(BSP_INT_ID_EMAC_TX);
    BSP_IntVectSet(BSP_INT_ID_EMAC_RX, EMAC_RX_IRQHandler);
    BSP_IntEn(BSP_INT_ID_EMAC_RX);

    netif_set_up(&netif);

    http_server_netconn_init();

    OSTaskSuspend(TASK_1_PRIO);
}



