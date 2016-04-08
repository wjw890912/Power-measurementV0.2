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
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdio.h>
#include  <string.h>
#include  <app_cfg.h>
#include  <includes.h>

#include  "crypto.h"

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

#define TASK_STACK_SIZE			256

#define TASK_1_PRIO				10
#define TASK_2_PRIO				11

                                                                /* ----------------- APPLICATION GLOBALS ------------------ */
static  OS_STK          StackOfTask_1[TASK_STACK_SIZE];
static  OS_STK          StackOfTask_2[TASK_STACK_SIZE];

OS_FLAG_GRP  			*pSecureEventFlagGrp;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTask_2                 (void        *p_arg);
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

    BSP_Set_PLL_Freq(CLK_XTL_12M, PLL_DEFAULT_HZ);				/* Configure PLL                                        */
    BSP_Set_System_Clock(CLK_PLL, 0);                      		/* Select HCLK source clock as PLL                      */
    
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

    OSTaskCreateExt((void (*)(void *)) AppTask_2,           /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&StackOfTask_2[TASK_STACK_SIZE - 1],
                    (INT8U           ) TASK_2_PRIO,
                    (INT16U          ) TASK_2_PRIO,
                    (OS_STK         *)&StackOfTask_2[0],
                    (INT32U          ) TASK_STACK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(TASK_1_PRIO, "Task 1", &err);
    OSTaskNameSet(TASK_2_PRIO, "Task 2", &err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */
    
    return (1);
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTask_2()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void CRYPTO_IRQHandler()
{
	INT8U  err;

	if (CRPT->INTSTS & CRPT_INTSTS_PRNGIF_Msk)
	{
		OSFlagPost(pSecureEventFlagGrp, 0x1, OS_FLAG_SET, &err);
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


static  void  AppTask_2 (void *p_arg)
{
	INT8U  err;
	
   	(void)p_arg;

    APP_TRACE_INFO(("AppTask_2\n\r"));

	pSecureEventFlagGrp = OSFlagCreate(0x0, &err);

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
		OSFlagPend(pSecureEventFlagGrp, 0x1, OS_FLAG_WAIT_SET_ANY, 0, &err);
		dump_PRNG();
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
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
   (void)p_arg;


    APP_TRACE_INFO(("AppTask_1\n\r"));
    
    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
    	APP_TRACE_INFO(("Task1\n"));
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
}



