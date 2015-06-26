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
// @file LCD.h
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK.
// 
// @brief 
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. This library is a refactored version of the one supplied
// in the Arduino SDK in such a way that it simplifies its extension
// to support other mechanism to communicate to LCDs such as I2C, Serial, SR, 
// The original library has been reworked in such a way that this will be
// the base class implementing all generic methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets.
//
// This base class is a pure abstract class and needs to be extended. As reference,
// it has been extended to drive 4 and 8 bit mode control, LCDs and I2C extension
// backpacks such as the I2CLCDextraIO using the PCF8574* I2C IO Expander ASIC.
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
//
// @version API 1.1.0
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#ifndef _LCD_H_
#define _LCD_H_

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t byte;

// General LCD macros used by the rest of the commands
// ---------------------------------------------------------------------------
#define waitUsec(x)    __delay_us(x)

#define setBit(port, bit_pos)           \
    do {                                \
        *(port) |= (1 << (bit_pos));    \
    } while (0)

#define clearBit(port, bit_pos)         \
    do {                                \
        *(port) &= ~(1 << (bit_pos));   \
    } while (0)

#define LCD_send(this, value, mode) (this)->send((this), (value), (mode))
#define LCD_command(this, value)    LCD_send(this, value, COMMAND)
#define LCD_write(this, value)      LCD_send(this, value, DATA)

/*!
 @defined 
 @abstract   Command execution time on the LCD.
 @discussion This defines how long a command takes to execute by the LCD.
 The time is expressed in micro-seconds.
 */
#define EXEC_TIME 40

/*!
 \brief   LCD available commands. All these definitions shouldn't be used unless you are writing
 a driver.
 \details All these definitions are for driver implementation only and
 shouldn't be used by applications.
 */

/**
 * \defgroup LCD_Commands LCD available commands
 *
 * @{
 */
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

/** @} */

/*!
 \brief   Flags available for display entry mode. All these definitions shouldn't be used unless you are writing
 a driver.
 \details All these definitions are for driver implementation only and
 shouldn't be used by applications.
 */

/**
 * \defgroup LCD_DisplayEntryMode Flags available for display entry mode
 *
 * @{
 */

#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/** @} */

/*!
 \brief   Flags available for display on/off and cursor control. All these definitions shouldn't be used unless you are writing
 a driver.
 \details All these definitions are for driver implementation only and
 shouldn't be used by applications.
 */

/**
 * \defgroup LCD_DisplayCursorON_OFF Display on/off and cursor control flags
 *
 * @{
 */

#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

/** @} */

/*!
 \brief   Flags available for display/cursor shift. All these definitions shouldn't be used unless you are writing
 a driver.
 \details All these definitions are for driver implementation only and
 shouldn't be used by applications.
 */

/**
 * \defgroup LCD_DisplayCursoShift Display/Cursor shift Flags
 *
 * @{
 */

#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

/** @} */

/*!
 \brief   Flags available for Function set. All these definitions shouldn't be used unless you are writing
 a driver.
 \details All these definitions are for driver implementation only and
 shouldn't be used by applications.
 */

/**
 * \defgroup LCD_FunctionSet Function set Flags
 *
 * @{
 */

#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

/** @} */

/*!
 \brief   Special Flags used by send function (COMMAND and DATA LCD for Register Select). All these definitions shouldn't be used unless you are writing
 a driver.
 \details All these definitions are for driver implementation only and
 shouldn't be used by applications.
 */

/**
 * \defgroup LCD_CommandData Special Flags
 *
 * @{
 */

#define COMMAND                 0
#define DATA                    1

/** @} */

/*!
 \def   HOME_CLEAR_EXEC
 \brief   Defines the duration of the home and clear commands
 \details This constant defines the time it takes for the home and clear
 commands in the LCD - Time in microseconds.
 */
#define HOME_CLEAR_EXEC      2000

/*!
 \brief   This struct represents a parallel interface for the LCD
 */
struct LCDParallelInt {
    volatile uint8_t *lcd_dport;    // Data port
    volatile uint8_t *lcd_cport;    // Control port
    uint8_t rs_pin;
    uint8_t enable_pin;
};

/*!
 \brief   This struct represents a shift register interface for the LCD
 */
struct LCDShiftRegInt {
    volatile uint8_t *sr_port;
    uint8_t srdata_pin;  // Serial Data pin
    uint8_t srclock_pin; // Clock Pin
    uint8_t strobe_pin;  // Enable Pin
};

/*!
 \brief   Struct to represent an LCD object
 */
