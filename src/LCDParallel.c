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

// pulseEnable
#define pulseEnable(this)   \
    do {                    \
        setBit(this->i.pi.lcd_cport, this->i.pi.enable_pin);     \
        __delay_us(5);                                           \
        clearBit(this->i.pi.lcd_cport, this->i.pi.enable_pin);   \
    } while (0)

//write8bits
#define write8bits(this, value)                         \
    do {                                                \
        *(this->i.pi.lcd_dport) = value;                \
        pulseEnable(this);                              \
    } while (0)

// write4bits
#ifndef LCD_USE_UPPER_NIBBLE
#define write4bits(this, value)                         \
    do {                                                \
        *(this->i.pi.lcd_dport) &= 0xF0;                \
        *(this->i.pi.lcd_dport) |= (value) & 0x0F;      \
        pulseEnable(this);                              \
    } while (0)
#else
#define write4bits(this, value)                         \
    do {                                                \
        *(this->lcd_dport) &= 0x0F;                     \
        *(this->lcd_dport) |= (value & 0x0F) << 4;      \
        pulseEnable(this);                              \
    } while (0)
#endif

/************ low level data pushing commands **********/
// Parallel Send Data/Command to the LCD
void LCD_sendParallel(struct LCD *this, uint8_t value, uint8_t mode)
{
    // Only interested in COMMAND or DATA
    if (mode == DATA)
        setBit(this->i.pi.lcd_cport, this->i.pi.rs_pin);
    else
        clearBit(this->i.pi.lcd_cport, this->i.pi.rs_pin);
    
   if (this->displayfunction & LCD_8BITMODE)
   {
      write8bits(this, value); 
   } 
   else 
   {
      write4bits (this, value >> 4);
      waitUsec(5);
      write4bits (this, value);
   }
    waitUsec(EXEC_TIME); // wait for the command to execute by the LCD
}

void LCD_beginParallel(struct LCD *this, uint8_t cols, uint8_t lines, uint8_t dotsize)
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
   
   // Now we pull both RS and ENABLE low to begin commands
   clearBit(this->i.pi.lcd_cport, this->i.pi.rs_pin);
   clearBit(this->i.pi.lcd_cport, this->i.pi.enable_pin);
   
   //put the LCD into 4 bit or 8 bit mode
   // -------------------------------------
   if (! (this->displayfunction & LCD_8BITMODE))    //4bit mode
   {
      // this is according to the hitachi HD44780 datasheet
      // figure 24, pg 46
      
      // we start in 8bit mode, try to set 4 bit mode
      write4bits(this, 0x03);
      __delay_us(4500); // wait min 4.1ms
      
      // second try
      write4bits(this, 0x03);
      __delay_us(4500); // wait min 4.1ms
      
      // third go!
      write4bits(this, 0x03); 
      __delay_us(150);
      
      // finally, set to 4-bit interface
      write4bits(this, 0x02); 
   } 
   else 
   {
      // this is according to the hitachi HD44780 datasheet
      // page 45 figure 23
      
      // Send function set command sequence
      LCD_command(this, LCD_FUNCTIONSET | this->displayfunction);
      __delay_us(4500);  // wait more than 4.1ms
      
      // second try
      LCD_command(this, LCD_FUNCTIONSET | this->displayfunction);
      __delay_us(150);
      
      // third go
      LCD_command(this, LCD_FUNCTIONSET | this->displayfunction);
   }
   
   // finally, set # lines, font size, etc.
   LCD_command(this, LCD_FUNCTIONSET | this->displayfunction);  
   
   // turn the display on with no cursor or blinking default
   this->displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
   LCD_display(this);
   
   // clear the LCD
   LCD_clear(this);
   
   // Initialize to default text direction (for romance languages)
   this->displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
   // set the entry mode
   LCD_command(this, LCD_ENTRYMODESET | this->displaymode);
}

void LCD_initParallel(struct LCD *this, uint8_t bitmode, volatile uint8_t *lcd_dport, volatile uint8_t *lcd_cport, uint8_t rs_pin, uint8_t enable_pin)
{
    this->displayfunction = bitmode | LCD_1LINE | LCD_5x8DOTS;
    
    this->i.pi.rs_pin = rs_pin;
    this->i.pi.enable_pin = enable_pin;
    this->i.pi.lcd_dport = lcd_dport;
    this->i.pi.lcd_cport = lcd_cport;

    // Now we pull both RS and ENABLE low to begin commands
    clearBit(lcd_cport, rs_pin);
    clearBit(lcd_cport, enable_pin);

    this->send = &LCD_sendParallel;
    this->begin = &LCD_beginParallel;
}