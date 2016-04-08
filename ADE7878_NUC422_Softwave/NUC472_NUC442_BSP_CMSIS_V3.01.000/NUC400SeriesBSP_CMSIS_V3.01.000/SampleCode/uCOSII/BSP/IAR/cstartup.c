/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2011; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                         EXCEPTION VECTORS
*
* Filename      : cstartup.c
* Version       : V1.00
* Programmer(s) : FT
*                 FF
*********************************************************************************************************
*/

#include <includes.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef  union {
    CPU_FNCT_VOID   Fnct;
    void           *Ptr;
} APP_INTVECT_ELEM;

/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#pragma language=extended
#pragma segment="CSTACK"

static  void  App_Reset_ISR       (void);

static  void  App_NMI_ISR         (void);

static  void  App_Fault_ISR       (void);

static  void  App_BusFault_ISR    (void);

static  void  App_UsageFault_ISR  (void);

static  void  App_MemFault_ISR    (void);

static  void  App_Spurious_ISR    (void);

extern  void  __iar_program_start (void);

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                  EXCEPTION / INTERRUPT VECTOR TABLE
*
* Note(s) : (1) The Cortex-M4 may have up to 256 external interrupts, which are the final entries in the
*               vector table.
*
*           (2) Interrupts vector 2-13 are implemented in this file as infinite loop for debuging
*               purposes only. The application might implement a recover procedure if it is needed.
*
*           (3) OS_CPU_PendSVHandler() and OS_CPU_SysTickHandler() are implemented in the generic OS
*               port.
*********************************************************************************************************
*/

