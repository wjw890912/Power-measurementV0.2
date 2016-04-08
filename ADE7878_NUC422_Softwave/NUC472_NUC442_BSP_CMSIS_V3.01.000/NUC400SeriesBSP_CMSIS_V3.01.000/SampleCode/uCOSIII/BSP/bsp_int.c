/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
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
*                                        BOARD SUPPORT PACKAGE
*
* Filename      : bsp_int.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_INT_MODULE
#include <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  BSP_INT_SRC_NBR                                 142


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/

static  CPU_FNCT_VOID  BSP_IntVectTbl[BSP_INT_SRC_NBR];


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

static  void  BSP_IntHandler     (CPU_DATA  int_id);
static  void  BSP_IntHandlerDummy(void);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              BSP_IntClr()
*
* Description : Clear interrupt.
*
* Argument(s) : int_id      Interrupt to clear.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) An interrupt does not need to be cleared within the interrupt controller.
*********************************************************************************************************
*/

void  BSP_IntClr (CPU_DATA  int_id)
{

}


/*
*********************************************************************************************************
*                                              BSP_IntDis()
*
* Description : Disable interrupt.
*
* Argument(s) : int_id      Interrupt to disable.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntDis (CPU_DATA  int_id)
{
    if (int_id < BSP_INT_SRC_NBR) {
        CPU_IntSrcDis(int_id + 16);
    }
}


/*
*********************************************************************************************************
*                                           BSP_IntDisAll()
*
* Description : Disable ALL interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntDisAll (void)
{
    CPU_IntDis();
}


/*
*********************************************************************************************************
*                                               BSP_IntEn()
*
* Description : Enable interrupt.
*
* Argument(s) : int_id      Interrupt to enable.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntEn (CPU_DATA  int_id)
{
    if (int_id < BSP_INT_SRC_NBR) {
        CPU_IntSrcEn(int_id + 16);
    }
}


/*
*********************************************************************************************************
*                                            BSP_IntVectSet()
*
* Description : Assign ISR handler.
*
* Argument(s) : int_id      Interrupt for which vector will be set.
*
*               isr         Handler to assign
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntVectSet (CPU_DATA       int_id,
                      CPU_FNCT_VOID  isr)
{
    CPU_SR_ALLOC();


    if (int_id < BSP_INT_SRC_NBR) {
        CPU_CRITICAL_ENTER();
        BSP_IntVectTbl[int_id] = isr;
        CPU_CRITICAL_EXIT();
    }
}


/*
*********************************************************************************************************
*                                            BSP_IntPrioSet()
*
* Description : Assign ISR priority.
*
* Argument(s) : int_id      Interrupt for which vector will be set.
*
*               prio        Priority to assign
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntPrioSet (CPU_DATA    int_id,
                      CPU_INT08U  prio)
{
    CPU_SR_ALLOC();


    if (int_id < BSP_INT_SRC_NBR) {
        CPU_CRITICAL_ENTER();
        CPU_IntSrcPrioSet(int_id + 16, prio);
        CPU_CRITICAL_EXIT();
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           INTERNAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              BSP_IntInit()
*
* Description : Initialize interrupts:
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntInit (void)
{
    CPU_DATA  int_id;


    for (int_id = 0; int_id < BSP_INT_SRC_NBR; int_id++) {
        BSP_IntVectSet(int_id, BSP_IntHandlerDummy);
    }
}


/*
*********************************************************************************************************
*                                        BSP_IntHandler####()
*
* Description : Handle an interrupt.
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

void  BSP_IntHandlerBOD                (void)  { BSP_IntHandler(BSP_INT_ID_BOD);                 }
void  BSP_IntHandlerIRC                (void)  { BSP_IntHandler(BSP_INT_ID_IRC);                 }
void  BSP_IntHandlerPWRWU              (void)  { BSP_IntHandler(BSP_INT_ID_PWRWU);               }
void  BSP_IntHandlerSRAMF              (void)  { BSP_IntHandler(BSP_INT_ID_SRAMF);               }
void  BSP_IntHandlerCLKF               (void)  { BSP_IntHandler(BSP_INT_ID_CLKF);                }
void  BSP_IntHandlerDefault            (void)  { BSP_IntHandler(BSP_INT_ID_Default);             }
void  BSP_IntHandlerRTC                (void)  { BSP_IntHandler(BSP_INT_ID_RTC);                 }
void  BSP_IntHandlerTAMPER             (void)  { BSP_IntHandler(BSP_INT_ID_TAMPER);              }
void  BSP_IntHandlerEINT0              (void)  { BSP_IntHandler(BSP_INT_ID_EINT0);               }
void  BSP_IntHandlerEINT1              (void)  { BSP_IntHandler(BSP_INT_ID_EINT1);               }
void  BSP_IntHandlerEINT2              (void)  { BSP_IntHandler(BSP_INT_ID_EINT2);               }
void  BSP_IntHandlerEINT3              (void)  { BSP_IntHandler(BSP_INT_ID_EINT3);               }
void  BSP_IntHandlerEINT4              (void)  { BSP_IntHandler(BSP_INT_ID_EINT4);               }
void  BSP_IntHandlerEINT5              (void)  { BSP_IntHandler(BSP_INT_ID_EINT5);               }
void  BSP_IntHandlerEINT6              (void)  { BSP_IntHandler(BSP_INT_ID_EINT6);               }
void  BSP_IntHandlerEINT7              (void)  { BSP_IntHandler(BSP_INT_ID_EINT7);               }
void  BSP_IntHandlerGPIOA              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOA);               }
void  BSP_IntHandlerGPIOB              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOB);               }
void  BSP_IntHandlerGPIOC              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOC);               }
void  BSP_IntHandlerGPIOD              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOD);               }
void  BSP_IntHandlerGPIOE              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOE);               }
void  BSP_IntHandlerGPIOF              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOF);               }
void  BSP_IntHandlerGPIOG              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOG);               }
void  BSP_IntHandlerGPIOH              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOH);               }
void  BSP_IntHandlerGPIOI              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOI);               }
void  BSP_IntHandlerGPIOJ              (void)  { BSP_IntHandler(BSP_INT_ID_GPIOJ);               }
void  BSP_IntHandlerTIMER0             (void)  { BSP_IntHandler(BSP_INT_ID_TIMER0);              }
void  BSP_IntHandlerTIMER1             (void)  { BSP_IntHandler(BSP_INT_ID_TIMER1);              }
void  BSP_IntHandlerTIMER2             (void)  { BSP_IntHandler(BSP_INT_ID_TIMER2);              }
void  BSP_IntHandlerTIMER3             (void)  { BSP_IntHandler(BSP_INT_ID_TIMER3);              }
void  BSP_IntHandlerPDMA               (void)  { BSP_IntHandler(BSP_INT_ID_PDMA);                }
void  BSP_IntHandlerADC                (void)  { BSP_IntHandler(BSP_INT_ID_ADC);                 }
void  BSP_IntHandlerWDT                (void)  { BSP_IntHandler(BSP_INT_ID_WDT);                 }
void  BSP_IntHandlerWWDT               (void)  { BSP_IntHandler(BSP_INT_ID_WWDT);                }
void  BSP_IntHandlerEADC0              (void)  { BSP_IntHandler(BSP_INT_ID_EADC0);               }
void  BSP_IntHandlerEADC1              (void)  { BSP_IntHandler(BSP_INT_ID_EADC1);               }
void  BSP_IntHandlerEADC2              (void)  { BSP_IntHandler(BSP_INT_ID_EADC2);               }
void  BSP_IntHandlerEADC3              (void)  { BSP_IntHandler(BSP_INT_ID_EADC3);               }
void  BSP_IntHandlerACMP0              (void)  { BSP_IntHandler(BSP_INT_ID_ACMP0);               }
void  BSP_IntHandlerOPA0               (void)  { BSP_IntHandler(BSP_INT_ID_OPA0);                }
void  BSP_IntHandlerOPA1               (void)  { BSP_IntHandler(BSP_INT_ID_OPA1);                }
void  BSP_IntHandlerICAP0              (void)  { BSP_IntHandler(BSP_INT_ID_ICAP0);               }
void  BSP_IntHandlerICAP1              (void)  { BSP_IntHandler(BSP_INT_ID_ICAP1);               }
void  BSP_IntHandlerPWMA0              (void)  { BSP_IntHandler(BSP_INT_ID_PWMA0);               }
void  BSP_IntHandlerPWMA1              (void)  { BSP_IntHandler(BSP_INT_ID_PWMA1);               }
void  BSP_IntHandlerPWMA2              (void)  { BSP_IntHandler(BSP_INT_ID_PWMA2);               }
void  BSP_IntHandlerPWMA3              (void)  { BSP_IntHandler(BSP_INT_ID_PWMA3);               }
void  BSP_IntHandlerPWMA4              (void)  { BSP_IntHandler(BSP_INT_ID_PWMA4);               }
void  BSP_IntHandlerPWMA5              (void)  { BSP_IntHandler(BSP_INT_ID_PWMA5);               }
void  BSP_IntHandlerPWMABRK            (void)  { BSP_IntHandler(BSP_INT_ID_PWMABRK);             }
void  BSP_IntHandlerQEI0               (void)  { BSP_IntHandler(BSP_INT_ID_QEI0);                }
void  BSP_IntHandlerPWMB0              (void)  { BSP_IntHandler(BSP_INT_ID_PWMB0);               }
void  BSP_IntHandlerPWMB1              (void)  { BSP_IntHandler(BSP_INT_ID_PWMB1);               }
void  BSP_IntHandlerPWMB2              (void)  { BSP_IntHandler(BSP_INT_ID_PWMB2);               }
void  BSP_IntHandlerPWMB3              (void)  { BSP_IntHandler(BSP_INT_ID_PWMB3);               }
void  BSP_IntHandlerPWMB4              (void)  { BSP_IntHandler(BSP_INT_ID_PWMB4);               }
void  BSP_IntHandlerPWMB5              (void)  { BSP_IntHandler(BSP_INT_ID_PWMB5);               }
void  BSP_IntHandlerPWMBBRK            (void)  { BSP_IntHandler(BSP_INT_ID_PWMBBRK);             }
void  BSP_IntHandlerQEI1               (void)  { BSP_IntHandler(BSP_INT_ID_QEI1);                }
void  BSP_IntHandlerEPWMA              (void)  { BSP_IntHandler(BSP_INT_ID_EPWMA);               }
void  BSP_IntHandlerEPWMABRK           (void)  { BSP_IntHandler(BSP_INT_ID_EPWMABRK);            }
void  BSP_IntHandlerEPWMB              (void)  { BSP_IntHandler(BSP_INT_ID_EPWMB);               }
void  BSP_IntHandlerEPWMBBRK           (void)  { BSP_IntHandler(BSP_INT_ID_EPWMBBRK);            }
void  BSP_IntHandlerUSBD               (void)  { BSP_IntHandler(BSP_INT_ID_USBD);                }
void  BSP_IntHandlerUSBH               (void)  { BSP_IntHandler(BSP_INT_ID_USBH);                }
void  BSP_IntHandlerUSB_OTG            (void)  { BSP_IntHandler(BSP_INT_ID_USB_OTG);             }
void  BSP_IntHandlerEMAC_TX            (void)  { BSP_IntHandler(BSP_INT_ID_EMAC_TX);             }
void  BSP_IntHandlerEMAC_RX            (void)  { BSP_IntHandler(BSP_INT_ID_EMAC_RX);             }
void  BSP_IntHandlerSPI0               (void)  { BSP_IntHandler(BSP_INT_ID_SPI0);                }
void  BSP_IntHandlerSPI1               (void)  { BSP_IntHandler(BSP_INT_ID_SPI1);                }
void  BSP_IntHandlerSPI2               (void)  { BSP_IntHandler(BSP_INT_ID_SPI2);                }
void  BSP_IntHandlerSPI3               (void)  { BSP_IntHandler(BSP_INT_ID_SPI3);                }
void  BSP_IntHandlerUART0              (void)  { BSP_IntHandler(BSP_INT_ID_UART0);               }
void  BSP_IntHandlerUART1              (void)  { BSP_IntHandler(BSP_INT_ID_UART1);               }
void  BSP_IntHandlerUART2              (void)  { BSP_IntHandler(BSP_INT_ID_UART2);               }
void  BSP_IntHandlerUART3              (void)  { BSP_IntHandler(BSP_INT_ID_UART3);               }
void  BSP_IntHandlerUART4              (void)  { BSP_IntHandler(BSP_INT_ID_UART4);               }
void  BSP_IntHandlerUART5              (void)  { BSP_IntHandler(BSP_INT_ID_UART5);               }
void  BSP_IntHandlerI2C0               (void)  { BSP_IntHandler(BSP_INT_ID_I2C0);                }
void  BSP_IntHandlerI2C1               (void)  { BSP_IntHandler(BSP_INT_ID_I2C1);                }
void  BSP_IntHandlerI2C2               (void)  { BSP_IntHandler(BSP_INT_ID_I2C2);                }
void  BSP_IntHandlerI2C3               (void)  { BSP_IntHandler(BSP_INT_ID_I2C3);                }
void  BSP_IntHandlerI2C4               (void)  { BSP_IntHandler(BSP_INT_ID_I2C4);                }
void  BSP_IntHandlerSC0                (void)  { BSP_IntHandler(BSP_INT_ID_SC0);                 }
void  BSP_IntHandlerSC1                (void)  { BSP_IntHandler(BSP_INT_ID_SC1);                 }
void  BSP_IntHandlerSC2                (void)  { BSP_IntHandler(BSP_INT_ID_SC2);                 }
void  BSP_IntHandlerSC3                (void)  { BSP_IntHandler(BSP_INT_ID_SC3);                 }
void  BSP_IntHandlerSC4                (void)  { BSP_IntHandler(BSP_INT_ID_SC4);                 }
void  BSP_IntHandlerSC5                (void)  { BSP_IntHandler(BSP_INT_ID_SC5);                 }
void  BSP_IntHandlerCAN0               (void)  { BSP_IntHandler(BSP_INT_ID_CAN0);                }
void  BSP_IntHandlerCAN1               (void)  { BSP_IntHandler(BSP_INT_ID_CAN1);                }
void  BSP_IntHandlerI2S0               (void)  { BSP_IntHandler(BSP_INT_ID_I2S0);                }
void  BSP_IntHandlerI2S1               (void)  { BSP_IntHandler(BSP_INT_ID_I2S1);                }
void  BSP_IntHandlerSD                 (void)  { BSP_IntHandler(BSP_INT_ID_SD);                  }
void  BSP_IntHandlerPS2D               (void)  { BSP_IntHandler(BSP_INT_ID_PS2D);                }
void  BSP_IntHandlerCRYPTO             (void)  { BSP_IntHandler(BSP_INT_ID_CRYPTO);              }
void  BSP_IntHandlerCRC                (void)  { BSP_IntHandler(BSP_INT_ID_CRC);                 }

/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          BSP_IntHandler()
*
* Description : Central interrupt handler.
*
* Argument(s) : int_id          Interrupt that will be handled.
*
* Return(s)   : none.
*
* Caller(s)   : ISR handlers.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_IntHandler (CPU_DATA  int_id)
{
    CPU_FNCT_VOID  isr;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();                                       /* Tell the OS that we are starting an ISR            */

    OSIntEnter();

    CPU_CRITICAL_EXIT();

    if (int_id < BSP_INT_SRC_NBR) {
        isr = BSP_IntVectTbl[int_id];
        if (isr != (CPU_FNCT_VOID)0) {
            isr();
        }
    }

    OSIntExit();                                                /* Tell the OS that we are leaving the ISR            */
}


/*
*********************************************************************************************************
*                                        BSP_IntHandlerDummy()
*
* Description : Dummy interrupt handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_IntHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_IntHandlerDummy (void)
{

}
