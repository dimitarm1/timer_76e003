/*
 * led.h
 *
 * Copyright 2014 Edward V. Emelianoff <eddy@sao.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __LED_H__
#define __LED_H__
#define U8 unsigned char
#define S32 long int

// #define CLOCK          P06 // DEBUG
#define CLOCK          P02 
#define DATA           P10

#define INVERT_SMALL_DIGITS // For Yellow 7 Seg

void set_display_buf(char *str);
void show_buf_digit(U8 N);
void show_next_digit(void);
void display_int_sec(S32 i);
void display_int(S32 i);
void display_DP_at_pos(U8 i);
void all_digits_off(void);


#endif // __LED_H__