struct LCD 
{
    /**
     *  LCD_5x10DOTS or LCD_5x8DOTS, LCD_4BITMODE or
     *  LCD_8BITMODE, LCD_1LINE or LCD_2LINE
     */
    uint8_t displayfunction;

    /**
     * LCD base control command LCD on/off, blink, cursor
     * all commands are "ored" to its contents.
     */
    uint8_t displaycontrol; 

    /** Text entry mode to the LCD */
    uint8_t displaymode;

    /** Number of lines of the LCD, initialized with begin() */
    uint8_t numlines;

    /** Number of columns in the LCD */
    uint8_t cols;

    /** Data related to specific driver implementation */
    union {
        struct LCDParallelInt pi;
        struct LCDShiftRegInt sri;
    } i;

    /** Methods related to the I/O interface of the driver */
    void (*send)(struct LCD *this, uint8_t value, uint8_t mode);
    void (*begin)(struct LCD *this, uint8_t cols, uint8_t lines, uint8_t dotsize);
};

/**
 * \defgroup LCD_Functions LCD API Public Functions
 *
 * @{
 */

/*!
\brief   LCD initialization.
\details Initializes the LCD to a given size (col, row). This methods
initializes the LCD, therefore, it MUST be called prior to using any other
method from this class.

This method is abstract, a base implementation is available common to all LCD
drivers. Should it not be compatible with some other LCD driver, a derived
implementation should be done on the driver specif class.

\param      this The LCD object reference
\param      cols the number of columns that the display has
\param      rows the number of rows that the display has
\param      charsize character size, default==LCD_5x8DOTS
*/
#define LCD_begin(this, cols, rows, charsize)   (this)->begin((this), cols, rows, charsize)

/*!
\brief   Clears the LCD.
\details Clears the LCD screen and positions the cursor in the upper-left
corner.

This operation is time consuming for the LCD.

\param this The LCD object reference
*/
void LCD_clear(struct LCD *this);

/*!
\brief   Sets the cursor to the upper-left corner.
\details Positions the cursor in the upper-left of the LCD.
That is, use that location in outputting subsequent text to the display.
To also clear the display, use the clear() function instead.

This operation is time consuming for the LCD.

\param this The LCD object reference
*/
void LCD_home(struct LCD *this);

/*!
\brief   Turns off the LCD display.
\details Turns off the LCD display, without losing the text currently
being displayed on it.

\param this The LCD object reference
*/
void LCD_noDisplay(struct LCD *this);

/*!
\brief   Turns on the LCD display.
\details Turns on the LCD display, after it's been turned off with
noDisplay(). This will restore the text (and cursor location) that was on
the display prior to calling noDisplay().

\param this The LCD object reference
*/
void LCD_display(struct LCD *this);

/*!
\brief   Turns off the blinking of the LCD cursor.

\param this The LCD object reference
*/
void LCD_noBlink(struct LCD *this);

/*!
\brief   Display the cursor of the LCD.
\details Display the blinking LCD cursor. If used in combination with
the cursor() function, the result will depend on the particular display.

\param this The LCD object reference
*/
void LCD_blink(struct LCD *this);

/*!
\brief   Hides the LCD cursor.

\param this The LCD object reference
*/
void LCD_noCursor(struct LCD *this);

/*!
\brief   Display the LCD cursor.
\details Display the LCD cursor: an underscore (line) at the location
where the next character will be written.

\param this The LCD object reference
*/
void LCD_cursor(struct LCD *this);

/*!
\brief   Scrolls the contents of the display (text and cursor) one space
to the left.

\param this The LCD object reference
*/
void LCD_scrollDisplayLeft(struct LCD *this);

/*!
\brief   Scrolls the contents of the display (text and cursor) one space
to the right.

\param this The LCD object reference
*/
void LCD_scrollDisplayRight(struct LCD *this);

/*!
\brief   Set the direction for text written to the LCD to left-to-right.
\details Set the direction for text written to the LCD to left-to-right.
All subsequent characters written to the display will go from left to right,
but does not affect previously-output text.

This is the default configuration.

\param this The LCD object reference
*/
void LCD_leftToRight(struct LCD *this);

/*!
\brief   Set the direction for text written to the LCD to right-to-left.
\details Set the direction for text written to the LCD to right-to-left.
All subsequent characters written to the display will go from right to left,
but does not affect previously-output text.

left-to-right is the default configuration.

\param this The LCD object reference
*/
void LCD_rightToLeft(struct LCD *this);

/*!
\brief   Moves the cursor one space to the left.
\details
\param this The LCD object reference
*/
void LCD_moveCursorLeft(struct LCD *this);


