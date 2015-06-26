// ---------------------------------------------------------------------------
// Created by Francisco Malpartida on 20/08/11.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file LiquidCrystal.h
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK.
// 
// @brief 
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. The original library has been reworked in such a way that 
// this class implements the all methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets using the parallel port of
// the LCD (4 bit and 8 bit).
//
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#ifndef LCD_4bit_h
#define LCD_4bit_h

#include <stdint.h>

/*!
 @defined 
 @abstract   Command execution time on the LCD.
 @discussion This defines how long a command takes to execute by the LCD.
 The time is expressed in micro-seconds.
 */
#define EXEC_TIME 40

struct LCD;
void LCD_init(struct LCD *this, uint8_t bitmode, volatile uint8_t *lcd_dport, volatile uint8_t *lcd_cport, uint8_t rs_pin, uint8_t enable_pin);

#endif
