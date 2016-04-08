/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                           Nuvoton NUC47x
*                                              on the
*
*                                             FPGA Board
*
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : YC
*                 
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               BSP present pre-processor macro definition.
*
*********************************************************************************************************
*/

#ifndef  BSP_PRESENT
#define  BSP_PRESENT


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_MODULE
#define  BSP_EXT
#else
#define  BSP_EXT  extern
#endif


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdio.h>
#include  <stdarg.h>

#include  <cpu.h>
#include  <cpu_core.h>

#include  <lib_def.h>
#include  <lib_ascii.h>

#include "NUC472_442.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               CLOCK
*********************************************************************************************************
*/
typedef enum
{
    CLK_XTL_12M = 0,
    CLK_XTL_32K,
    CLK_OSC_22M,
    CLK_OSC_10K,
    CLK_PLL
}     E_CLK_SRC;

#define HXT_HZ              12000000u
#define LXT_HZ              32768u
#define LIRC_HZ             10000u
#define USB_PLL_HZ          48000000u
#define PLL_DEFAULT_HZ      84000000u
#define HIRC_HZ             22118400u


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                                 MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               INT DEFINES
*********************************************************************************************************
*/

#define  BSP_INT_ID_BOD                                    0    /* Brown Out detection                                  */
#define  BSP_INT_ID_IRC                                    1    /* Internal RC                                          */
#define  BSP_INT_ID_PWRWU                                  2    /* Power Down Wake Up                                   */
#define  BSP_INT_ID_SRAMF                                  3    /* SRAM parity error                                    */
#define  BSP_INT_ID_CLKF                                   4    /* Clock failed                                         */
#define  BSP_INT_ID_Default                                5    /* Reserved                                             */
#define  BSP_INT_ID_RTC                                    6    /* Real Time Clock                                      */
#define  BSP_INT_ID_TAMPER                                 7    /* Tamper detection                                     */
#define  BSP_INT_ID_EINT0                                  8    /* External Input 0                                     */
#define  BSP_INT_ID_EINT1                                  9    /* External Input 1                                     */
#define  BSP_INT_ID_EINT2                                 10    /* External Input 2                                     */
#define  BSP_INT_ID_EINT3                                 11    /* External Input 3                                     */
#define  BSP_INT_ID_EINT4                                 12    /* External Input 4                                     */
#define  BSP_INT_ID_EINT5                                 13    /* External Input 5                                     */
#define  BSP_INT_ID_EINT6                                 14    /* External Input 6                                     */
#define  BSP_INT_ID_EINT7                                 15    /* External Input 7                                     */
#define  BSP_INT_ID_GPIOA                                 16    /* GPIO Port A                                          */
#define  BSP_INT_ID_GPIOB                                 17    /* GPIO Port B                                          */
#define  BSP_INT_ID_GPIOC                                 18    /* GPIO Port C                                          */
#define  BSP_INT_ID_GPIOD                                 19    /* GPIO Port D                                          */
#define  BSP_INT_ID_GPIOE                                 20    /* GPIO Port E                                          */
#define  BSP_INT_ID_GPIOF                                 21    /* GPIO Port F                                          */
#define  BSP_INT_ID_GPIOG                                 22    /* GPIO Port G                                          */
#define  BSP_INT_ID_GPIOH                                 23    /* GPIO Port H                                          */
#define  BSP_INT_ID_GPIOI                                 24    /* GPIO Port I                                          */
#define  BSP_INT_ID_GPIOJ                                 25    /* GPIO Port J                                          */
#define  BSP_INT_ID_TIMER0                                32    /* Timer 0                                              */
#define  BSP_INT_ID_TIMER1                                33    /* Timer 1                                              */
#define  BSP_INT_ID_TIMER2                                34    /* Timer 2                                              */
#define  BSP_INT_ID_TIMER3                                35    /* Timer 3                                              */
#define  BSP_INT_ID_PDMA                                  40    /* Peripheral DMA                                       */
#define  BSP_INT_ID_ADC                                   42    /* ADC                                                  */
#define  BSP_INT_ID_WDT                                   46    /* Watch Dog Timer                                      */
#define  BSP_INT_ID_WWDT                                  47    /* Window Watch Dog Timer                               */
#define  BSP_INT_ID_EADC0                                 48    /*                                                      */
#define  BSP_INT_ID_EADC1                                 49    /*                                                      */
#define  BSP_INT_ID_EADC2                                 50    /*                                                      */
#define  BSP_INT_ID_EADC3                                 51    /*                                                      */
#define  BSP_INT_ID_ACMP0                                 56    /* Analog Comparator                                    */
#define  BSP_INT_ID_OPA0                                  60    /* OPA0                                                 */
#define  BSP_INT_ID_OPA1                                  61    /* OPA1                                                 */
#define  BSP_INT_ID_ICAP0                                 62    /* ICAP0                                                */
#define  BSP_INT_ID_ICAP1                                 63    /* ICAP1                                                */
#define  BSP_INT_ID_PWMA0                                 64    /* PWM A channel 0                                      */
#define  BSP_INT_ID_PWMA1                                 65    /* PWM A channel 1                                      */
#define  BSP_INT_ID_PWMA2                                 66    /* PWM A channel 2                                      */
#define  BSP_INT_ID_PWMA3                                 67    /* PWM A channel 3                                      */
#define  BSP_INT_ID_PWMA4                                 68    /* PWM A channel 4                                      */
#define  BSP_INT_ID_PWMA5                                 69    /* PWM A channel 5                                      */
#define  BSP_INT_ID_PWMABRK                               70    /*                                                      */
#define  BSP_INT_ID_QEI0                                  71    /*                                                      */
#define  BSP_INT_ID_PWMB0                                 72    /* PWM B channel 0                                      */
#define  BSP_INT_ID_PWMB1                                 73    /* PWM B channel 1                                      */
#define  BSP_INT_ID_PWMB2                                 74    /* PWM B channel 2                                      */
#define  BSP_INT_ID_PWMB3                                 75    /* PWM B channel 3                                      */
#define  BSP_INT_ID_PWMB4                                 76    /* PWM B channel 4                                      */
#define  BSP_INT_ID_PWMB5                                 77    /* PWM B channel 5                                      */
#define  BSP_INT_ID_PWMBBRK                               78    /*                                                      */
#define  BSP_INT_ID_QEI1                                  79    /*                                                      */
#define  BSP_INT_ID_EPWMA                                 80    /*                                                      */
#define  BSP_INT_ID_EPWMABRK                              81    /*                                                      */
#define  BSP_INT_ID_EPWMB                                 82    /*                                                      */
#define  BSP_INT_ID_EPWMBBRK                              83    /*                                                      */
#define  BSP_INT_ID_USBD                                  88    /* USBH Device                                          */
#define  BSP_INT_ID_USBH                                  89    /* USB Host                                             */
#define  BSP_INT_ID_USB_OTG                               90    /* USB OTG                                              */
#define  BSP_INT_ID_EMAC_TX                               92    /* Ethernet MAC TX                                      */
#define  BSP_INT_ID_EMAC_RX                               93    /* Ethernet MAC RX                                      */
#define  BSP_INT_ID_SPI0                                  96    /* SPI 0                                                */
#define  BSP_INT_ID_SPI1                                  97    /* SPI 1                                                */
#define  BSP_INT_ID_SPI2                                  98    /* SPI 2                                                */
#define  BSP_INT_ID_SPI3                                  99    /* SPI 3                                                */
#define  BSP_INT_ID_UART0                                104    /* UART 0                                               */
#define  BSP_INT_ID_UART1                                105    /* UART 1                                               */
#define  BSP_INT_ID_UART2                                106    /* UART 2                                               */
#define  BSP_INT_ID_UART3                                107    /* UART 3                                               */
#define  BSP_INT_ID_UART4                                108    /* UART 4                                               */
#define  BSP_INT_ID_UART5                                109    /* UART 5                                               */
#define  BSP_INT_ID_I2C0                                 112    /* I2C 0                                                */
#define  BSP_INT_ID_I2C1                                 113    /* I2C 1                                                */
#define  BSP_INT_ID_I2C2                                 114    /* I2C 2                                                */
#define  BSP_INT_ID_I2C3                                 115    /* I2C 3                                                */
#define  BSP_INT_ID_I2C4                                 116    /* I2C 4                                                */
#define  BSP_INT_ID_SC0                                  120    /* Smart Card 0                                         */
#define  BSP_INT_ID_SC1                                  121    /* Smart Card 1                                         */
#define  BSP_INT_ID_SC2                                  122    /* Smart Card 2                                         */
#define  BSP_INT_ID_SC3                                  123    /* Smart Card 3                                         */
#define  BSP_INT_ID_SC4                                  124    /* Smart Card 4                                         */
#define  BSP_INT_ID_SC5                                  125    /* Smart Card 5                                         */
#define  BSP_INT_ID_CAN0                                 128    /* CAN 0                                                */
#define  BSP_INT_ID_CAN1                                 129    /* CAN 1                                                */
#define  BSP_INT_ID_I2S0                                 132    /* I2S 0                                                */
#define  BSP_INT_ID_I2S1                                 133    /* I2S 1                                                */
#define  BSP_INT_ID_SD                                   136    /* SD Card                                              */
#define  BSP_INT_ID_PS2D                                 138    /* PS2 Device                                           */
#define  BSP_INT_ID_CRYPTO                               140    /* Cryptographic Engine                                 */
#define  BSP_INT_ID_CRC                                  141    /* CRC                                                  */


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void        BSP_Init                          (void);
void        BSP_IntDisAll                     (void);
CPU_INT32U  BSP_CPU_ClkFreq                   (void);
void        BSP_Tick_Init                     (void);
int         BSP_Enable_Clock                  (E_CLK_SRC clk);
int         BSP_Set_System_Clock              (E_CLK_SRC, int clk_div);
int         BSP_Set_PLL_Freq                  (E_CLK_SRC, CPU_INT32U pll_freq_Hz);

