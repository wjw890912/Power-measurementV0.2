/*
    FreeRTOS V7.4.0 - Copyright (C) 2013 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.

    >>>>>>NOTE<<<<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details. You should have received a copy of the GNU General Public License
    and the FreeRTOS license exception along with FreeRTOS; if not itcan be
    viewed here: http://www.freertos.org/a00114.html and also obtained by
    writing to Real Time Engineers Ltd., contact details for whom are available
    on the FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, and our new
    fully thread aware and reentrant UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems, who sell the code with commercial support,
    indemnification and middleware, under the OpenRTOS brand.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.
*/

/******************************************************************************
 * >>>>>> NOTE 1: <<<<<<
 *
 * main() can be configured to create either a very simple LED flasher demo, or
 * a more comprehensive test/demo application.
 *
 * To create a very simple LED flasher example, set the
 * mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY constant (defined below) to 1.  When
 * this is done, only the standard demo flash tasks are created.  The standard
 * demo flash example creates three tasks, each of which toggle an LED at a
 * fixed but different frequency.
 *
 * To create a more comprehensive test and demo application, set
 * mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY to 0.
 *
 * >>>>>> NOTE 2: <<<<<<
 *
 * In addition to the normal set of standard demo tasks, the comprehensive test
 * makes heavy use of the floating point unit, and forces floating point
 * instructions to be used from interrupts that nest three deep.  The nesting
 * starts from the tick hook function, resulting is an abnormally long context
 * switch time.  This is done purely to stress test the FPU context switching
 * implementation, and that part of the test can be removed by setting
 * configUSE_TICK_HOOK to 0 in FreeRTOSConfig.h.
 ******************************************************************************
 *
 * main() creates all the demo application tasks and software timers, then starts
 * the scheduler.  The web documentation provides more details of the standard
 * demo application tasks, which provide no particular functionality, but do
 * provide a good example of how to use the FreeRTOS API.
 *
 */

/* A TCP echo server which is implemented with LwIP under FreeRTOS. 
   The server listen to port 80, IP address could configure statically 
   to 192.168.1.2 or assign by DHCP server. This server replies 
   "Hello World!!" if the received string is "nuvoton", otherwise 
    reply "Wrong Password!!" to its client. */
    
#include <stdio.h>

/* Hardware and starter kit includes. */
#include "NUC472_442.h"


#include "lwip/api.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"
#include "ethernetif.h"
#include "function.h"
#include "btrcu.h"

struct netif netif;
__IO uint32_t TCPTimer = 0;
__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
__IO uint32_t DHCPfineTimer = 0;
__IO uint32_t DHCPcoarseTimer = 0;
#endif

__IO uint32_t DisplayTimer = 0;
uint8_t LedToggle = 4;
uint8_t	Server = 0;

#define SYSTEMTICK_PERIOD_MS  10

__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;

/*-----------------------------------------------------------*/
/*
uint8_t sys_IpType = 0;	// 0 ~ static ip, 1 ~ dhcp
uint8_t sys_IpAddr[4] = {192, 168, 1, 230};
uint8_t sys_SubnetMask[4] = {255, 255, 0, 0};
uint8_t sys_RouterIp[4] = {192, 168, 0, 20};
uint8_t sys_ServerIp[4] = {192, 168, 0, 5};
uint8_t sys_RoomNumber[16] = {0};
*/
uint8_t sys_Settings[OPT_MAX_LEN];

uint8_t g_u8SendData[TXBUFSIZE];
uint8_t g_u8RecvData[RXBUFSIZE];
volatile uint32_t g_u32comRhead  = 0;
volatile uint32_t g_u32comRtail  = 0;
volatile uint32_t g_u32comShead  = 0;
volatile uint32_t g_u32comStail  = 0;

uint8_t g_u8SerialBuf[RXBUFSIZE];
volatile uint16_t g_u16SerialLen;
	
static void SYS_Init( void );
/*-----------------------------------------------------------*/

unsigned char my_mac_addr[6] = {0x00, 0x00, 0x00, 0x55, 0x66, 0x77};
struct netif netif;

/*-----------------------------------------------------------*/
void SetSysTick(uint32_t us)
{
    SysTick->LOAD = us * CyclesPerUs;
    SysTick->VAL  =  (0x00);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

static void SYS_Init( void )
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady( CLK_STATUS_HXTSTB_Msk);

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT,CLK_CLKDIV0_HCLK(1));

    /* Set PLL to power down mode and PLL_STB bit in CLKSTATUS register will be cleared by hardware.*/
    CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;

    /* Set PLL frequency */
    CLK->PLLCTL = CLK_PLLCTL_84MHz_HXT;

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_STATUS_PLLSTB_Msk);

    /* Switch HCLK clock source to PLL */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL,CLK_CLKDIV0_HCLK(1));

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(EMAC_MODULE);
    CLK_EnableModuleClock(WDT_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART(1));

    // Configure MDC clock rate to HCLK / (127 + 1) = 656 kHz if system is running at 84 MHz
    CLK_SetModuleClock(EMAC_MODULE, 0, CLK_CLKDIV3_EMAC(127));
    CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);


    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();


    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPG multi-function pins for UART0 RXD and TXD */
#ifdef EVM_BOARD	
    SYS->GPG_MFPL = SYS_GPG_MFPL_PG1MFP_UART0_RXD | SYS_GPG_MFPL_PG2MFP_UART0_TXD;
#else
    SYS->GPA_MFPH = SYS_GPA_MFPH_PA13MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD;
