PIC Liquid Crystal Display Library
===================================

Welcome to the LCD Library for PIC. It is a derivate of the original LiquidCrystal Library as sourced in the Arduino SDK. 
It has been ported to PIC by Ivan de Jesus Deras (ideras@gmail.com).  As the original library this one is extendable if need be.

It supports most Hitachi HD44780 based LCDs, or compatible, connected to any project using: 4, 8 wire parallel interface, and Shift Regiter.

This port doesn't support the I2C IO expander, but it can be easily extended to support it.

You can find the original library at 

https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home

Tested only on PIC16F887, PIC18F45K22.  You'll need XC8 compiler from Microchip to compile the code, but the code should be easily compiled qith another C compiler.

Usage
=====

Here's a example of using the library in 4-bit parallel mode:
```C
struct LCD theLCD;

void main()
{
  PORTD = 0;
  ANSELD = 0;
  TRISD = 0; // LCD is connected to PORTD
  
  /* 
   * Init the LCD in parallel 4 bit mode
   * LATD is data port bits [3:0]
   * LATD is control port were BIT 4 is RS (Register Select) and BIT 5 is ENABLE
   */
  LCD_initParallel(&theLCD, LCD_4BITMODE, &LATD, &LATD, 4, 5);
  LCD_begin(&theLCD, 16, 2, LCD_5x8DOTS);
  LCD_home(&theLCD);
  LCD_printString(&theLCD, "Hello PIC");
  LCD_setCursor(&theLCD, 0, 1);
  LCD_printString("WORLD!");
}
```
The same example can used with 8-bit parallel mode changing the line:

```C
LCD_initParallel(&theLCD, LCD_4BITMODE, &LATD, &LATD, 4, 5);
```

to

```C
LCD_initParallel(&theLCD, LCD_8BITMODE, &LATD, &LATB, 0, 1);
```

In this case you need two ports, one for data and another for control.  In this example PORT D is used for data and PORT B for control. You have to declare pins 0 and 1 of PORTB as OUTPUT by setting TRISB to 0x03, also if this pins are used for Analog functions, set them as Digitals by setting the corresponding ANSEL register, on PIC18F45K22 is ANSELB &= 0xFC.

For shift register mode you have to use the following line:

```C
LCD_initShiftReg(&theLCD, &LATD, 0, 1, 2);
```
Bits 0, 1, 2 of PORT D are DATA, CLOCK and STROBE pins of the shift register.  Also in this case don't forget to set this pins as digital outputs setting the corresponding registers.

For the physical connections you can take a look at http://playground.arduino.cc/Code/LCD3wires
