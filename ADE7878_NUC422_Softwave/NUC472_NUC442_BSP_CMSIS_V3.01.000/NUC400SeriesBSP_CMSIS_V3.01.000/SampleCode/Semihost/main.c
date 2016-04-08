/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 14/05/29 1:14p $
 * @brief    Sample program for NUC472/NUC442 MCU
 *           This sample code demo semihost function
 *
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NUC472_442.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/




void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    // TODO: Configure system clock


    /* Lock protected registers */
    SYS_LockReg();

}

int main()
{
    int8_t item;

    SYS_Init();

    printf("\n Start SEMIHOST test: \n");

    while(1) {
        item = getchar();
        printf("%c\n",item);
    }

}

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