#endif

    // Configure RMII pins
    SYS->GPC_MFPL = SYS_GPC_MFPL_PC0MFP_EMAC_REFCLK |
                    SYS_GPC_MFPL_PC1MFP_EMAC_MII_RXERR |
                    SYS_GPC_MFPL_PC2MFP_EMAC_MII_RXDV |
                    SYS_GPC_MFPL_PC3MFP_EMAC_MII_RXD1 |
                    SYS_GPC_MFPL_PC4MFP_EMAC_MII_RXD0 |
                    SYS_GPC_MFPL_PC6MFP_EMAC_MII_TXD0 |
                    SYS_GPC_MFPL_PC7MFP_EMAC_MII_TXD1;


    SYS->GPC_MFPH = SYS_GPC_MFPH_PC8MFP_EMAC_MII_TXEN;
    // Enable high slew rate on all RMII pins
    PC->SLEWCTL |= 0x1DF;

    // Configure MDC, MDIO at PB14 & PB15
    SYS->GPB_MFPH = SYS_GPB_MFPH_PB14MFP_EMAC_MII_MDC | SYS_GPB_MFPH_PB15MFP_EMAC_MII_MDIO;

	SetSysTick(10000);		// Set SysTick as 10ms
	
    /* Lock protected registers */
    SYS_LockReg();
}

void UART0_Init(void)
{
	g_u32comRhead = 0;
	g_u32comRtail = RXBUFSIZE - 1;
	g_u32comShead = 0;
	g_u32comStail = TXBUFSIZE - 1;

    /* Init UART to 19200-8n1 for print message */
    UART_Open(UART0, 19200);
    UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);		
}	

void UART0_IRQHandler(void)
{
    uint32_t u32IntSts= UART0->INTSTS;

	if(u32IntSts & UART_INTSTS_RDAINT_Msk) {
		while(UART_IS_RX_READY(UART0)) 
		{
			if(g_u32comRtail != g_u32comRhead) {
				g_u8RecvData[g_u32comRhead] = UART_READ(UART0);
				g_u32comRhead ++;
				if (g_u32comRhead == RXBUFSIZE)
				{
					g_u32comRhead = 0;
				}
			}
        }
    }
}

void SysTick_Handler(void)
{
	LocalTime += SYSTEMTICK_PERIOD_MS;
}

static void LwipInit()
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;

	  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();

/*	IP4_ADDR(&ipaddr, sys_Settings[OPT_IP_ADDRESS], sys_Settings[OPT_IP_ADDRESS+1], sys_Settings[OPT_IP_ADDRESS+2], sys_Settings[OPT_IP_ADDRESS+3]);
	IP4_ADDR(&gw, sys_Settings[OPT_ROUTER_IP], sys_Settings[OPT_ROUTER_IP+1], sys_Settings[OPT_ROUTER_IP+2], sys_Settings[OPT_ROUTER_IP+3]);
	IP4_ADDR(&netmask, sys_Settings[OPT_SUBNET_MASK], sys_Settings[OPT_SUBNET_MASK+1], sys_Settings[OPT_SUBNET_MASK+2], sys_Settings[OPT_SUBNET_MASK+3]);
    netif.hwaddr[0] = sys_Settings[OPT_MAC_ADDRESS];
    netif.hwaddr[1] = sys_Settings[OPT_MAC_ADDRESS+1];
    netif.hwaddr[2] = sys_Settings[OPT_MAC_ADDRESS+2];
    netif.hwaddr[3] = sys_Settings[OPT_MAC_ADDRESS+3];
    netif.hwaddr[4] = sys_Settings[OPT_MAC_ADDRESS+4];
    netif.hwaddr[5] = sys_Settings[OPT_MAC_ADDRESS+5];
*/
	IP4_ADDR(&ipaddr, 192, 168, 0, 10);
	IP4_ADDR(&gw, 192, 168, 0, 20);
	IP4_ADDR(&netmask, 255, 255, 0, 0);
	netif.hwaddr[0] = 0x00;
	netif.hwaddr[1] = 0x26;
	netif.hwaddr[2] = 0xc0;
	netif.hwaddr[3] = 0xa8;
	netif.hwaddr[4] = 0x00;
	netif.hwaddr[5] = 10;
	
//	printf("RCU start\r\n");
	
//    tcpip_init(NULL, NULL);

    netif_add(&netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ethernet_input);

    netif_set_default(&netif);

/*	if (sys_Settings[OPT_IP_TYPE] == 1)
	{	
		dhcp_start(&netif);
	}
	else
*/	{
		netif_set_up(&netif);
	}

	//NVIC_SetPriority(EMAC_TX_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ(EMAC_TX_IRQn);
    //NVIC_SetPriority(EMAC_RX_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ(EMAC_RX_IRQn);
}

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{
  /* TCP periodic process every 250 ms */
  if (localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  localtime;
    tcp_tmr();
  }
  /* ARP periodic process every 5s */
  if (localtime - ARPTimer >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  localtime;
    etharp_tmr();
  }

#if LWIP_DHCP
  /* Fine DHCP periodic process every 500ms */
  if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  localtime;
    dhcp_fine_tmr();
  }
  /* DHCP Coarse periodic process every 60s */
  if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  localtime;
    dhcp_coarse_tmr();
  }
#endif

}
/*-----------------------------------------------------------*/
int main(void)
{
    /* Configure the hardware ready to run the test. */
    SYS_Init();
	UART0_Init();

//	LoadSysSettings();
//	DefaultSettings();
//	SaveSysSettings();
	
	LwipInit();
	
    while(1)
	{
		LwIP_Periodic_Handle(LocalTime);
	}
}
