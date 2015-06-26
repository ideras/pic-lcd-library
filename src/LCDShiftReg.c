// Original library created by Francisco Malpartida on 20/08/11.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// Ported to PIC microcontrollers  by Ivan de Jesus Deras (ideras@gmail.com)
//
// You can find more information of the original library at:
// https://github.com/marcmerlin/NewLiquidCrystal

#include <stdio.h>
#include "LCD.h"

// bitmasks for control bits on shift register
#define SR_EN_BIT 0b00010000   // LCD Data enable bit.
#define SR_RW_BIT 0b00100000   // RW can be pinned low since we only send
#define SR_RS_BIT 0b01000000   // LOW: command. HIGH: character.

static void shiftOut(volatile uint8_t *port, uint8_t dataPin, uint8_t clockPin, uint8_t val)
{
        uint8_t i;

        for (i = 0; i < 8; i++)  {
            if ((val & (1 << i)) != 0)
                setBit(port, dataPin);
            else
                clearBit(port, dataPin);
                        
            setBit(port, clockPin);
            clearBit(port, clockPin);
        }
}

static void _pushOut(struct LCD *this, uint8_t nibble)
{
    // Make data available for pushing to the LCD.
    shiftOut(this->i.sri.sr_port, this->i.sri.srdata_pin, this->i.sri.srclock_pin, nibble);

    // Make new data active.
    setBit(this->i.sri.sr_port, this->i.sri.strobe_pin);
    waitUsec(1); // strobe pulse must be >450ns (old code had 10ms)
    clearBit(this->i.sri.sr_port, this->i.sri.strobe_pin);
    waitUsec(40); // commands need > 37us to settle
}

static void write4bits(struct LCD *this, uint8_t nibble)
{
    nibble &= ~SR_RW_BIT; // set RW LOW (we do this always since we only write).

    // Send a High transition to display the data that was pushed
    nibble |=  SR_EN_BIT; // LCD Data Enable HIGH
    _pushOut(this, nibble);
    nibble &= ~SR_EN_BIT; // LCD Data Enable LOW
    _pushOut(this, nibble); 
}

// Parallel Send Data/Command to the LCD
static void LCD_shiftRegSend(struct LCD *this, uint8_t value, uint8_t mode)
{
   uint8_t nibble;
   
   mode = mode ? SR_RS_BIT : 0; // RS bit; LOW: command.  HIGH: character.

   nibble = value >> 4; // Get high nibble.
   write4bits(this, nibble | mode);

   //delay(1); // This was in the LCD3 code but does not seem needed -- merlin

   nibble = value & 0x0f; // Get low nibble
   write4bits(this, nibble | mode);
}

void LCD_beginShiftReg(struct LCD *this, uint8_t cols, uint8_t lines, uint8_t dotsize) 
{
    uint8_t i;
    
   if (lines > 1) 
   {
      this->displayfunction |= LCD_2LINE;
   }

   this->numlines = lines;
   this->cols = cols;
   
   // for some 1 line displays you can select a 10 pixel high font
   // ------------------------------------------------------------
   if ((dotsize != 0) && (lines == 1)) 
   {
      this->displayfunction |= LCD_5x10DOTS;
   }
   
   // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
   // according to datasheet, we need at least 40ms after power rises above 2.7V
   // before sending commands. Arduino can turn on way before 4.5V so we'll wait 
   // 50.
   // Note from Ivan Deras: I don't know if this is necessary in PIC, but it shouldn't matter anyway.
   // ---------------------------------------------------------------------------
   
   for (i=0; i<5; i++) {
       __delay_ms(10);
   }

   // This init is copied verbatim from the spec sheet.
   // 8 bit codes are shifted to 4 bit
   write4bits(this, (LCD_FUNCTIONSET | LCD_8BITMODE) >> 4);
   __delay_us(4500);  // wait more than 4.1ms
   
   // Second try
   write4bits(this, (LCD_FUNCTIONSET | LCD_8BITMODE) >> 4);
   __delay_us(150);
   // Third go
   write4bits(this, (LCD_FUNCTIONSET | LCD_8BITMODE) >> 4);
   
   // And finally, set to 4-bit interface
   write4bits(this, (LCD_FUNCTIONSET | LCD_4BITMODE) >> 4);
   
   // Set # lines, font size, etc.
   LCD_command(this, LCD_FUNCTIONSET | this->displayfunction);
   // Turn the display on with no cursor or blinking default
   this->displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
   LCD_display(this);
   // Clear it off
   LCD_clear(this);
   // Initialize to default text direction (for romance languages)
    this->displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
   // set the entry mode
   LCD_command(this, LCD_ENTRYMODESET | this->displaymode);
   LCD_home(this);
}

void LCD_initShiftReg(struct LCD *this, volatile uint8_t *sr_port, uint8_t srdata, uint8_t srclock, uint8_t strobe)
{
    this->displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x10DOTS;
    
    // Initialize private variables
    this->i.sri.sr_port = sr_port;
    this->i.sri.srdata_pin  = srdata; 
    this->i.sri.srclock_pin = srclock; 
    this->i.sri.strobe_pin = strobe;

   // Initialize _strobe_pin at low.
    clearBit(sr_port, strobe);
    
    // Little trick to force a pulse of the LCD enable bit and make sure it is
    // low before we start further writes since this is assumed.
    write4bits(this, 0);
    
    this->send = &LCD_shiftRegSend;
    this->begin = &LCD_beginShiftReg;
}
