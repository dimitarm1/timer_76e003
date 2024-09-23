/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* SPDX-License-Identifier: Apache-2.0                                                                     */
/* Copyright(c) 2023 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include "numicro_8051.h"

/*
//----------------------------------------------------------------------------------------------//
void main (void)
{
    // UART0 settting for printf function 
    MODIFY_HIRC(HIRC_166);
//    Enable_UART0_VCOM_printf_166M_115200();
//    printf ("\n Test start ...");

    ALL_GPIO_QUASI_MODE;
    P12_PUSHPULL_MODE;
    P13_INPUT_MODE;

    while(1)
    {
      P0 = ~P0;
      P1 = ~P1;
      P30 ^= 1;
      P1 |= SET_BIT2;
      if (!(P1&SET_BIT3))
      {
        CALL_NOP;
      }
     

      P1 &= CLR_BIT2;
      if (P1&SET_BIT3)
      {
        CALL_NOP;
      }
     
    }
}

*/


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


#define BUTTON_MINUS   0x01
#define BUTTON_PLUS    0x02
#define BUTTON_START   0x04
#define BUTTON_STOP    0x08

#define STATE_NONE					0
#define STATE_WAITING   		1
#define STATE_WORKING   		2
#define STATE_COOLING   		3
#define STATE_DISPLAY_HOURS 4

#define DEFAULT_COOL_TIME_SEC	(3*60)
#define DEFAULT_WAIT_TIME_SEC (9*60)
#define ROLL_OVER_VALUE					336


unsigned short keys_scan_buffer[4];
unsigned char key_state;
short counter = 0;
int seconds_counter = 0;
int pre_time;
int main_time;
int cool_time;
U8 last_key;
U8 new_key;
U8 buzz_counter;
U8 current_state;
U8 finished;
U8 refresh_display;


typedef union {
	char data_bytes[4];
	int time;
} timedata_t;

timedata_t timedata;


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
//				result = result | (0x10<<i);
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


// Unused
void start_pressed() {
	
	if(current_state == STATE_WAITING) {
		current_state = STATE_WORKING;
		P14 = 1; // Fan is on
		//P16 = 1; // Relay lampi is on
	}
	if(current_state == STATE_NONE) {
		if(pre_time) {
			current_state = STATE_WAITING;
		}
		else {
			display_int_sec(timedata.time);
		}
		return;
	}
}


#if defined __C51__
void Timer0_ISR (void) interrupt 1              // vector=0x0B 

#elif defined __ICC8051__
#pragma vector=0x0B                             // Interrupt 1 
__interrupt void Timer0_ISR(void)

#elif defined __SDCC__
void Timer0_ISR (void) __interrupt (1)          // vector=0x0B
#endif
{
    SFRS_TMP = SFRS;              /* for SFRS page */
    SFRS = 0;
/* following setting for reload Timer 0 counter */
    TH0 = TH0TMP;
    TL0 = TL0TMP;
/* following clear flag for next interrupt */
    clr_TCON_TF0;
    show_next_digit();
    counter++;
    if(buzz_counter) {
    	buzz_counter--;
    	if(!buzz_counter) {
    		P05 = 1;
    	}
    }
    if(counter > ROLL_OVER_VALUE) {
    	counter = 0;
			
    	P14 ^= 1;
			if(current_state != STATE_NONE) {
				refresh_display = 1;
				if(pre_time) {
					pre_time--;
					if(!pre_time) {
						if(main_time) {
							P14 = 1; // Fan is on
							//P16 = 1; // Relay lampi is on
							current_state = STATE_WORKING;
						}
						else {
							if(!cool_time) {
								cool_time = DEFAULT_COOL_TIME_SEC;
							}
							P14 = 1; // Fan is on
							//P16 = 0; // Relay lampi is off
							current_state = STATE_COOLING;
						}
					}
				}
				else if(main_time) {
					main_time--;
					seconds_counter++;
					if(!main_time) {
						if(!cool_time) {
							cool_time = DEFAULT_COOL_TIME_SEC;
						}
						P14 = 1; // Fan is on
						//P16 = 0; // Relay lampi is off
						current_state = STATE_COOLING;
					}
				}
				else if(cool_time) {
					cool_time--;
					if(!cool_time) {
						P14 = 0; // Fan is off
					  //P16 = 0; // Relay lampi is off
						// Write data to EEPROM
						current_state = STATE_NONE;
						finished = 1;
					}				
				}
			}	
    }
		if(refresh_display){
			refresh_display = 0;
			if(current_state == STATE_WORKING){
				display_int_sec(main_time);
			}
			else if(current_state == STATE_WAITING){
				display_int_sec(pre_time);
			}
			else if(current_state == STATE_COOLING){
				display_int_sec(cool_time);
			}
			else if(current_state == STATE_NONE){
				display_int_sec(0);
			}
		}
		
    if (SFRS_TMP)                 /* for SFRS page */
    {
      ENABLE_SFR_PAGE1;
    }
}


