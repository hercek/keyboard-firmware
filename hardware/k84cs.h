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

#ifndef __K84CS_H
#define __K84CS_H

#include "keystate.h"

// Continue using the same USB VID/PID pair that's assigned to the hardware
#define USB_VENDOR_ID  0x1d50 // Openmoko, Inc
#define USB_PRODUCT_ID 0x6028 // ErgoDox ergonomic keyboard

// But define different descriptive strings
#define USB_MANUFACTURER_STRING L"andreae.gen.nz"
#define USB_SERIAL_NUMBER_STRING L"andreae.gen.nz:k84cs"
#define USB_PRODUCT_STRING L"K84CS USB Keyboard"

// Unique identifier representing this keyboard's layout and
// definition of logical_keycode values.  Is reported to the
// configuration program over USB to identify the layout.
#define LAYOUT_ID 4

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
#define KEYPAD_LAYER_SIZE  84

//84 physical keys, 78 of which (all but keypad/program) have a separate keypad layer mapping
#define NUM_LOGICAL_KEYS (KEYPAD_LAYER_START + (KEYPAD_LAYER_SIZE * 3))

// Katy selects columns and reads rows. We're going to reverse the naming convention,
// since our convention is that we select "rows" and read "columns".  We select
// the same row on each side (pulling it low) and then scan the columns (input
// with internal pull-ups).


#define MATRIX_COLS 6  // 5 rows on each side, right side direct, left side via shift register)
#define MATRIX_ROWS 14 // 7 cols on each side

// Logical keys we have: logical keys represent a key-position+keypad-layer combination.
#define LOGICAL_KEY_KEYPAD LOGICAL_KEY_RROW1
#define LOGICAL_KEY_PROGRAM LOGICAL_KEY_LROW1
#define LOGICAL_KEY_LEFT_LAYER_SHIFT LOGICAL_KEY_TH_LL
#define LOGICAL_KEY_RIGHT_LAYER_SHIFT LOGICAL_KEY_TH_RR
enum logical_keys {
	LOGICAL_KEY_PRG,    // program key
	LOGICAL_KEY_LaLck,  // layer lock key
	LOGICAL_KEY_L_KpSh, // keypad layer shift
	LOGICAL_KEY_R_KpSh, // keypad layer shift
	LOGICAL_KEY_L_FnSh, // function layer shift
	LOGICAL_KEY_R_FnSh, // function layer shift
	LOGICAL_KEY_L_PALM, // palm layer shift
	LOGICAL_KEY_R_PALM, // palm layer shift
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
	LOGICAL_KEY_SEMICOL,
	LOGICAL_KEY_COMMA,
	LOGICAL_KEY_PERIOD,
	LOGICAL_KEY_SLASH,
	// Left hand extra key well keys
	LOGICAL_KEY_EQ,
	LOGICAL_KEY_TAB,
	LOGICAL_KEY_ESC,
	LOGICAL_KEY_DEL,
	LOGICAL_KEY_TILDE,
	LOGICAL_KEY_IntK,
	LOGICAL_KEY_LArr,
	LOGICAL_KEY_RArr,
	LOGICAL_KEY_VOL,
	LOGICAL_KEY_CAPS,
	LOGICAL_KEY_PRINT,
	// Right hand extra key well keys
	LOGICAL_KEY_MINUS,
	LOGICAL_KEY_BSLASH,
	LOGICAL_KEY_QUOT,
	LOGICAL_KEY_ENTER,
	LOGICAL_KEY_ClBrc,
	LOGICAL_KEY_OpBrc,
	LOGICAL_KEY_UpArr,
	LOGICAL_KEY_DnArr,
	LOGICAL_KEY_INSERT,
	LOGICAL_KEY_MENU,
	LOGICAL_KEY_BREAK,
	// Left hand thumb pad
	LOGICAL_KEY_L_CTRL,
	LOGICAL_KEY_L_ALT,
	LOGICAL_KEY_HOME,
	LOGICAL_KEY_END,
	LOGICAL_KEY_L_WIN,
	LOGICAL_KEY_L_SH,
	LOGICAL_KEY_BckSp,
	// Right hand thumb pad
	LOGICAL_KEY_R_CTRL,
	LOGICAL_KEY_R_ALT,
	LOGICAL_KEY_PGUP,
	LOGICAL_KEY_PGDN,
	LOGICAL_KEY_R_WIN,
	LOGICAL_KEY_R_SH,
	LOGICAL_KEY_SPACE,
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
// 6 rows  (InRRow0-4): PD0-PD4,PB2
// 7 cols (OutRCol0-6): PA0-PA6

////////////////////////////////////////////////////////////////////////////////////////

#define USE_BUZZER 1

#define LED_CAPS     1
#define LED_NUM      2
#define LED_SCROLL   4
#define LED_KEYPAD   8
#define LED_FUNCTION 16
#define ALL_LEDS (LED_CAPS | LED_NUM | LED_SCROLL)

void ports_init(void);

#define SPI_PORT_SS    PORTB
#define SPI_BIT_SS_bm  PIN1_bm
// SS setup time is 100 ns, frequency is 32 MHz ->
//   1 instruction in spi_slave_on plus this
#define SPI_EEPROM_CS_SETUP_DELAY asm volatile( "nop\n\tnop\n\tnop\n\t" ::)
// SS hold time is 100 ns from the last spi_transfer, frequency is 32 MHz
//   3 instructions in spi_transfer and 1 instruction in spi_slave_off is enough
#define SPI_EEPROM_CS_HOLD_DELAY
void spi_eeprom_enable_write_everywhere(void); // just a prototype; defined in spi_eeprom.c

/**
 * Gets the current physical input for a given physical position
 */
void matrix_select_row(uint8_t matrix_row);
uint8_t matrix_read_column(uint8_t matrix_column);

/* Macros: */
/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
#define LEDMASK_USB_NOTREADY     0xE0

/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
#define LEDMASK_USB_ENUMERATING  0xE1

/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
#define LEDMASK_USB_READY        0xE2

/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
#define LEDMASK_USB_ERROR        0xE3

/** LED mask for the library LED driver, to indicate NOP (no change to the LEDs themselves). */
#define LEDMASK_NOP              0xEf

#define LEDMASK_CAPS       LED_CAPS
#define LEDMASK_NUMLOCK    LED_NUM
#define LEDMASK_SCROLLLOCK LED_SCROLL
#define LEDMASK_KEYPAD     LED_KEYPAD
#define LEDMASK_FUNCTION   LED_FUNCTION

#define LEDMASK_MACROS_ENABLED  0x20
#define LEDMASK_PROGRAMMING_SRC 0x40
#define LEDMASK_PROGRAMMING_DST 0x60
#define LEDMASK_MACRO_TRIGGER   0x80
#define LEDMASK_MACRO_RECORD    0xA0
#define LEDMASK_ALL ALL_LEDS
#define LEDMASK_NONE 0

void set_all_leds(uint8_t led_mask);

void test_leds(void);
bool run_photosensor(uint32_t cur_time_ms);

#endif // __K84CS_H