/*
*********************************************************************************************************
*                                           INTERRUPT SERVICES
*********************************************************************************************************
*/

void        BSP_IntInit                       (void);

void        BSP_IntEn                         (CPU_DATA       int_id);

void        BSP_IntDis                        (CPU_DATA       int_id);

void        BSP_IntClr                        (CPU_DATA       int_id);

void        BSP_IntVectSet                    (CPU_DATA       int_id,
                                               CPU_FNCT_VOID  isr);

void        BSP_IntPrioSet                    (CPU_DATA       int_id,
                                               CPU_INT08U     prio);

void         BSP_IntHandlerBOD                (void);
void         BSP_IntHandlerIRC                (void);
void         BSP_IntHandlerPWRWU              (void);
void         BSP_IntHandlerSRAMF              (void);
void         BSP_IntHandlerCLKF               (void);
void         BSP_IntHandlerRTC                (void);
void         BSP_IntHandlerTAMPER             (void);
void         BSP_IntHandlerEINT0              (void);
void         BSP_IntHandlerEINT1              (void);
void         BSP_IntHandlerEINT2              (void);
void         BSP_IntHandlerEINT3              (void);
void         BSP_IntHandlerEINT4              (void);
void         BSP_IntHandlerEINT5              (void);
void         BSP_IntHandlerEINT6              (void);
void         BSP_IntHandlerEINT7              (void);
void         BSP_IntHandlerGPIOA              (void);
void         BSP_IntHandlerGPIOB              (void);
void         BSP_IntHandlerGPIOC              (void);
void         BSP_IntHandlerGPIOD              (void);
void         BSP_IntHandlerGPIOE              (void);
void         BSP_IntHandlerGPIOF              (void);
void         BSP_IntHandlerGPIOG              (void);
void         BSP_IntHandlerGPIOH              (void);
void         BSP_IntHandlerGPIOI              (void);
void         BSP_IntHandlerGPIOJ              (void);
void         BSP_IntHandlerTIMER0             (void);
void         BSP_IntHandlerTIMER1             (void);
void         BSP_IntHandlerTIMER2             (void);
void         BSP_IntHandlerTIMER3             (void);
void         BSP_IntHandlerPDMA               (void);
void         BSP_IntHandlerADC                (void);
void         BSP_IntHandlerWDT                (void);
void         BSP_IntHandlerWWDT               (void);
void         BSP_IntHandlerEADC0              (void);
void         BSP_IntHandlerEADC1              (void);
void         BSP_IntHandlerEADC2              (void);
void         BSP_IntHandlerEADC3              (void);
void         BSP_IntHandlerACMP0              (void);
void         BSP_IntHandlerOPA0               (void);
void         BSP_IntHandlerOPA1               (void);
void         BSP_IntHandlerICAP0              (void);
void         BSP_IntHandlerICAP1              (void);
void         BSP_IntHandlerPWMA0              (void);
void         BSP_IntHandlerPWMA1              (void);
void         BSP_IntHandlerPWMA2              (void);
void         BSP_IntHandlerPWMA3              (void);
void         BSP_IntHandlerPWMA4              (void);
void         BSP_IntHandlerPWMA5              (void);
void         BSP_IntHandlerPWMABRK            (void);
void         BSP_IntHandlerQEI0               (void);
void         BSP_IntHandlerPWMB0              (void);
void         BSP_IntHandlerPWMB1              (void);
void         BSP_IntHandlerPWMB2              (void);
void         BSP_IntHandlerPWMB3              (void);
void         BSP_IntHandlerPWMB4              (void);
void         BSP_IntHandlerPWMB5              (void);
void         BSP_IntHandlerPWMBBRK            (void);
void         BSP_IntHandlerQEI1               (void);
void         BSP_IntHandlerEPWMA              (void);
void         BSP_IntHandlerEPWMABRK           (void);
void         BSP_IntHandlerEPWMB              (void);
void         BSP_IntHandlerEPWMBBRK           (void);
void         BSP_IntHandlerUSBD               (void);
void         BSP_IntHandlerUSBH               (void);
void         BSP_IntHandlerUSB_OTG            (void);
void         BSP_IntHandlerEMAC_TX            (void);
void         BSP_IntHandlerEMAC_RX            (void);
void         BSP_IntHandlerSPI0               (void);
void         BSP_IntHandlerSPI1               (void);
void         BSP_IntHandlerSPI2               (void);
void         BSP_IntHandlerSPI3               (void);
void         BSP_IntHandlerUART0              (void);
void         BSP_IntHandlerUART1              (void);
void         BSP_IntHandlerUART2              (void);
void         BSP_IntHandlerUART3              (void);
void         BSP_IntHandlerUART4              (void);
void         BSP_IntHandlerUART5              (void);
void         BSP_IntHandlerI2C0               (void);
void         BSP_IntHandlerI2C1               (void);
void         BSP_IntHandlerI2C2               (void);
void         BSP_IntHandlerI2C3               (void);
void         BSP_IntHandlerI2C4               (void);
void         BSP_IntHandlerSC0                (void);
void         BSP_IntHandlerSC1                (void);
void         BSP_IntHandlerSC2                (void);
void         BSP_IntHandlerSC3                (void);
void         BSP_IntHandlerSC4                (void);
void         BSP_IntHandlerSC5                (void);
void         BSP_IntHandlerCAN0               (void);
void         BSP_IntHandlerCAN1               (void);
void         BSP_IntHandlerI2S0               (void);
void         BSP_IntHandlerI2S1               (void);
void         BSP_IntHandlerSD                 (void);
void         BSP_IntHandlerPS2D               (void);
void         BSP_IntHandlerCRYPTO             (void);
void         BSP_IntHandlerCRC                (void);
void         BSP_IntHandlerDefault            (void);


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/


#endif                                                          /* End of module include.                               */