void main (void) 
{
		unsigned char i;
		unsigned int j;
		MODIFY_HIRC(HIRC_16);

		seconds_counter = 0;
		last_key = 0;

    ALL_GPIO_QUASI_MODE; // All GPIO are disabled

    P06_PUSHPULL_MODE; // Temporary serial clock
    P10_PUSHPULL_MODE; // Shift data out

    P14_PUSHPULL_MODE; // Led Fan (green)
//    P16_PUSHPULL_MODE; // Led Lampi (red)


    // Digits Common Anode
    P01_PUSHPULL_MODE; // Digit 1
    P12_PUSHPULL_MODE; // Digit 2
    P04_PUSHPULL_MODE; // Digit 3
    P11_PUSHPULL_MODE; // Digit 4
    P03_PUSHPULL_MODE; // Digit 5
    P00_PUSHPULL_MODE; // Digit 6

    P05_PUSHPULL_MODE; // Buzzer
    // Keyboard
    P15 = 1; // Key1
    P30 = 1; // Key2
    P07 = 1; // Key3
    P13 = 1; // Key4

    all_digits_off();
    display_int(0);

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

		if(P15 && P30) { //new_key == (BUTTON_STOP | BUTTON_START)){
			// Reset counter in flash
			buzz_counter = 250;
			finished = 255;
		}
		else {
			finished = 0;
		}
		Timer0_AutoReload_Interrupt_Initial(24,2000);
    ENABLE_GLOBAL_INTERRUPT;
			
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
//	  P14 ^= 1;
      for(j = 0; j < 640; j++) {
    	  ;
      }

      new_key = scan_keys();
      if(new_key != last_key) {
				DISABLE_GLOBAL_INTERRUPT;
    	  last_key = new_key;
    	  if( new_key) {
//    		  seconds_counter++;
    		  buzz_counter = 25;
    		  P05 = 0; // Buzzer on
    		  switch (new_key) {
    		  case BUTTON_START:
					{
						if(current_state == STATE_NONE) {
							if(main_time) {								
								current_state = STATE_WAITING;
								pre_time = DEFAULT_WAIT_TIME_SEC;
								counter = ROLL_OVER_VALUE/4;
								display_int_sec(pre_time);
							}
							else {
						
								// Read EEPROM
#if defined __C51__
								timedata.data_bytes[0] = Read_APROM_BYTE(0x38FB);
								timedata.data_bytes[1] = Read_APROM_BYTE(0x38FC);
								timedata.data_bytes[2] = Read_APROM_BYTE(0x38FD);
								timedata.data_bytes[3] = Read_APROM_BYTE(0x38FE);
#elif defined __ICC8051__
								timedata.data_bytes[0] = Read_APROM_BYTE((uint16_t __code *)0x38FB);
								timedata.data_bytes[1] = Read_APROM_BYTE((uint16_t __code *)0x38FC);
								timedata.data_bytes[2] = Read_APROM_BYTE((uint16_t __code *)0x38FD);
								timedata.data_bytes[3] = Read_APROM_BYTE((uint16_t __code *)0x38FE);
#elif defined __SDCC__
								timedata.data_bytes[0] = Read_APROM_BYTE((uint16_t __code*)0x38FB);
								timedata.data_bytes[1] = Read_APROM_BYTE((uint16_t __code*)0x38FC);
								timedata.data_bytes[2] = Read_APROM_BYTE((uint16_t __code*)0x38FD);
								timedata.data_bytes[3] = Read_APROM_BYTE((uint16_t __code*)0x38FE);
#endif
								display_int_sec(timedata.time);
							}
						}
						else if(current_state == STATE_WAITING) {
							pre_time = 0;
							current_state = STATE_WORKING;
							P14 = 1; // Fan is on
						//P16 = 1; // Relay lampi is on
						}
					}						
   			  break;
    		  case BUTTON_STOP:
					{
						if(current_state == STATE_NONE || current_state == STATE_WAITING) {
							pre_time = 0;
							main_time = 0;
							cool_time = 0;
							current_state = STATE_NONE;
							display_int_sec(main_time);
						}
						else if(current_state == STATE_WORKING) {
							pre_time = 0;
							main_time = 0;
							current_state = STATE_COOLING;
							cool_time = DEFAULT_COOL_TIME_SEC;
							display_int_sec(cool_time);
						}
					}
				  break;
    		  case BUTTON_MINUS:
					{
						if(current_state == STATE_NONE && main_time < 30*60) {
							main_time = main_time + 60;
							display_int_sec(main_time);
						}
					}
				  break;
    		  case BUTTON_PLUS:						
					{
    			  if(current_state == STATE_NONE && main_time > 59) {
    				  main_time = main_time - 60;
							display_int_sec(main_time);
    			  }
					}
				  break;
    		  }
    		 
    	  }
				 ENABLE_GLOBAL_INTERRUPT;
      }
			if(finished) {
#if defined __C51__
				timedata.data_bytes[0] = Read_APROM_BYTE(0x38FB);
				timedata.data_bytes[1] = Read_APROM_BYTE(0x38FC);
				timedata.data_bytes[2] = Read_APROM_BYTE(0x38FD);
				timedata.data_bytes[3] = Read_APROM_BYTE(0x38FE);
#elif defined __ICC8051__
				timedata.data_bytes[0] = Read_APROM_BYTE((uint16_t __code *)0x38FB);
				timedata.data_bytes[1] = Read_APROM_BYTE((uint16_t __code *)0x38FC);
				timedata.data_bytes[2] = Read_APROM_BYTE((uint16_t __code *)0x38FD);
				timedata.data_bytes[3] = Read_APROM_BYTE((uint16_t __code *)0x38FE);
#elif defined __SDCC__
				timedata.data_bytes[0] = Read_APROM_BYTE((uint16_t __code*)0x38FB);
				timedata.data_bytes[1] = Read_APROM_BYTE((uint16_t __code*)0x38FC);
				timedata.data_bytes[2] = Read_APROM_BYTE((uint16_t __code*)0x38FD);
				timedata.data_bytes[3] = Read_APROM_BYTE((uint16_t __code*)0x38FE);
#endif
				seconds_counter = timedata.time + seconds_counter/60;
				if(finished == 255 || seconds_counter == 0xFFFFFFFF) {
					timedata.time = 0;
				}
				else {
					timedata.time = seconds_counter;
				}
				Write_DATAFLASH_BYTE (0x38FB,timedata.data_bytes[0]);
				Write_DATAFLASH_BYTE (0x38FC,timedata.data_bytes[1]);
				Write_DATAFLASH_BYTE (0x38FD,timedata.data_bytes[2]);
				Write_DATAFLASH_BYTE (0x38FE,timedata.data_bytes[3]);
				finished = 0;
				seconds_counter = 0;
			}
   }

}



