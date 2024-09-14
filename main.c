/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* SPDX-License-Identifier: Apache-2.0                                                                     */
/* Copyright(c) 2023 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include "numicro_8051.h"
#include "led.h"

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
int counter = 0;


/********** current variant **********/
/*
 * One digit:                          TABLE:
 *   ***A***                   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  -  h
 *   *     *         (F) PA1   0  1  1  1  0  0  0  1  0  0  0  0  0  1  0  0  1  0
 *   F     B         (B) PB4   0  0  0  0  0  1  1  0  0  0  0  1  1  0  1  1  1  1
 *   *     *         (A) PB5   0  1  0  0  1  0  0  0  0  0  0  1  0  1  0  0  1  1
 *   ***G***         (G) PC3   1  1  0  0  0  0  0  1  0  0  0  0  1  0  0  0  0  0
 *   *     *         (C) PC4   0  0  1  0  0  0  0  0  0  0  0  0  1  0  1  1  1  0
 *   E     C         (DP)PC5   1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1
 *   *     *   **    (D) PC6   0  1  0  0  1  0  0  1  0  0  1  0  0  0  0  1  1  1
 *   ***D***  *DP*   (E) PC7   0  1  0  1  1  1  0  1  0  1  0  0  0  0  0  0  1  0
 *             **
 */

/*
 * Number of digit on indicator with common anode
 * digis 0..3: PA3, PD6, PD4, PD1
 */

/************* arrays for ports *************/
//static const U8 LED_bits[18] = {0xfe,0x30,0xed,0xf9,0x33,0xdb,0xdf,0xf0,0xff,0xfb,
//													0xf7,0x3f,0x0d,0x3d,0xcf,0xe7,0x01,0};
//
//static U8 display_buffer[6] = {' ',' ',' ',' ',' ',' '}; // blank by default
//U8 N_current = 0; // current digit to display

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


void all_digits_off(void) {
    P12 = 1; // Digit 1
    P01 = 1; // Digit 2
    P04 = 0; // Digit 3
    P11 = 0; // Digit 4
    P03 = 1; // Digit 5
    P00 = 1; // Digit 6
}

/**
 * fills buffer to display
 * @param str - string to display, contains "0..f" for digits, " " for space, "." for DP
 * 				for example: " 1.22" or "h1ab" (something like "0...abc" equivalent to "0.abc"
 * 				register independent!
 * 			any other letter would be omitted
 * 			if NULL - fill buffer with spaces
 */
//void set_display_buf(char *str){
//	U8 B[4];
//	signed char ch, M = 0, i;
//	//N_current = 0; // refresh current digit number
//	// empty buffer
//	for(i = 0; i < 6; i++)
//		display_buffer[i] = ' ';
//	if(!str) return;
//	i = 0;
//	for(;(ch = *str) && (i < 6); str++){
//		M = 0;
//		if(ch > '/' && ch < ':'){ // digit
//			M = '0';
//		}else if(ch > '`' & ch < 'g'){ // a..f
//			M = 'a' - 10;
//		}else if(ch > '@' & ch < 'G'){ // A..F
//			M = 'A' - 10;
//		}else if(ch == '-'){ // minus
//			M = '-' - 16;
//		}else if(ch == 'h'){ // hex
//			M = 'h' - 17;
//		}else if(ch == 'H'){ // hex
//			M = 'H' - 17;
//		}else if(ch == '.'){ // DP, set it to previous char
//			if(i == 0){ // word starts from '.' - make a space with point
//				B[0] = 0xff;
//			}else{ // set point for previous character
//				B[i-1] |= 0x80;
//			}
//			continue;
//		}else if(ch != ' '){ // bad character - continue
//			continue;
//		}
//		B[i] = ch - M;
//		i++;
//	}
//	// now make align to right
//	ch = 5;
//	for(M = i-1; M > -1; M--, ch--){
//		display_buffer[ch] = B[M];
//	}
//}

/**
 * Turn on anode power for digit N (0..3: PA3, PD6, PD4, PD1 -- A0x08, D0x40, D0x10, D0x02)
 * @param N - number of digit (0..3), if other - no action (display off)
 * @return
 */
//void light_up_digit(U8 N){
//	switch(N){
//		case 0:
//			 P12 = 1; // Digit 1
//		break;
//		case 1:
//			 P01 = 1; // Digit 2
//		break;
//		case 2:
//			 P04 = 0; // Digit 3
//	  break;
//		case 3:
//			 P11 = 0; // Digit 4
//		break;
//		case 4:
//			 P03 = 1; // Digit 5
//		break;
//		case 5:
//			 P00 = 1; // Digit 6
//		break;
//	}
//}
//
///**
// * convert integer value in into string and display it
// * @param i - value to display, -999999 <= i <= 999999, if wrong, displays "---E"
// */
//void display_int(S32 I){
//	int rem = 0;
//	signed char i;
//	signed char sign = 0;
////	if(I < -999999 || I > 999999){
////		set_display_buf("---E");
////		return;
////	}
//	set_display_buf(NULL); // empty buffer
//	for(i = 0; i < 6; i++)
//		display_buffer[i] = 0;
//	if(I == 0){ // just show zero
//		display_buffer[3] = 0;
//		return;
//	}
//	if(I < 0){
//		sign = 1;
//		I *= -1;
//	}
//	for (i = 0; i < 6; i++)
//	{
////		rem = I % 10;
//		rem = I - (I/10);
//		display_buffer[5-i] = rem; //rem;
//		I = I/10;
//		if(I == 0) break;
//	}
//	if(sign && i < 6) display_buffer[5-i] = 16; // minus sign
//}


void main (void) 
{
	unsigned char i;
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

    all_digits_off();
    display_int(4);

    P12 = 0; // Digit 1
	P01 = 0; // Digit 2
	P04 = 1; // Digit 3
	P11 = 1; // Digit 4
	P03 = 0; // Digit 5
	P00 = 0; // Digit 6

	P10 = 0;
    for(i = 0; i < 16; i++) {
	  P06 ^=1;
	  P10 = (i&2)/2;
	}
    while(1)
    {


//      display_int(counter++);

//	  P0 = ~P0;
//	  P1 = ~P1;
//	  P30 ^= 1;
//      for(i = 0; i < 16; i++) {
//    	  P06 ^=1;
//      }
//	  P10 ^= 1;
//	  P04 ^= 1;
	  P14 ^= 1;
      for(i = 0; i < 6; i++) {
    	  Timer2_Delay(24000000,4,200,3);
//		  Timer2_Delay(24000000,4,200,1000);
    	  show_next_digit();
      }
   }

}



