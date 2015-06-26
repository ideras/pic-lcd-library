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
// @file LCD.cpp
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK.
// 
// @brief 
// This is a basic implementation of the HD44780 library of the
// Arduino SDK. This library is a refactored version of the one supplied
// in the Arduino SDK in such a way that it simplifies its extension
// to support other mechanism to communicate to LCDs such as I2C, Serial, SR, ...
// The original library has been reworked in such a way that this will be
// the base class implementing all generic methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets.
//
// This base class is a pure abstract class and needs to be extended. As reference,
// it has been extended to drive 4 and 8 bit mode control, LCDs and I2C extension
// backpacks such as the I2CLCDextraIO using the PCF8574* I2C IO Expander ASIC.
//
//
// @version API 1.1.0
//
// 2012.03.29 bperrybap - changed comparision to use LCD_5x8DOTS rather than 0
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#include <stdio.h>
#include "LCD.h"

uint8_t digit[6];

// PUBLIC METHODS
// ---------------------------------------------------------------------------
// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the microcontroller doesn't reset the LCD, so we
// can't assume that its in that state when the program starts (and the
// LiquidCrystal init method is called).
// A call to begin() will reinitialize the LCD.
//

// Common LCD Commands
// ---------------------------------------------------------------------------
void LCD_clear(struct LCD *this)
{
   LCD_command(this, LCD_CLEARDISPLAY);     // clear display, set cursor position to zero
   __delay_us(HOME_CLEAR_EXEC);             // this command is time consuming
}

void LCD_home(struct LCD *this)
{
   LCD_command(this, LCD_RETURNHOME);   // set cursor position to zero
   __delay_us(HOME_CLEAR_EXEC);         // This command is time consuming
}

void LCD_setCursor(struct LCD *this, uint8_t col, uint8_t row)
{
   const byte row_offsetsDef[]   = { 0x00, 0x40, 0x14, 0x54 }; // For regular LCDs
   const byte row_offsetsLarge[] = { 0x00, 0x40, 0x10, 0x50 }; // For 16x4 LCDs
   
   if ( row >= this->numlines )
   {
      row = this->numlines-1;    // rows start at 0
   }
   
   // 16x4 LCDs have special memory map layout
   // ----------------------------------------
   if ( this->cols == 16 && this->numlines == 4 )
   {
      LCD_command(this, LCD_SETDDRAMADDR | (col + row_offsetsLarge[row]));
   }
   else 
   {
      LCD_command(this, LCD_SETDDRAMADDR | (col + row_offsetsDef[row]));
   }
   
}

// Turn the display on/off
void LCD_noDisplay(struct LCD *this)
{
   this->displaycontrol &= ~LCD_DISPLAYON;
   LCD_command(this, LCD_DISPLAYCONTROL | this->displaycontrol);
}

void LCD_display(struct LCD *this)
{
   this->displaycontrol |= LCD_DISPLAYON;
   LCD_command(this, LCD_DISPLAYCONTROL | this->displaycontrol);
}

// Turns the underline cursor on/off
void LCD_noCursor(struct LCD *this)
{
   this->displaycontrol &= ~LCD_CURSORON;
   LCD_command(this, LCD_DISPLAYCONTROL | this->displaycontrol);
}
void LCD_cursor(struct LCD *this)
{
   this->displaycontrol |= LCD_CURSORON;
   LCD_command(this, LCD_DISPLAYCONTROL | this->displaycontrol);
}

// Turns on/off the blinking cursor
void LCD_noBlink(struct LCD *this)
{
   this->displaycontrol &= ~LCD_BLINKON;
   LCD_command(this, LCD_DISPLAYCONTROL | this->displaycontrol);
}

void LCD_blink(struct LCD *this)
{
   this->displaycontrol |= LCD_BLINKON;
   LCD_command(this, LCD_DISPLAYCONTROL | this->displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCD_scrollDisplayLeft(struct LCD *this)
{
   LCD_command(this, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LCD_scrollDisplayRight(struct LCD *this)
{
   LCD_command(this, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCD_leftToRight(struct LCD *this)
{
   this->displaymode |= LCD_ENTRYLEFT;
   LCD_command(this, LCD_ENTRYMODESET | this->displaymode);
}

// This is for text that flows Right to Left
void LCD_rightToLeft(struct LCD *this)
{
   this->displaymode &= ~LCD_ENTRYLEFT;
   LCD_command(this, LCD_ENTRYMODESET | this->displaymode);
}

// This method moves the cursor one space to the right
void LCD_moveCursorRight(struct LCD *this)
{
   LCD_command(this, LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVERIGHT);
}

// This method moves the cursor one space to the left
void LCD_moveCursorLeft(struct LCD *this)
{
   LCD_command(this, LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVELEFT);
}


// This will 'right justify' text from the cursor
void LCD_autoscroll(struct LCD *this)
{
   this->displaymode |= LCD_ENTRYSHIFTINCREMENT;
   LCD_command(this, LCD_ENTRYMODESET | this->displaymode);
}

// This will 'left justify' text from the cursor
void LCD_noAutoscroll(struct LCD *this)
{
   this->displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
   LCD_command(this, LCD_ENTRYMODESET | this->displaymode);
}

// Write to CGRAM of new characters
void LCD_createChar(struct LCD *this, uint8_t location, uint8_t charmap[])
{
    int8_t i;

   location &= 0x7;            // we only have 8 locations 0-7
   
   LCD_command(this, LCD_SETCGRAMADDR | (location << 3));
   __delay_us(40);
   
   for (i=0; i<8; i++)
   {
      LCD_write(this, charmap[i]);      // call the virtual write method
      __delay_us(40);
   }
}

//
// Switch fully on the LCD (backlight and LCD)
void LCD_on(struct LCD *this)
{
   LCD_display(this);
}

//
// Switch fully off the LCD (backlight and LCD) 
void LCD_off(struct LCD *this)
{
   LCD_noDisplay(this);
}

// Write a null terminated string to the LCD
void LCD_printString(struct LCD *this, const char *value)
{
    const char *p = value;

    while (*p != '\0') {
        LCD_write(this, (uint8_t)(*p));
        p++;
    }
}

static inline int8_t parseInt(uint16_t value)
{
    int8_t pos = 0;

    do {
        digit[pos] = 48 + (value % 10);
        value /= 10;
        pos++;
    } while ( value != 0 );

    return pos-1;
}

static inline void writeDigits(struct LCD *this, int lastIndex)
{
    int8_t i = lastIndex;

    while (i >= 0) {
        LCD_write(this, digit[i]);
        i--;
    }
}

// Write an unsigned integer value to the LCD
void LCD_printUInt(struct LCD *this, uint16_t value)
{
    int8_t lastIndex;
    
    lastIndex = parseInt(value);

    writeDigits(this, lastIndex);
}

// Write a signed integer value to the LCD
void LCD_printSInt(struct LCD *this, int16_t value)
{
    uint8_t sign, lastIndex;

    sign = (value & (1 << 15)) != 0;

    // Value in 2's complement representation
    if (sign == 1)
        value = ~value + 1;

    lastIndex = parseInt(value);

    if (sign) {
        ++lastIndex;
        digit[lastIndex] = '-';
    }

    writeDigits(this, lastIndex);
}
