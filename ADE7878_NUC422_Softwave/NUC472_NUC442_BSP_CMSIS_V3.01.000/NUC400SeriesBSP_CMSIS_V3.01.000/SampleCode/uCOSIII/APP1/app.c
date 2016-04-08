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

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : EHS
*                 DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <includes.h>

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

                                                                /* ----------------- APPLICATION GLOBALS ------------------ */
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

OS_FLAG_GRP  	SecureEventFlagGrp;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart          (void     *p_arg);
static  void  AppTaskCreate         (void);
static  void  AppObjCreate          (void);

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
/* Sample demo for uC/OS-III, which demonstrates the crypto function 
    under uC/OS-III. */
int main(void)
{
    OS_ERR  err;

	SYS_UnlockReg();

    BSP_Init();                                                 /* Initialize BSP functions                             */

    BSP_Set_PLL_Freq(CLK_XTL_12M, PLL_DEFAULT_HZ);				/* Configure PLL                                        */
    BSP_Set_System_Clock(CLK_PLL, 0);                      		/* Select HCLK source clock as PLL                      */
    
    SYS_LockReg();

    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */

    OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE  )AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
    
    (void)&err;
    
    return (0);
}

void CRYPTO_IRQHandler()
{
	OS_ERR  err;

	if (CRPT->INTSTS & CRPT_INTSTS_PRNGIF_Msk)
	{
		OSFlagPost(&SecureEventFlagGrp, 0x1, OS_OPT_POST_FLAG_SET, &err);
		CRPT->INTSTS = CRPT_INTSTS_PRNGIF_Msk;
	}
}


void  dump_PRNG()
{
	uint32_t    au32PrngData[8];
	
	PRNG_Read(au32PrngData);
	bsp_printf("PRNG DATA ==>\n");
	bsp_printf("    0x%08x  0x%08x  0x%08x  0x%08x\n", au32PrngData[0], au32PrngData[1], au32PrngData[2], au32PrngData[3]);
	bsp_printf("    0x%08x  0x%08x  0x%08x  0x%08x\n", au32PrngData[4], au32PrngData[5], au32PrngData[6], au32PrngData[7]);
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    OS_ERR      err;
    CPU_TS		ts;

    APP_TRACE_DBG(("Creating Application kernel objects\n\r"));
    AppObjCreate();                                             /* Create Applicaiton kernel objects                    */
    
    APP_TRACE_DBG(("Creating Application Tasks\n\r"));
    AppTaskCreate();                                            /* Create Application tasks                             */

	OSFlagCreate(&SecureEventFlagGrp, "PRNG_DONE", 0, &err);

    /* Enable Crypto clock */        
    CLK->AHBCLK |= CLK_AHBCLK_CRPTCKEN_Msk;

	BSP_IntVectSet(BSP_INT_ID_CRYPTO, CRYPTO_IRQHandler);
	BSP_IntEn(BSP_INT_ID_CRYPTO);
	PRNG_ENABLE_INT();

	while (1)
	{
		PRNG_Open(PRNG_KEY_SIZE_256, 0, 0);
		bsp_printf("Start PRNG...\n");
		PRNG_Start();
		OSFlagPend(&SecureEventFlagGrp, 0x1, 0, OS_OPT_PEND_FLAG_SET_ALL, &ts, &err);
		dump_PRNG();
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_DLY, &err);
    }
}

/*
*********************************************************************************************************
*                                          AppTaskCreate()
*
* Description : Create application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{

}


/*
*********************************************************************************************************
*                                          AppObjCreate()
*
* Description : Create application kernel objects tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{

}

