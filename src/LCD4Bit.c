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
// @file LiquidCrystal.cpp
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
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#include <stdio.h>
#include "LCD.h"
#include "LCD4Bit.h"

// CONSTANT  definitions
// ---------------------------------------------------------------------------
#define LCD_NOBACKLIGHT 0xFF

// LCD driver configuration (4bit or 8bit driver control)
#define LCD_4BIT                1
#define LCD_8BIT                0

uint8_t outport_latch;

#define setBit(port, bit_pos)           \
    do {                                \
        uint8_t mask = ~(1 << bit_pos); \
        outport_latch &= mask;          \
        outport_latch |= (1 << bit_pos);\
        *port = outport_latch;          \
    } while (0);

#define clearBit(port, bit_pos)         \
    do {                                \
        uint8_t mask = ~(1 << bit_pos); \
        *port &= mask;                  \
    } while (0);

// pulseEnable
#define pulseEnable(this)                                                       \
    do {                                                                        \
        setBit(this->iface_4_8bit.lcd_port, this->iface_4_8bit.enable_pin);     \
        __delay_us(EXEC_TIME);                                                  \
        clearBit(this->iface_4_8bit.lcd_port, this->iface_4_8bit.enable_pin);   \
        __delay_us(EXEC_TIME);                                                  \
    } while (0)

// write4bits
#define write4bits(this, value)                         \
    do {                                                \
        outport_latch &= 0xF0;                          \
        outport_latch |= value & 0x0F;                  \
        *(this->iface_4_8bit.lcd_port) = outport_latch; \
        pulseEnable(this);                              \
    } while (0)

/************ low level data pushing commands **********/
// send
void LCD4Bit_send(struct LCD *this, uint8_t value, uint8_t mode)
{
    // Only interested in COMMAND or DATA
    outport_latch = 0;
    if (mode == DATA)
        outport_latch |= 1 << this->iface_4_8bit.rs_pin;
    
    if ( mode != FOUR_BITS )
    {
       write4bits (this, value >> 4);
       write4bits (this, value);
    }
    else
    {
      write4bits (this, value);
    }
    __delay_us ( EXEC_TIME ); // wait for the command to execute by the LCD
}

//
// setBacklightPin
void LCD4Bit_setBacklightPin (struct LCD *this, uint8_t pin, TBacklightPol pol)
{
}

//
// setBacklight
void LCD4Bit_setBacklight (struct LCD *this, uint8_t value)
{
}

// init
void LCD4Bit_init(struct LCD *this, uint8_t rs_pin, uint8_t enable_pin, volatile uint8_t *lcd_port)
{   
   // Initialise displaymode functions to defaults: LCD_1LINE and LCD_5x8DOTS
   // -------------------------------------------------------------------------
   this->displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
   this->iface_4_8bit.rs_pin = rs_pin;
   this->iface_4_8bit.enable_pin = enable_pin;
   this->iface_4_8bit.lcd_port = lcd_port;
   
   // Now we pull both RS and ENABLE low to begin commands
   clearBit(lcd_port, rs_pin);
   clearBit(lcd_port, enable_pin);

   this->send = &LCD4Bit_send;
}