__root  const  APP_INTVECT_ELEM  __vector_table[] @ ".intvec" = {
    { .Ptr = (void *)__sfe( "CSTACK" )},                        /*  0, SP start value.                                    */
    App_Reset_ISR,                                              /*  1, PC start value.                                    */
    App_NMI_ISR,                                                /*  2, NMI.                                               */
    App_Fault_ISR,                                              /*  3, Hard Fault.                                        */
    App_MemFault_ISR,                                           /*  4, Memory Management.                                 */
    App_BusFault_ISR,                                           /*  5, Bus Fault.                                         */
    App_UsageFault_ISR,                                         /*  6, Usage Fault.                                       */
    App_Spurious_ISR,                                           /*  7, Reserved.                                          */
    App_Spurious_ISR,                                           /*  8, Reserved.                                          */
    App_Spurious_ISR,                                           /*  9, Reserved.                                          */
    App_Spurious_ISR,                                           /* 10, Reserved.                                          */
    App_Spurious_ISR,                                           /* 11, SVCall.                                            */
    App_Spurious_ISR,                                           /* 12, Debug Monitor.                                     */
    App_Spurious_ISR,                                           /* 13, Reserved.                                          */
    OS_CPU_PendSVHandler,                                       /* 14, PendSV Handler.                                    */
    OS_CPU_SysTickHandler,                                      /* 15, uC/OS-III Tick ISR Handler.                        */

    BSP_IntHandlerBOD           ,//  0: Brown Out detection
    BSP_IntHandlerIRC           ,//  1: Internal RC
    BSP_IntHandlerPWRWU         ,//  2: Power Down Wake Up
    BSP_IntHandlerSRAMF         ,//  3: SRAM parity error
    BSP_IntHandlerCLKF          ,//  4: Clock failed
    BSP_IntHandlerDefault       ,//  5: Reserved
    BSP_IntHandlerRTC           ,//  6: Real Time Clock
    BSP_IntHandlerTAMPER        ,//  7: Tamper detection
    BSP_IntHandlerEINT0         ,//  8: External Input 0
    BSP_IntHandlerEINT1         ,//  9: External Input 1
    BSP_IntHandlerEINT2         ,// 10: External Input 2
    BSP_IntHandlerEINT3         ,// 11: External Input 3
    BSP_IntHandlerEINT4         ,// 12: External Input 4
    BSP_IntHandlerEINT5         ,// 13: External Input 5
    BSP_IntHandlerEINT6         ,// 14: External Input 6
    BSP_IntHandlerEINT7         ,// 15: External Input 7
    BSP_IntHandlerGPIOA         ,// 16: GPIO Port A
    BSP_IntHandlerGPIOB         ,// 17: GPIO Port B
    BSP_IntHandlerGPIOC         ,// 18: GPIO Port C
    BSP_IntHandlerGPIOD         ,// 19: GPIO Port D
    BSP_IntHandlerGPIOE         ,// 20: GPIO Port E
    BSP_IntHandlerGPIOF         ,// 21: GPIO Port F
    BSP_IntHandlerGPIOG         ,// 22: GPIO Port G
    BSP_IntHandlerGPIOH         ,// 23: GPIO Port H
    BSP_IntHandlerGPIOI         ,// 24: GPIO Port I
    BSP_IntHandlerGPIOJ         ,// 25: GPIO Port J
    BSP_IntHandlerDefault       ,// 26: Reserved
    BSP_IntHandlerDefault       ,// 27: Reserved
    BSP_IntHandlerDefault       ,// 28: Reserved
    BSP_IntHandlerDefault       ,// 29: Reserved
    BSP_IntHandlerDefault       ,// 30: Reserved
    BSP_IntHandlerDefault       ,// 31: Reserved
    BSP_IntHandlerTIMER0        ,// 32: Timer 0
    BSP_IntHandlerTIMER1        ,// 33: Timer 1
    BSP_IntHandlerTIMER2        ,// 34: Timer 2
    BSP_IntHandlerTIMER3        ,// 35: Timer 3
    BSP_IntHandlerDefault       ,// 36: Reserved
    BSP_IntHandlerDefault       ,// 37: Reserved
    BSP_IntHandlerDefault       ,// 38: Reserved
    BSP_IntHandlerDefault       ,// 39: Reserved
    BSP_IntHandlerPDMA          ,// 40: Peripheral DMA
    BSP_IntHandlerDefault       ,// 41: Reserved
    BSP_IntHandlerADC           ,// 42: ADC
    BSP_IntHandlerDefault       ,// 43: Reserved
    BSP_IntHandlerDefault       ,// 44: Reserved
    BSP_IntHandlerDefault       ,// 45: Reserved
    BSP_IntHandlerWDT           ,// 46: Watch Dog Timer
    BSP_IntHandlerWWDT          ,// 47: Window Watch Dog Timer
    BSP_IntHandlerEADC0         ,// 48: EADC0
    BSP_IntHandlerEADC1         ,// 49: EADC1
    BSP_IntHandlerEADC2         ,// 50: EADC2
    BSP_IntHandlerEADC3         ,// 51: EADC3
    BSP_IntHandlerDefault       ,// 52: Reserved
    BSP_IntHandlerDefault       ,// 53: Reserved
    BSP_IntHandlerDefault       ,// 54: Reserved
    BSP_IntHandlerDefault       ,// 55: Reserved
    BSP_IntHandlerACMP0         ,// 56: Analog Comparator
    BSP_IntHandlerDefault       ,// 57: Reserved
    BSP_IntHandlerDefault       ,// 58: Reserved
    BSP_IntHandlerDefault       ,// 59: Reserved
    BSP_IntHandlerOPA0          ,// 60: OPA0
    BSP_IntHandlerOPA1          ,// 61: OPA1
    BSP_IntHandlerICAP0         ,// 62: ICAP0
    BSP_IntHandlerICAP1         ,// 63: ICAP1
    BSP_IntHandlerPWMA0         ,// 64: PWM A channel 0
    BSP_IntHandlerPWMA1         ,// 65: PWM A channel 1
    BSP_IntHandlerPWMA2         ,// 66: PWM A channel 2
    BSP_IntHandlerPWMA3         ,// 67: PWM A channel 3
    BSP_IntHandlerPWMA4         ,// 68: PWM A channel 4
    BSP_IntHandlerPWMA5         ,// 69: PWM A channel 5
    BSP_IntHandlerPWMABRK       ,// 70:
    BSP_IntHandlerQEI0          ,// 71:
    BSP_IntHandlerPWMB0         ,// 72: PWM B channel 0
    BSP_IntHandlerPWMB1         ,// 73: PWM B channel 1
    BSP_IntHandlerPWMB2         ,// 74: PWM B channel 2
    BSP_IntHandlerPWMB3         ,// 75: PWM B channel 3
    BSP_IntHandlerPWMB4         ,// 76: PWM B channel 4
    BSP_IntHandlerPWMB5         ,// 77: PWM B channel 5
    BSP_IntHandlerPWMBBRK       ,// 78:
    BSP_IntHandlerQEI1          ,// 79:
    BSP_IntHandlerEPWMA         ,// 80:
    BSP_IntHandlerEPWMABRK      ,// 81:
    BSP_IntHandlerEPWMB         ,// 82:
    BSP_IntHandlerEPWMBBRK      ,// 83:
    BSP_IntHandlerDefault       ,// 84: Reserved
    BSP_IntHandlerDefault       ,// 85: Reserved
    BSP_IntHandlerDefault       ,// 86: Reserved
    BSP_IntHandlerDefault       ,// 87: Reserved
    BSP_IntHandlerUSBD          ,// 88: USB Device
    BSP_IntHandlerUSBH          ,// 89: USB Host
    BSP_IntHandlerUSB_OTG       ,// 90: USB On-The-Go
    BSP_IntHandlerDefault       ,// 91: Reserved
    BSP_IntHandlerEMAC_TX       ,// 92: Ethernet MAC TX
    BSP_IntHandlerEMAC_RX       ,// 93: Ethernet MAC RX
    BSP_IntHandlerDefault       ,// 94: Reserved
    BSP_IntHandlerDefault       ,// 95: Reserved
    BSP_IntHandlerSPI0          ,// 96: SPI0
    BSP_IntHandlerSPI1          ,// 97: SPI1
    BSP_IntHandlerSPI2          ,// 98: SPI2
    BSP_IntHandlerSPI3          ,// 99: SPI3
    BSP_IntHandlerDefault       ,//100: Reserved
    BSP_IntHandlerDefault       ,//101: Reserved
    BSP_IntHandlerDefault       ,//102: Reserved
    BSP_IntHandlerDefault       ,//103: Reserved
    BSP_IntHandlerUART0         ,//104: UART0
    BSP_IntHandlerUART1         ,//105: UART1
    BSP_IntHandlerUART2         ,//106: UART2
    BSP_IntHandlerUART3         ,//107: UART3
    BSP_IntHandlerUART4         ,//108: UART4
    BSP_IntHandlerUART5         ,//109: UART5
    BSP_IntHandlerDefault       ,//110: Reserved
    BSP_IntHandlerDefault       ,//111: Reserved
    BSP_IntHandlerI2C0          ,//112: I2C 0
    BSP_IntHandlerI2C1          ,//113: I2C 1
    BSP_IntHandlerI2C2          ,//114: I2C 2
    BSP_IntHandlerI2C3          ,//115: I2C 3
    BSP_IntHandlerI2C4          ,//116: I2C 4
    BSP_IntHandlerDefault       ,//117: Reserved
    BSP_IntHandlerDefault       ,//118: Reserved
    BSP_IntHandlerDefault       ,//119: Reserved
    BSP_IntHandlerSC0           ,//120: Smart Card 0
    BSP_IntHandlerSC1           ,//121: Smart Card 1
    BSP_IntHandlerSC2           ,//122: Smart Card 2
    BSP_IntHandlerSC3           ,//123: Smart Card 3
    BSP_IntHandlerSC4           ,//124: Smart Card 4
    BSP_IntHandlerSC5           ,//125: Smart Card 5
    BSP_IntHandlerDefault       ,//126: Reserved
    BSP_IntHandlerDefault       ,//127: Reserved
    BSP_IntHandlerCAN0          ,//128: CAN 0
    BSP_IntHandlerCAN1          ,//129: CAN 1
    BSP_IntHandlerDefault       ,//130: Reserved
    BSP_IntHandlerDefault       ,//131: Reserved
    BSP_IntHandlerI2S0          ,//132: I2S0
    BSP_IntHandlerI2S1          ,//133: I2S1
    BSP_IntHandlerDefault       ,//134: Reserved
    BSP_IntHandlerDefault       ,//135: Reserved
    BSP_IntHandlerSD            ,//136: SD Card
    BSP_IntHandlerDefault       ,//137: Reserved
    BSP_IntHandlerPS2D          ,//138: PS2 Device
    BSP_IntHandlerDefault       ,//139: Reserved
    BSP_IntHandlerCRYPTO        ,//140: Cryptographic Engine
    BSP_IntHandlerCRC           ,//141: CRC
};