/*!
\brief   Moves the cursor one space to the right.

\param this The LCD object reference
*/
void LCD_moveCursorRight(struct LCD *this);

/*!
\brief   Turns on automatic scrolling of the LCD.
\details Turns on automatic scrolling of the LCD. This causes each
character output to the display to push previous characters over by one
space. If the current text direction is left-to-right (the default),
the display scrolls to the left; if the current direction is right-to-left,
the display scrolls to the right.
This has the effect of outputting each new character to the same location on
the LCD.

\param this The LCD object reference
*/
void LCD_autoscroll(struct LCD *this);

/*!
\brief   Turns off automatic scrolling of the LCD.
\details Turns off automatic scrolling of the LCD, this is the default
configuration of the LCD.

\param this The LCD object reference
*/
void LCD_noAutoscroll(struct LCD *this);

/*!
\brief   Creates a custom character for use on the LCD.
\details Create a custom character (glyph) for use on the LCD.
Most chipsets only support up to eight characters of 5x8 pixels. Therefore,
this methods has been limited to locations (numbered 0 to 7).

The appearance of each custom character is specified by an array of eight
bytes, one for each row. The five least significant bits of each byte
determine the pixels in that row. To display a custom character on screen,
write()/print() its number, i.e. lcd.print (char(x)); // Where x is 0..7.

\param      this The LCD object reference
\param      location LCD memory location of the character to create (0 to 7)
\param      charmap the bitmap array representing each row of the character.
*/
void LCD_createChar(struct LCD *this, uint8_t location, uint8_t charmap[]);

/*!
\brief   Position the LCD cursor.
\details Sets the position of the LCD cursor. Set the location at which
subsequent text written to the LCD will be displayed.

\param      this The LCD object reference
\param      col LCD column
\param      row LCD row - line.
*/
void LCD_setCursor(struct LCD *this, uint8_t col, uint8_t row);

/*!
\brief   Switch on the LCD module.
\details Switch on the LCD module, it will switch on the LCD controller
and the backlight. This method has the same effect of calling display and
backlight. @see LCD_display, @see LCD_backlight

\param this The LCD object reference
*/
void LCD_on(struct LCD *this);

/*!
\brief   Switch off the LCD module.
\details Switch off the LCD module, it will switch off the LCD controller
and the backlight. This method has the same effect of calling noDisplay and
noBacklight. @see LCD_display, @see LCD_backlight

\param this The LCD object reference
*/
void LCD_off(struct LCD *this);

/*!
\brief   Writes a string to the LCD.
\details This method writes character string to the LCD in the current cursor
position.

This is the virtual write method.

\param      this The LCD object reference
\param      value Value to write to the LCD.
*/
void LCD_printString(struct LCD *this, const char *value);

/*!
\brief   Writes an unsigned integer to the LCD.
\details This method writes an unsigned integer value to the LCD in the current cursor
position.

\param      this The LCD object reference
\param      value Value to write to the LCD.
*/
void LCD_printUInt(struct LCD *this, uint16_t value);

/*!
\brief   Writes a signed integer to the LCD.
\details This method writes a signed integer value to the LCD in the current cursor
position.

\param      this The LCD object reference
\param      value Value to write to the LCD.
*/
void LCD_printSInt(struct LCD *this, int16_t value);

/*!
\brief   Writes a character to the LCD.
\details This function writes a character to the LCD in the current cursor
position.

\hideinitializer
\param      this The LCD object reference
\param      value Value to write to the LCD.
*/
#define LCD_printChar(this, value) (this)->send(this, value, DATA)

/*!
\brief   Initialize the LCD in parallel mode.
\details Initialize the LCD to use the parallel interface.

\param      this        The LCD object reference
\param      bitmode     4/8 access mode.
\param      lcd_dport   Data port
\param      lcd_cport   Control port
\param      rs_pin      Register Select pin to use in control port
\param      enable_pin  Enable (Clock) pin to use in control port
*/
void LCD_initParallel(struct LCD *this, uint8_t bitmode, volatile uint8_t *lcd_dport, volatile uint8_t *lcd_cport, uint8_t rs_pin, uint8_t enable_pin);

/*!
\brief   Initialize the LCD in parallel mode.
\details Initialize the LCD to use the parallel interface.

\param      this        The LCD object reference
\param      sr_port     Port where the shift register is connected
\param      srdata      Shift register data pin
\param      srclock     Shift register clock pin
\param      strobe      Shift register strobe pin
*/
void LCD_initShiftReg(struct LCD *this, volatile uint8_t *sr_port, uint8_t srdata, uint8_t srclock, uint8_t strobe);
#endif

/** @} */