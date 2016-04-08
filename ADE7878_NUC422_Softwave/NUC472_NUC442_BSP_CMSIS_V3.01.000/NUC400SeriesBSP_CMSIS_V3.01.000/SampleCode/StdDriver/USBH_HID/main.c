/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 9 $
 * $Date: 14/05/29 1:14p $
 * @brief    Use USB Host core driver and HID driver. It reads raw data 
 *           from a USB mouse.
 *
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC472_442.h"
#include "usbh_core.h"
#include "usbh_hid.h"


#define PLLCON_SETTING      0xc22e
#define PLL_CLOCK           48000000

uint32_t PllClock         = PLL_CLOCK;

/*
 *  The following path definitions are depended on device.
 */
#define   PATHLEN           3
int       PATH_IN[PATHLEN] = { 0x00010002, 0x00010001, 0x00010031 };
#define   RECV_PACKET_LEN       4
char      packet[RECV_PACKET_LEN];



void Delay(uint32_t delayCnt)
{
    while(delayCnt--) {
        __NOP();
        __NOP();
    }
}


void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady( CLK_STATUS_HXTSTB_Msk);

    /* Switch HCLK clock source to XTAL */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT,CLK_CLKDIV0_HCLK(1));

    /* Set PLL to power down mode and PLL_STB bit in CLKSTATUS register will be cleared by hardware.*/
    CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;

    /* Set PLL frequency */
    CLK->PLLCTL = PLLCON_SETTING;

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_STATUS_PLLSTB_Msk);

    /* Switch HCLK clock source to PLL */
    CLK->CLKSEL0 = CLK_CLKSEL0_HCLKSEL_PLL;

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE,CLK_CLKSEL1_UARTSEL_HXT,CLK_CLKDIV0_UART(1));

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            // PLL
    SystemCoreClock = PLL_CLOCK / 1;        // HCLK
    CyclesPerUs     = PLL_CLOCK / 1000000;  // For SYS_SysTickDelay()

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPG multi-function pins for UART0 RXD and TXD */
    SYS->GPG_MFPL = SYS_GPG_MFPL_PG1MFP_UART0_RXD | SYS_GPG_MFPL_PG2MFP_UART0_TXD ;

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init USB Host clock                                                                                     */
    /*---------------------------------------------------------------------------------------------------------*/

    // Configure OTG function as Host-Only
    SYS->USBPHY = 0x101;

    // Multi-function PB.0 OTG_5V status, PB.1 OTG_5V enable
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_USB0_OTG5V_ST | SYS_GPB_MFPL_PB1MFP_USB0_OTG5V_EN)) | 0x11;

    // Multi-function (PB.2,PB.3) for USB port 2 (D-,D+)
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB2MFP_USB1_D_N | SYS_GPB_MFPL_PB3MFP_USB1_D_P)) | 0x3300;

    // Set PB.4 output high to enable USB power
    SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB4MFP_Msk;
    PB->MODE = (PB->MODE & ~GPIO_MODE_MODE4_Msk) | (0x1 << GPIO_MODE_MODE4_Pos);
    PB->DOUT |= 0x10;

    // Select USB Host clock source from PLL
    CLK->CLKSEL0 |= CLK_CLKSEL0_USBHSEL_Msk;

    // USB clock is HCLK divided by 1
    CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_USBHDIV_Msk) | (0 << CLK_CLKDIV0_USBHDIV_Pos);

    // Enable USB Host
    CLK->AHBCLK |= CLK_AHBCLK_USBHCKEN_Msk;

    /* Lock protected registers */
    SYS_LockReg();
}


void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate */
    UART0->LINE = 0x07;
    UART0->BAUD = 0x30000066;   /* 12MHz reference clock input, for 115200 */
}


void  int_read_callback(uint8_t *rdata, int data_len)
{
    int  i;
    printf("INT-in pipe data %d bytes received =>\n", data_len);
    for (i = 0; i < data_len; i++)
        printf("0x%02x ", rdata[i]);
    printf("\n");
}

static uint8_t  _write_data_buff[4];

void  int_write_callback(uint8_t **wbuff, int *buff_size)
{
    printf("INT-out pipe request to write data.\n");

    *wbuff = &_write_data_buff[0];
    *buff_size = 4;
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    int32_t       i32Handle;
    int32_t       ret;
    HIDInterfaceMatcher  matcher = { 0x0, 0x0, 0 };

    /* Lock protected registers */
    if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART0 for printf */
    UART0_Init();

    printf("\n\n");
    printf("+--------------------------------------+\n");
    printf("|                                      |\n");
    printf("|     USB Host HID sample program      |\n");
    printf("|                                      |\n");
    printf("+--------------------------------------+\n");

    USBH_Open();

    USBH_HidInit();

    Delay(0x500000);
    USBH_ProcessHubEvents();

    ret = USBH_HidOpen(0, &matcher, &i32Handle);
    if (ret != HID_RET_SUCCESS) {
        printf("hid_force_open failed with return code %d\n", ret);
        goto err_out;
    }

    ret = USBH_HidDumpTree(i32Handle);
    if (ret != HID_RET_SUCCESS) {
        printf("hid_dump_tree failed with return code %d\n", ret);
        goto err_out;
    }

    printf("\nUSBH_HidGetInputReport...\n");
    ret = USBH_HidGetInputReport(i32Handle, PATH_IN, PATHLEN, packet, RECV_PACKET_LEN);
    if (ret != HID_RET_SUCCESS) {
        printf("hid_get_input_report failed with return code %d\n", ret);
        //goto err_out;
    }

    printf("\nUSBH_HidStartIntReadPipe...\n");
    if (USBH_HidStartIntReadPipe(i32Handle, int_read_callback) == HID_RET_SUCCESS) {
        printf("Press any key to stop it...\n");
        getchar();
        USBH_ProcessHubEvents();
    }

    if (USBH_HidStartIntWritePipe(i32Handle, int_write_callback) == HID_RET_SUCCESS) {
        printf("Press any key to stop it...\n");
        getchar();
    }

    ret = USBH_HidClose(i32Handle);
    if (ret != HID_RET_SUCCESS) {
        printf("hid_close failed with return code %d\n", ret);
        goto err_out;
    }

    printf("Done.\n");

    while (1);

err_out:
    while (1);
}


/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
