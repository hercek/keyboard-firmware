/*
  Kinesis ergonomic keyboard firmware replacement

  Copyright 2012 Chris Andreae (chris (at) andreae.gen.nz)

  Licensed under the GNU GPL v2 (see GPL2.txt).

  ==========================

  If built for V-USB, this program includes library and sample code from:
	 V-USB, (C) Objective Development Software GmbH
	 Licensed under the GNU GPL v2 (see GPL2.txt)

  ==========================

  If built for LUFA, this program includes library and sample code from:
			 LUFA Library
	 Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
		   www.lufa-lib.org

  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef __KATY_H
#define __KATY_H

#include "keystate.h"

// Continue using the same USB VID/PID pair that's assigned to the hardware
#define USB_VENDOR_ID  0x1d50 // Openmoko, Inc
#define USB_PRODUCT_ID 0x6028 // ErgoDox ergonomic keyboard

// But define different descriptive strings
#define USB_MANUFACTURER_STRING L"andreae.gen.nz"
#define USB_SERIAL_NUMBER_STRING L"andreae.gen.nz:katy"
#if (ARCH == ARCH_AVR8)
#define USB_PRODUCT_STRING L"K80CS USB Keyboard (ATMega)"
#elif (ARCH == ARCH_XMEGA)
#define USB_PRODUCT_STRING L"K80CS USB Keyboard (ATXMega)"
#else
# error "Unknown architecture."
#endif

// Unique identifier representing this keyboard's layout and
// definition of logical_keycode values.  Is reported to the
// configuration program over USB to identify the layout.
#define LAYOUT_ID 3

/* Storage layout */
#define CONSTANT_STORAGE           avr_pgm
#define MAPPING_STORAGE            avr_eeprom

#define SAVED_MAPPING_STORAGE      spi_eeprom
#define SAVED_MAPPING_COUNT        255          // 2-byte entries (maximum allowed value is 255)
#define MACRO_INDEX_STORAGE        avr_eeprom
#define MACRO_INDEX_COUNT          32           // 6-byte entries
#define MACROS_STORAGE             spi_eeprom
#define MACROS_SIZE                1024
#define PROGRAM_STORAGE            avr_eeprom
#define PROGRAM_SIZE               1024
#define PROGRAM_COUNT              6
#define NUM_KEY_MAPPING_INDICES    10 // this can be at most 10

// if the keypad mode is selected, and a logical key greater than KEYPAD_LAYER_START
// is read, add KEYPAD_LAYER_SIZE to look up the mapping.
#define KEYPAD_LAYER_START 0
#define KEYPAD_LAYER_SIZE  80

//80 physical keys, 78 of which (all but keypad/program) have a separate keypad layer mapping
#define NUM_LOGICAL_KEYS (KEYPAD_LAYER_START + (KEYPAD_LAYER_SIZE * 2))

// Katy selects columns and reads rows. We're going to reverse the naming convention,
// since our convention is that we select "rows" and read "columns".  We select
// the same row on each side (pulling it low) and then scan the columns (input
// with internal pull-ups).


#define MATRIX_COLS 5  // 5 rows on each side, right side direct, left side via shift register)
#define MATRIX_ROWS 16 // 8 cols on each side

