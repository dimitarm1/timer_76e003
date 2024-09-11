/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* SPDX-License-Identifier: Apache-2.0                                                                     */
/* Copyright(c) 2023 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include "numicro_8051.h"

#define KEY_0_PRESSED  0x01
#define KEY_1_PRESSED  0x02
#define KEY_2_PRESSED  0x04
#define KEY_3_PRESSED  0x08
#define KEY_PRESSED    0x0F

#define KEY_0_RELEASED 0x10
#define KEY_1_RELEASED 0x20
#define KEY_2_RELEASED 0x40
#define KEY_3_RELEASED 0x80
#define KEY_RELEASED   0xF0


unsigned short keys_scan_buffer[4];
unsigned char key_state;

unsigned char scan_keys(void)
{
	unsigned char i, result = 0;
	keys_scan_buffer[0] = (keys_scan_buffer[0]<<1) | (!P15); // start)
	keys_scan_buffer[1] = (keys_scan_buffer[1]<<1) | (!P30); // stop
	keys_scan_buffer[2] = (keys_scan_buffer[2]<<1) | (!P07); // Dn
	keys_scan_buffer[3] = (keys_scan_buffer[3]<<1) | (!P13); // Up
	for(i = 0; i<4; i++)
	{
		if(keys_scan_buffer[i] == 0xffff)
		{
			if(!(key_state & (1<<i)))
			{
				key_state = key_state | (1<<i);
				result = result | (0x01<<i);
			}
		}
		if(keys_scan_buffer[i] == 0)
		{
			if((key_state & (1<<i)))
			{
				key_state = key_state & (~((unsigned char)(0x11<<i)));
				result = result | (0x10<<i);
			}
		}

	}
	return result;
}


void main (void) 
{
/* UART0 initial setting
  * include sys.c in Library for modify HIRC value to 24MHz
  * include uart.c in Library for UART initial setting
*/
//    MODIFY_HIRC(HIRC_166);
//    Enable_UART0_VCOM_printf_166M_115200();

//    printf("\n\r Hello world!");

    ALL_GPIO_QUASI_MODE; // All GPIO are disabled

    P06_PUSHPULL_MODE; // Temporary serial clock
    P10_PUSHPULL_MODE; // Shift data out

    P14_PUSHPULL_MODE; // Led Fan (green)
    P16_PUSHPULL_MODE; // Led Lampi (red)


    // Digits Common Anode
    P12_PUSHPULL_MODE; // Digit 1
    P01_PUSHPULL_MODE; // Digit 2
    P04_PUSHPULL_MODE; // Digit 3
    P11_PUSHPULL_MODE; // Digit 4
    P03_PUSHPULL_MODE; // Digit 5
    P00_PUSHPULL_MODE; // Digit 6
    // Keyboard
    P15_INPUT_MODE; // Key1
    P30_INPUT_MODE; // Key2
    P07_INPUT_MODE; // Key3
    P13_INPUT_MODE; // Key4

    P12 = 0; // Digit 1
    P01 = 0; // Digit 2
    P04 = 0; // Digit 3
    P11 = 1; // Digit 4
    P03 = 0; // Digit 5
    P00 = 0; // Digit 6

    while(1)
    {
      unsigned char i;
//	  P0 = ~P0;
//	  P1 = ~P1;
//	  P30 ^= 1;
      for(i = 0; i < 16; i++) {
    	  P06 ^=1;
      }
	  P10 ^= 1;
	  P14 ^= 1;
	  Timer2_Delay(24000000,4,200,1000);
   }

}



