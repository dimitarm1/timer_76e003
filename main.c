/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* SPDX-License-Identifier: Apache-2.0                                                                     */
/* Copyright(c) 2023 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include "numicro_8051.h"


void main (void) 
{
/* UART0 initial setting
  * include sys.c in Library for modify HIRC value to 24MHz
  * include uart.c in Library for UART initial setting
*/
//    MODIFY_HIRC(HIRC_166);
    Enable_UART0_VCOM_printf_166M_115200();

//    printf("\n\r Hello world!");

    ALL_GPIO_QUASI_MODE;
    P14_PUSHPULL_MODE;
    P13_INPUT_MODE;
    while(1)
    {
//	  P0 = ~P0;
//	  P1 = ~P1;
//	  P30 ^= 1;
	  P14 ^= 1;
	  Timer2_Delay(24000000,4,200,1000);
   }

}