// Logical keys we have: logical keys represent a key-position+keypad-layer combination.
#define LOGICAL_KEY_KEYPAD LOGICAL_KEY_RROW1
#define LOGICAL_KEY_PROGRAM LOGICAL_KEY_LROW1
#define LOGICAL_KEY_LEFT_LAYER_SHIFT LOGICAL_KEY_TH_LL
#define LOGICAL_KEY_RIGHT_LAYER_SHIFT LOGICAL_KEY_TH_RR
enum logical_keys {
	LOGICAL_KEY_LROW1, // left bottom row (outer)          //program
	LOGICAL_KEY_RROW1, // right bottom row (outer)         //keypad
	LOGICAL_KEY_TH_LL, // extra left side left thumb key   // keypadShift
	LOGICAL_KEY_TH_RR, // extra right side right thumb key // keypadShift
	// Main key blocks
	LOGICAL_KEY_A,
	LOGICAL_KEY_B,
	LOGICAL_KEY_C,
	LOGICAL_KEY_D,
	LOGICAL_KEY_E,
	LOGICAL_KEY_F,
	LOGICAL_KEY_G,
	LOGICAL_KEY_H,
	LOGICAL_KEY_I,
	LOGICAL_KEY_J,
	LOGICAL_KEY_K,
	LOGICAL_KEY_L,
	LOGICAL_KEY_M,
	LOGICAL_KEY_N,
	LOGICAL_KEY_O,
	LOGICAL_KEY_P,
	LOGICAL_KEY_Q,
	LOGICAL_KEY_R,
	LOGICAL_KEY_S,
	LOGICAL_KEY_T,
	LOGICAL_KEY_U,
	LOGICAL_KEY_V,
	LOGICAL_KEY_W,
	LOGICAL_KEY_X,
	LOGICAL_KEY_Y,
	LOGICAL_KEY_Z,
	LOGICAL_KEY_1,
	LOGICAL_KEY_2,
	LOGICAL_KEY_3,
	LOGICAL_KEY_4,
	LOGICAL_KEY_5,
	LOGICAL_KEY_6,
	LOGICAL_KEY_7,
	LOGICAL_KEY_8,
	LOGICAL_KEY_9,
	LOGICAL_KEY_0,
	LOGICAL_KEY_SEMICOLON,
	LOGICAL_KEY_COMMA,
	LOGICAL_KEY_PERIOD,
	LOGICAL_KEY_SLASH,

	// Left hand extra keys
	LOGICAL_KEY_LCOL1_1, // outer column (top)
	LOGICAL_KEY_LCOL1_2,
	LOGICAL_KEY_LCOL1_3,
	LOGICAL_KEY_LCOL1_4,
	LOGICAL_KEY_LROW2, // bottom row (outer)
	LOGICAL_KEY_LROW3,
	LOGICAL_KEY_LROW4,
	LOGICAL_KEY_LROW5,
	LOGICAL_KEY_LCOL2_1, // inner column (top)
	LOGICAL_KEY_LCOL2_2,
	LOGICAL_KEY_LCOL2_3,

	// Right hand extra keys
	LOGICAL_KEY_RCOL1_1, // outer column (top)
	LOGICAL_KEY_RCOL1_2,
	LOGICAL_KEY_RCOL1_3,
	LOGICAL_KEY_RCOL1_4,
	LOGICAL_KEY_RROW2, // bottom row (outer)
	LOGICAL_KEY_RROW3,
	LOGICAL_KEY_RROW4,
	LOGICAL_KEY_RROW5,
	LOGICAL_KEY_RCOL2_1, // inner column (top)
	LOGICAL_KEY_RCOL2_2,
	LOGICAL_KEY_RCOL2_3,

	// Left hand thumb pad
	LOGICAL_KEY_L_ALT,
	LOGICAL_KEY_L_CTRL,
	LOGICAL_KEY_HOME,
	LOGICAL_KEY_END,
	LOGICAL_KEY_DELETE,
	LOGICAL_KEY_BACKSPACE,
	LOGICAL_KEY_TH_LR, // extra left side right thumb key

	// Right hand thumb pad
	LOGICAL_KEY_R_ALT,
	LOGICAL_KEY_R_CTRL,
	LOGICAL_KEY_PGUP,
	LOGICAL_KEY_PGDN,
	LOGICAL_KEY_ENTER,
	LOGICAL_KEY_SPACE,
	LOGICAL_KEY_TH_RL, // extra right side left thumb key

	// The keypad layer duplicates all the previous keys
};

