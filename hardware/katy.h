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
#define USB_PRODUCT_ID 0x6028 // Katy ergonomic keyboard

// But define different descriptive strings
#define USB_MANUFACTURER_STRING L"andreae.gen.nz"
#define USB_PRODUCT_STRING L"Programmable USB Keyboard"
#define USB_SERIAL_NUMBER_STRING L"andreae.gen.nz:katy"

// Unique identifier representing this keyboard's layout and
// definition of logical_keycode values.  Is reported to the
// configuration program over USB to identify the layout.
#define LAYOUT_ID 2

// For conditional compilation: Do we use keypad layer?
#define KEYPAD_LAYER 1

// if the keypad mode is selected, and a logical key greater than KEYPAD_LAYER_START
// is read, add KEYPAD_LAYER_SIZE to look up the mapping.
#define KEYPAD_LAYER_START 2
#define KEYPAD_LAYER_SIZE  78

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
enum logical_keys {
	LOGICAL_KEY_RROW1, // bottom row (outer)
	LOGICAL_KEY_LROW1, // bottom row (outer)
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
	LOGICAL_KEY_BACKSPACE,
	LOGICAL_KEY_DELETE,
	LOGICAL_KEY_TH_LL, // extra left side left thumb key
	LOGICAL_KEY_TH_LR, // extra left side right thumb key

	// Right hand thumb pad
	LOGICAL_KEY_R_ALT,
	LOGICAL_KEY_R_CTRL,
	LOGICAL_KEY_PGUP,
	LOGICAL_KEY_PGDN,
	LOGICAL_KEY_ENTER,
	LOGICAL_KEY_SPACE,
	LOGICAL_KEY_TH_RL, // extra right side left thumb key
	LOGICAL_KEY_TH_RR, // extra right side right thumb key

	// The keypad layer duplicates the previous 78 keys
};

// Which logical keys to use for special in-built combinations
#define SPECIAL_LKEY_MACRO_RECORD  LOGICAL_KEY_W
#define SPECIAL_LKEY_REMAP         LOGICAL_KEY_R
#define SPECIAL_LKEY_REBOOT        LOGICAL_KEY_B
#define SPECIAL_LKEY_RESET_CONFIG  LOGICAL_KEY_C
#define SPECIAL_LKEY_RESET_FULLY   LOGICAL_KEY_F
#define SPECIAL_LKEY_READ_EEPROM   LOGICAL_KEY_1
#define SPECIAL_LKEY_WRITE_EEPROM  LOGICAL_KEY_2
#define SPECIAL_LKEY_TEST_LEDS     LOGICAL_KEY_3

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

#define USE_BUZZER 1
#define SPECIAL_LKEY_TOGGLE_BUZZER LOGICAL_KEY_RCOL1_1 // minus

#define LED_CAPS   1
#define LED_NUM    2
#define LED_SCROLL 4
#define LED_KEYPAD 8
#define ALL_LEDS (LED_CAPS | LED_NUM | LED_SCROLL)

// EEPROM not initially installed.

void ports_init(void);

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

#endif // __KINESIS_H