/*
*********************************************************************************************************
*                                            App_Reset_ISR()
*
* Description : Handle Reset.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_Reset_ISR (void)
{
#if __ARMVFP__                                                  /* Enable access to Floating-point coprocessor.         */
    CPU_REG_NVIC_CPACR = CPU_REG_NVIC_CPACR_CP10_FULL_ACCESS | CPU_REG_NVIC_CPACR_CP11_FULL_ACCESS;

    DEF_BIT_CLR(CPU_REG_SCB_FPCCR, DEF_BIT_31);                 /* Disable automatic FP register content                */
    DEF_BIT_CLR(CPU_REG_SCB_FPCCR, DEF_BIT_30);                 /* Disable Lazy context switch                          */
#endif

    __iar_program_start();
}

/*
*********************************************************************************************************
*                                            App_NMI_ISR()
*
* Description : Handle Non-Maskable Interrupt (NMI).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) Since the NMI is not being used, this serves merely as a catch for a spurious
*                   exception.
*********************************************************************************************************
*/

static  void  App_NMI_ISR (void)
{
    while (DEF_TRUE) {
        ;
    }
}


/*
*********************************************************************************************************
*                                             App_Fault_ISR()
*
* Description : Handle hard fault.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_Fault_ISR (void)
{
    while (DEF_TRUE) {
        ;
    }
}


/*
*********************************************************************************************************
*                                           App_BusFault_ISR()
*
* Description : Handle bus fault.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_BusFault_ISR (void)
{
    while (DEF_TRUE) {
        ;
    }
}


/*
*********************************************************************************************************
*                                          App_UsageFault_ISR()
*
* Description : Handle usage fault.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_UsageFault_ISR (void)
{
    while (DEF_TRUE) {
        ;
    }
}


/*
*********************************************************************************************************
*                                           App_MemFault_ISR()
*
* Description : Handle memory fault.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_MemFault_ISR (void)
{
    while (DEF_TRUE) {
        ;
    }
}


/*
*********************************************************************************************************
*                                           App_Spurious_ISR()
*
* Description : Handle spurious interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_Spurious_ISR (void)
{
    while (DEF_TRUE) {
        ;
    }
}