// Which logical keys to use for special in-built combinations
#define SPECIAL_HKEY_CONFIG_LOAD   HID_KEYBOARD_SC_E
#define SPECIAL_HKEY_CONFIG_SAVE   HID_KEYBOARD_SC_S
#define SPECIAL_HKEY_CONFIG_DELETE HID_KEYBOARD_SC_D
#define SPECIAL_HKEY_MACROS_ENABLE HID_KEYBOARD_SC_Q
#define SPECIAL_HKEY_MACRO_RECORD  HID_KEYBOARD_SC_W
#define SPECIAL_HKEY_REMAP         HID_KEYBOARD_SC_R
#define SPECIAL_HKEY_REBOOT        HID_KEYBOARD_SC_B
#define SPECIAL_HKEY_RESET_CONFIG  HID_KEYBOARD_SC_C
#define SPECIAL_HKEY_RESET_FULLY   HID_KEYBOARD_SC_F
#define SPECIAL_HKEY_TOGGLE_BUZZER HID_KEYBOARD_SC_Z
#define SPECIAL_HKEY_READ_EEPROM   HID_KEYBOARD_SC_I
#define SPECIAL_HKEY_WRITE_EEPROM  HID_KEYBOARD_SC_O
#define SPECIAL_HKEY_TEST_LEDS     HID_KEYBOARD_SC_L

extern const logical_keycode matrix_to_logical_map[MATRIX_ROWS][MATRIX_COLS] PROGMEM;

/* For each key, maps an index position to a default HID key code. */
/* stored in flash. */
extern const hid_keycode logical_to_hid_map_default[NUM_LOGICAL_KEYS] PROGMEM;


//////////////////////////////////////////////////////////////////////////////////////
// Katy board layout

// Katy keyboard drives (writes) columns and scans (reads) rows.
// However Chris' firmware drives/writes rows, and scans (reads) columns.
// For this reason, we need to change the meaning of rows/cols in
// matrix_select_row() and matrix_read_columns() functions.

// Right hand rows and columns
// 5 rows  (InRRow0-4): PC6 PD7 PE6 PB4 PB7
// 8 cols (OutRCol0-7): PF4-PF7 + PD0-PD3
// Note: OutRCol0-3 (PF4-PF7) is disabled when JTAG is being used.

// For now, no eeprom.

////////////////////////////////////////////////////////////////////////////////////////
#if (ARCH == ARCH_AVR8)
#define RIGHT_MATRIX_IN_1_PORT PORTC
#define RIGHT_MATRIX_IN_1_DDR  DDRC
#define RIGHT_MATRIX_IN_1_MASK (1<<6)
#define RIGHT_MATRIX_IN_1_PIN  PINC
#define RIGHT_MATRIX_IN_2_PORT PORTD
#define RIGHT_MATRIX_IN_2_DDR  DDRD
#define RIGHT_MATRIX_IN_2_MASK (1<<7)
#define RIGHT_MATRIX_IN_2_PIN  PIND
#define RIGHT_MATRIX_IN_3_PORT PORTE
#define RIGHT_MATRIX_IN_3_DDR  DDRE
#define RIGHT_MATRIX_IN_3_MASK (1<<6)
#define RIGHT_MATRIX_IN_3_PIN  PINE
#define RIGHT_MATRIX_IN_4_PORT PORTB
#define RIGHT_MATRIX_IN_4_DDR  DDRB
#define RIGHT_MATRIX_IN_4_MASK 0b10010000
#define RIGHT_MATRIX_IN_4_PIN  PINB

#define RIGHT_MATRIX_OUT_1_PORT PORTF
#define RIGHT_MATRIX_OUT_1_DDR  DDRF
#define RIGHT_MATRIX_OUT_1_MASK 0b11110000
#define RIGHT_MATRIX_OUT_2_PORT PORTD
#define RIGHT_MATRIX_OUT_2_DDR  DDRD
#define RIGHT_MATRIX_OUT_2_MASK 0b00001111


#define LEFT_O_LOAD_DDR  DDRF
#define LEFT_O_LOAD_PORT PORTF
#define LEFT_O_LOAD_MASK _BV(PF0)
#define LEFT_O_LOAD_LOW  LEFT_O_LOAD_PORT &= ~LEFT_O_LOAD_MASK; //_delay_us(1);
#define LEFT_O_LOAD_HIGH LEFT_O_LOAD_PORT |=  LEFT_O_LOAD_MASK; //_delay_us(1);

#define LEFT_I_DATA_DDR  DDRF
#define LEFT_I_DATA_PIN  PINF
#define LEFT_I_DATA_MASK _BV(PF1)

#define LEFT_CLK0_DDR  DDRD
#define LEFT_CLK0_PORT PORTD
#define LEFT_CLK0_MASK _BV(PD5)
#define LEFT_CLK0_HIGH LEFT_CLK0_PORT |=  LEFT_CLK0_MASK; //_delay_us(1);
#define LEFT_CLK0_LOW  LEFT_CLK0_PORT &= ~LEFT_CLK0_MASK; //_delay_us(1);

#define LEFT_CLK1_DDR  DDRB
#define LEFT_CLK1_PORT PORTB
#define LEFT_CLK1_MASK _BV(PB0)
#define LEFT_CLK1_HIGH LEFT_CLK1_PORT |=  LEFT_CLK1_MASK; //_delay_us(1);
#define LEFT_CLK1_LOW  LEFT_CLK1_PORT &= ~LEFT_CLK1_MASK; //_delay_us(1);
#elif (ARCH == ARCH_XMEGA)
// no stuff here (yet)
#else
# error "Unknown architecture."
#endif

#define USE_BUZZER 1

#define LED_CAPS   1
#define LED_NUM    2
#define LED_SCROLL 4
#define LED_KEYPAD 8
#define ALL_LEDS (LED_CAPS | LED_NUM | LED_SCROLL)

void ports_init(void);

#if (ARCH == ARCH_AVR8)
#  define SPI_PORT_SS PORTE
#  define SPI_BIT_SS  PORTE2
   // SS setup time is 100 ns, frequency is 16 MHz ->
   //   spi_slave_on() will use one instruction => one nop here is enough
#  define SPI_EEPROM_CS_SETUP_DELAY asm volatile( "nop\n\t" ::)
   // SS hold time is 200 ns from the last spi_transfer, frequency is 16 MHz ->
   //   the spi_transfer will have at least 3 instructions and 2 instructions will
   //   be used by spi_slave_off() => this delay can be empty
#  define SPI_EEPROM_CS_HOLD_DELAY
#elif  (ARCH == ARCH_XMEGA)
#  define SPI_PORT_SS    PORTB
#  define SPI_BIT_SS_bm  PIN1_bm
   // SS setup time is 100 ns, frequency is 32 MHz ->
   //   1 instruction in spi_slave_on plus this
#  define SPI_EEPROM_CS_SETUP_DELAY asm volatile( "nop\n\tnop\n\tnop\n\t" ::)
   // SS hold time is 100 ns from the last spi_transfer, frequency is 32 MHz
   //   3 instructions in spi_transfer and 1 instruction in spi_slave_off is enough
#  define SPI_EEPROM_CS_HOLD_DELAY
#else
#  error "Unknown architecture."
#endif
void spi_eeprom_enable_write_everywhere(void); // just a prototype; defined in spi_eeprom.c

/**
 * Gets the current physical input for a given physical position
 */
void matrix_select_row(uint8_t matrix_row);
uint8_t matrix_read_column(uint8_t matrix_column);

/* Macros: */
/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
#define LEDMASK_USB_NOTREADY     0xf0

/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
#define LEDMASK_USB_ENUMERATING  0xf1

/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
#define LEDMASK_USB_READY        0xf2

/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
#define LEDMASK_USB_ERROR        0xf3

/** LED mask for the library LED driver, to indicate NOP (no change to the LEDs themselves). */
#define LEDMASK_NOP              0xff

#define LEDMASK_CAPS       LED_CAPS
#define LEDMASK_NUMLOCK    LED_NUM
#define LEDMASK_SCROLLLOCK LED_SCROLL
#define LEDMASK_KEYPAD     LED_KEYPAD

#define LEDMASK_PROGRAMMING_SRC 0x10
#define LEDMASK_PROGRAMMING_DST 0x20
#define LEDMASK_MACRO_TRIGGER   0x30
#define LEDMASK_MACRO_RECORD    0x40
#define LEDMASK_ALL ALL_LEDS
#define LEDMASK_NONE 0

void set_all_leds(uint8_t led_mask);

void test_leds(void);
#if (ARCH == ARCH_AVR8)
  inline bool run_photosensor(uint32_t cur_time_ms) {return false;}
#elif  (ARCH == ARCH_XMEGA)
  bool run_photosensor(uint32_t cur_time_ms);
#else
#  error "Unknown architecture."
#endif

#endif // __KINESIS_H
