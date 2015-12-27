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

#include <util/delay.h>     /* for _delay_ms() */
#include "katy.h"
#include "../buzzer.h"
#include "../Lcd.h"
#include "../printing.h"
//#include "twi.h"

#define KEY_NONE NO_KEY
// Because the matrix may not be tightly packed, we want a map from matrix
// position to logical key.

//#define MATRIX_COLS 5  // 5 rows on each side, right side direct, left side via shift register)
//#define MATRIX_ROWS 16 // 8 cols on each side
const logical_keycode matrix_to_logical_map[MATRIX_ROWS][MATRIX_COLS] PROGMEM =
	// ROW 1               ROW 2               ROW 3               ROW 4               ROW 5
	// Left Hand Side
	// iData0 [in]         iData1 [in]         iData2 [in]         iData3 [in]         iData4 [in]
	{ {LOGICAL_KEY_L_ALT,  LOGICAL_KEY_HOME,   LOGICAL_KEY_END,    LOGICAL_KEY_TH_LR,  LOGICAL_KEY_TH_LL}     // COL 1 | oLoad7 [out]
	, {LOGICAL_KEY_LCOL2_1,LOGICAL_KEY_LCOL2_2,LOGICAL_KEY_LCOL2_3,LOGICAL_KEY_L_CTRL, LOGICAL_KEY_DELETE}    // COL 2 | oLoad6 [out]
	, {LOGICAL_KEY_5,      LOGICAL_KEY_T,      LOGICAL_KEY_G,      LOGICAL_KEY_B,      LOGICAL_KEY_BACKSPACE} // COL 3 | oLoad5 [out]
	, {LOGICAL_KEY_4,      LOGICAL_KEY_R,      LOGICAL_KEY_F,      LOGICAL_KEY_V,      LOGICAL_KEY_LROW5}     // COL 4 | oLoad4 [out]
	, {LOGICAL_KEY_3,      LOGICAL_KEY_E,      LOGICAL_KEY_D,      LOGICAL_KEY_C,      LOGICAL_KEY_LROW4}     // COL 5 | oLoad3 [out]
	, {LOGICAL_KEY_2,      LOGICAL_KEY_W,      LOGICAL_KEY_S,      LOGICAL_KEY_X,      LOGICAL_KEY_LROW3}     // COL 6 | oLoad2 [out]
	, {LOGICAL_KEY_1,      LOGICAL_KEY_Q,      LOGICAL_KEY_A,      LOGICAL_KEY_Z,      LOGICAL_KEY_LROW2}     // COL 7 | oLoad1 [out]
	, {LOGICAL_KEY_LCOL1_1,LOGICAL_KEY_LCOL1_2,LOGICAL_KEY_LCOL1_3,LOGICAL_KEY_LCOL1_4,LOGICAL_KEY_LROW1}     // COL 8 | oLoad0 [out]
	// Right Hand Side
	// PC6 [in]           PD7 [in]            PE6 [in]              PB4 [in]            PB7 [in]
	// XD0 [in]           XD1 [in]            XD2 [in]              XD3 [in]            XD4 [in]
	, {LOGICAL_KEY_TH_RR, LOGICAL_KEY_TH_RL,  LOGICAL_KEY_PGDN,     LOGICAL_KEY_PGUP,   LOGICAL_KEY_R_ALT}   // COL 9  | PF4 XA0 [out]
	, {LOGICAL_KEY_ENTER, LOGICAL_KEY_R_CTRL, LOGICAL_KEY_RCOL2_3,  LOGICAL_KEY_RCOL2_2,LOGICAL_KEY_RCOL2_1} // COL 10 | PF5 XA1 [out]
	, {LOGICAL_KEY_SPACE, LOGICAL_KEY_N,      LOGICAL_KEY_H,        LOGICAL_KEY_Y,      LOGICAL_KEY_6}       // COL 11 | PF6 XA2 [out]
	, {LOGICAL_KEY_RROW5, LOGICAL_KEY_M,      LOGICAL_KEY_J,        LOGICAL_KEY_U,      LOGICAL_KEY_7}       // COL 12 | PF7 XA3 [out]
	, {LOGICAL_KEY_RROW4, LOGICAL_KEY_COMMA,  LOGICAL_KEY_K,        LOGICAL_KEY_I,      LOGICAL_KEY_8}       // COL 13 | PD0 XA4 [out]
	, {LOGICAL_KEY_RROW3, LOGICAL_KEY_PERIOD, LOGICAL_KEY_L,        LOGICAL_KEY_O,      LOGICAL_KEY_9}       // COL 14 | PD1 XA5 [out]
	, {LOGICAL_KEY_RROW2, LOGICAL_KEY_SLASH,  LOGICAL_KEY_SEMICOLON,LOGICAL_KEY_P,      LOGICAL_KEY_0}       // COL 15 | PD2 XA6 [out]
	, {LOGICAL_KEY_RROW1, LOGICAL_KEY_RCOL1_4,LOGICAL_KEY_RCOL1_3,  LOGICAL_KEY_RCOL1_2,LOGICAL_KEY_RCOL1_1} // COL 16 | PD3 XA7 [out]
	};
#undef KEY_NONE

const hid_keycode logical_to_hid_map_default[NUM_LOGICAL_KEYS] PROGMEM = {
	// non-keypad layer
	SPECIAL_HID_KEY_PROGRAM,						   //	LOGICAL_KEY_LROW1, // left bottom row (outer)
	SPECIAL_HID_KEY_KEYPAD_TOGGLE,					   //	LOGICAL_KEY_RROW1, // right bottom row (outer)
	SPECIAL_HID_KEY_KEYPAD_SHIFT,					   //	LOGICAL_KEY_TH_LL, // extra left side left thumb key
	SPECIAL_HID_KEY_KEYPAD_SHIFT,					   //	LOGICAL_KEY_TH_RR, // extra right side right thumb key
	//---------------------------------------------- on-the-fly remapable
	HID_KEYBOARD_SC_A,								   //	LOGICAL_KEY_A
	HID_KEYBOARD_SC_B,								   //	LOGICAL_KEY_B
	HID_KEYBOARD_SC_C,								   //	LOGICAL_KEY_C
	HID_KEYBOARD_SC_D,								   //	LOGICAL_KEY_D
	HID_KEYBOARD_SC_E,								   //	LOGICAL_KEY_E
	HID_KEYBOARD_SC_F,								   //	LOGICAL_KEY_F
	HID_KEYBOARD_SC_G,								   //	LOGICAL_KEY_G
	HID_KEYBOARD_SC_H,								   //	LOGICAL_KEY_H
	HID_KEYBOARD_SC_I,								   //	LOGICAL_KEY_I
	HID_KEYBOARD_SC_J,								   //	LOGICAL_KEY_J
	HID_KEYBOARD_SC_K,								   //	LOGICAL_KEY_K
	HID_KEYBOARD_SC_L,								   //	LOGICAL_KEY_L
	HID_KEYBOARD_SC_M,								   //	LOGICAL_KEY_M
	HID_KEYBOARD_SC_N,								   //	LOGICAL_KEY_N
	HID_KEYBOARD_SC_O,								   //	LOGICAL_KEY_O
	HID_KEYBOARD_SC_P,								   //	LOGICAL_KEY_P
	HID_KEYBOARD_SC_Q,								   //	LOGICAL_KEY_Q
	HID_KEYBOARD_SC_R,								   //	LOGICAL_KEY_R
	HID_KEYBOARD_SC_S,								   //	LOGICAL_KEY_S
	HID_KEYBOARD_SC_T,								   //	LOGICAL_KEY_T
	HID_KEYBOARD_SC_U,								   //	LOGICAL_KEY_U
	HID_KEYBOARD_SC_V,								   //	LOGICAL_KEY_V
	HID_KEYBOARD_SC_W,								   //	LOGICAL_KEY_W
	HID_KEYBOARD_SC_X,								   //	LOGICAL_KEY_X
	HID_KEYBOARD_SC_Y,								   //	LOGICAL_KEY_Y
	HID_KEYBOARD_SC_Z,								   //	LOGICAL_KEY_Z
	HID_KEYBOARD_SC_1_AND_EXCLAMATION,				   //	LOGICAL_KEY_1
	HID_KEYBOARD_SC_2_AND_AT,						   //	LOGICAL_KEY_2
	HID_KEYBOARD_SC_3_AND_HASHMARK,					   //	LOGICAL_KEY_3
	HID_KEYBOARD_SC_4_AND_DOLLAR,					   //	LOGICAL_KEY_4
	HID_KEYBOARD_SC_5_AND_PERCENTAGE,				   //	LOGICAL_KEY_5
	HID_KEYBOARD_SC_6_AND_CARET,					   //	LOGICAL_KEY_6
	HID_KEYBOARD_SC_7_AND_AMPERSAND,				   //	LOGICAL_KEY_7
	HID_KEYBOARD_SC_8_AND_ASTERISK,					   //	LOGICAL_KEY_8
	HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,		   //	LOGICAL_KEY_9
	HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,		   //	LOGICAL_KEY_0
	HID_KEYBOARD_SC_SEMICOLON_AND_COLON,			   //	LOGICAL_KEY_SEMICOLON
	HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,		   //	LOGICAL_KEY_COMMA
	HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,		   //	LOGICAL_KEY_PERIOD
	HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,		   //	LOGICAL_KEY_SLASH
    // LHS extra keys
	HID_KEYBOARD_SC_EQUAL_AND_PLUS,					   //	LOGICAL_KEY_LCOL1_1, // outer column (top)
	HID_KEYBOARD_SC_TAB, 							   //	LOGICAL_KEY_LCOL1_2,
	HID_KEYBOARD_SC_ESCAPE,							   //	LOGICAL_KEY_LCOL1_3,
	HID_KEYBOARD_SC_DELETE,							   //	LOGICAL_KEY_LCOL1_4,
	HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,			   //	LOGICAL_KEY_LROW2, // bottom row (outer)
	HID_KEYBOARD_SC_NON_US_BACKSLASH_AND_PIPE,		   //	LOGICAL_KEY_LROW3,
	HID_KEYBOARD_SC_LEFT_ARROW,						   //	LOGICAL_KEY_LROW4,
	HID_KEYBOARD_SC_RIGHT_ARROW,					   //	LOGICAL_KEY_LROW5,
	HID_KEYBOARD_SC_PRINT_SCREEN,					   //	LOGICAL_KEY_LCOL2_1, // inner column (top)
	HID_KEYBOARD_SC_CAPS_LOCK,						   //	LOGICAL_KEY_LCOL2_2,
	HID_KEYBOARD_SC_VOLUME_DOWN,					   //	LOGICAL_KEY_LCOL2_3,
	// Right hand extra keys
	HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,			   //	LOGICAL_KEY_RCOL1_1, // outer column (top)
	HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,				   //	LOGICAL_KEY_RCOL1_2,
	HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE, 			   //	LOGICAL_KEY_RCOL1_3,
	HID_KEYBOARD_SC_ENTER,							   //	LOGICAL_KEY_RCOL1_4,
	HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE, //	LOGICAL_KEY_RROW2, // bottom row (outer)
	HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE, //	LOGICAL_KEY_RROW3,
	HID_KEYBOARD_SC_UP_ARROW,						   //	LOGICAL_KEY_RROW4,
	HID_KEYBOARD_SC_DOWN_ARROW,						   //	LOGICAL_KEY_RROW5,
	HID_KEYBOARD_SC_PAUSE,							   //	LOGICAL_KEY_RCOL2_1, // inner column (top)
	0x65/*WinContextMenu (not in LUFA header)*/,	   //	LOGICAL_KEY_RCOL2_2,
	HID_KEYBOARD_SC_INSERT,							   //	LOGICAL_KEY_RCOL2_3,
	// Left hand thumbpad
	HID_KEYBOARD_SC_LEFT_ALT, 						   //	LOGICAL_KEY_L_ALT,
	HID_KEYBOARD_SC_LEFT_CONTROL,					   //	LOGICAL_KEY_L_CTRL,
	HID_KEYBOARD_SC_HOME,							   //	LOGICAL_KEY_HOME,
	HID_KEYBOARD_SC_END,							   //	LOGICAL_KEY_END,
	HID_KEYBOARD_SC_LEFT_SHIFT,						   //	LOGICAL_KEY_DELETE,
	HID_KEYBOARD_SC_BACKSPACE,						   //	LOGICAL_KEY_BACKSPACE,
	HID_KEYBOARD_SC_LEFT_GUI,						   //	LOGICAL_KEY_TH_LR, // extra left side right thumb key
	// Right hand thumb pad
	HID_KEYBOARD_SC_RIGHT_ALT,						   //	LOGICAL_KEY_R_ALT,
	HID_KEYBOARD_SC_RIGHT_CONTROL,					   //	LOGICAL_KEY_R_CTRL,
	HID_KEYBOARD_SC_PAGE_UP,						   //	LOGICAL_KEY_PGUP,
	HID_KEYBOARD_SC_PAGE_DOWN,						   //	LOGICAL_KEY_PGDN,
	HID_KEYBOARD_SC_RIGHT_SHIFT,					   //	LOGICAL_KEY_ENTER,
	HID_KEYBOARD_SC_SPACE,							   //	LOGICAL_KEY_SPACE,
	HID_KEYBOARD_SC_RIGHT_GUI,						   //	LOGICAL_KEY_TH_RL, // extra right side left thumb key
	/////////////////////////////////////////////////
	// keypad layer									   //	***/### keypad mode default differs to base layer
	NO_KEY,		//SPECIAL_HID_KEY_PROGRAM
	NO_KEY,		//SPECIAL_HID_KEY_KEYPAD_TOGGLE
	NO_KEY,		//SPECIAL_HID_KEY_KEYPAD_SHIFT
	NO_KEY,		//SPECIAL_HID_KEY_KEYPAD_SHIFT
	//---------------------------------------------- on-the-fly remapable
	SPECIAL_HID_KEY_MOUSE_BTN1,						   //	LOGICAL_KEY_A ###
	HID_KEYBOARD_SC_B,								   //	LOGICAL_KEY_B
	HID_KEYBOARD_SC_C,								   //	LOGICAL_KEY_C
	SPECIAL_HID_KEY_MOUSE_BACK, 					   //	LOGICAL_KEY_D ###
	SPECIAL_HID_KEY_MOUSE_FWD, 						   //	LOGICAL_KEY_E ###
	SPECIAL_HID_KEY_MOUSE_RIGHT, 					   //	LOGICAL_KEY_F ###
	HID_KEYBOARD_SC_G,								   //	LOGICAL_KEY_G
	HID_KEYBOARD_SC_KEYPAD_SLASH,					   //	LOGICAL_KEY_H ***
	HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW,			   //	LOGICAL_KEY_I ***
	HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW,		   //	LOGICAL_KEY_J ***
	HID_KEYBOARD_SC_KEYPAD_5,						   //	LOGICAL_KEY_K ***
	HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW,		   //	LOGICAL_KEY_L ***
	HID_KEYBOARD_SC_KEYPAD_1_AND_END,				   //	LOGICAL_KEY_M ***
	HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT,			   //	LOGICAL_KEY_N ***
	HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP,			   //	LOGICAL_KEY_O ***
	HID_KEYBOARD_SC_KEYPAD_PLUS,					   //	LOGICAL_KEY_P ***
	SPECIAL_HID_KEY_MOUSE_BTN2,						   //	LOGICAL_KEY_Q ###
	SPECIAL_HID_KEY_MOUSE_BTN4,						   //	LOGICAL_KEY_R ###
	SPECIAL_HID_KEY_MOUSE_LEFT,						   //	LOGICAL_KEY_S ###
	SPECIAL_HID_KEY_MOUSE_BTN5,						   //	LOGICAL_KEY_T ###
	HID_KEYBOARD_SC_KEYPAD_7_AND_HOME,				   //	LOGICAL_KEY_U ***
	HID_KEYBOARD_SC_V,								   //	LOGICAL_KEY_V
	SPECIAL_HID_KEY_MOUSE_BTN3,						   //	LOGICAL_KEY_W ###
	HID_KEYBOARD_SC_X,								   //	LOGICAL_KEY_X
	HID_KEYBOARD_SC_KEYPAD_ASTERISK,				   //	LOGICAL_KEY_Y ***
	HID_KEYBOARD_SC_MUTE,							   //	LOGICAL_KEY_Z ***
	HID_KEYBOARD_SC_F1,								   //	LOGICAL_KEY_1 ***
	HID_KEYBOARD_SC_F2,								   //	LOGICAL_KEY_2 ***
	HID_KEYBOARD_SC_F3,								   //	LOGICAL_KEY_3 ***
	HID_KEYBOARD_SC_F4,								   //	LOGICAL_KEY_4 ***
	HID_KEYBOARD_SC_F5,								   //	LOGICAL_KEY_5 ***
	HID_KEYBOARD_SC_F6,								   //	LOGICAL_KEY_6 ***
	HID_KEYBOARD_SC_F7,								   //	LOGICAL_KEY_7 ***
	HID_KEYBOARD_SC_F8,								   //	LOGICAL_KEY_8 ***
	HID_KEYBOARD_SC_F9,								   //	LOGICAL_KEY_9 ***
	HID_KEYBOARD_SC_F10,							   //	LOGICAL_KEY_0 ***
	HID_KEYBOARD_SC_KEYPAD_MINUS,					   //	LOGICAL_KEY_SEMICOLON ***
	HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW,		   //	LOGICAL_KEY_COMMA ***
	HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN,			   //	LOGICAL_KEY_PERIOD ***
	HID_KEYBOARD_SC_KEYPAD_ENTER,					   //	LOGICAL_KEY_SLASH ***
    // LHS extra keys
	HID_KEYBOARD_SC_EQUAL_AND_PLUS,					   //	LOGICAL_KEY_LCOL1_1, // outer column (top)
	HID_KEYBOARD_SC_TAB, 							   //	LOGICAL_KEY_LCOL1_2,
	HID_KEYBOARD_SC_ESCAPE,							   //	LOGICAL_KEY_LCOL1_3,
	HID_KEYBOARD_SC_DELETE,							   //	LOGICAL_KEY_LCOL1_4,
	HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,			   //	LOGICAL_KEY_LROW2, // bottom row (outer)
	HID_KEYBOARD_SC_NON_US_BACKSLASH_AND_PIPE,		   //	LOGICAL_KEY_LROW3,
	HID_KEYBOARD_SC_LEFT_ARROW,						   //	LOGICAL_KEY_LROW4,
	HID_KEYBOARD_SC_RIGHT_ARROW,					   //	LOGICAL_KEY_LROW5,
	HID_KEYBOARD_SC_F11,							   //	LOGICAL_KEY_LCOL2_1, *** // inner column (top)
	HID_KEYBOARD_SC_CAPS_LOCK,						   //	LOGICAL_KEY_LCOL2_2,
	HID_KEYBOARD_SC_VOLUME_UP,						   //	LOGICAL_KEY_LCOL2_3, ***
	// Right hand extra keys
	HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,			   //	LOGICAL_KEY_RCOL1_1, // outer column (top)
	HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,				   //	LOGICAL_KEY_RCOL1_2,
	HID_KEYBOARD_SC_KEYPAD_EQUAL_SIGN, 				   //	LOGICAL_KEY_RCOL1_3, ***
	HID_KEYBOARD_SC_ENTER,							   //	LOGICAL_KEY_RCOL1_4,
	HID_KEYBOARD_SC_KEYPAD_COMMA,					   //	LOGICAL_KEY_RROW2, *** // bottom row (outer)
	HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE, 			   //	LOGICAL_KEY_RROW3, ***
	HID_KEYBOARD_SC_UP_ARROW,						   //	LOGICAL_KEY_RROW4,
	HID_KEYBOARD_SC_DOWN_ARROW,						   //	LOGICAL_KEY_RROW5,
	HID_KEYBOARD_SC_F12,							   //	LOGICAL_KEY_RCOL2_1, *** // inner column
	HID_KEYBOARD_SC_NUM_LOCK,						   //	LOGICAL_KEY_RCOL2_2, ***
	HID_KEYBOARD_SC_F13,							   //	LOGICAL_KEY_RCOL2_3, ***
	// Left hand thumbpad
	HID_KEYBOARD_SC_LEFT_ALT, 						   //	LOGICAL_KEY_L_ALT,
	HID_KEYBOARD_SC_LEFT_CONTROL,					   //	LOGICAL_KEY_L_CTRL,
	HID_KEYBOARD_SC_HOME,							   //	LOGICAL_KEY_HOME,
	HID_KEYBOARD_SC_END,							   //	LOGICAL_KEY_END,
	HID_KEYBOARD_SC_LEFT_SHIFT, 					   //	LOGICAL_KEY_DELETE,
	HID_KEYBOARD_SC_BACKSPACE,						   //	LOGICAL_KEY_BACKSPACE,
	HID_KEYBOARD_SC_LEFT_GUI,						   //	LOGICAL_KEY_TH_LR, // extra left side right thumb key
	// Right hand thumbpad
	HID_KEYBOARD_SC_RIGHT_ALT,						   //	LOGICAL_KEY_R_ALT,
	HID_KEYBOARD_SC_RIGHT_CONTROL,					   //	LOGICAL_KEY_R_CTRL,
	HID_KEYBOARD_SC_PAGE_UP,						   //	LOGICAL_KEY_PGUP,
	HID_KEYBOARD_SC_PAGE_DOWN,						   //	LOGICAL_KEY_PGDN,
	HID_KEYBOARD_SC_RIGHT_SHIFT,					   //	LOGICAL_KEY_ENTER,
	HID_KEYBOARD_SC_SPACE,							   //	LOGICAL_KEY_SPACE,
	HID_KEYBOARD_SC_RIGHT_GUI,						   //	LOGICAL_KEY_TH_RL, // extra right side left thumb key
};


// SPI EEPROM related stuf:
#if (ARCH == ARCH_AVR8)
#  define SPI_DD_SS   DDE2
#  define SPI_DD_SCK  DDB1
#  define SPI_DD_MOSI DDB2
#  define SPI_DD_MISO DDB3
#  define SPI_DDR_SS   DDRE
#  define SPI_DDR_SCK  DDRB
#  define SPI_DDR_MOSI DDRB
#  define SPI_DDR_MISO DDRB
#elif  (ARCH == ARCH_XMEGA)
#  define SPI_DD_SS    PIN1_bp
#  define SPI_DD_SCK   PIN7_bp
#  define SPI_DD_MOSI  PIN5_bp
#  define SPI_DD_MISO  PIN6_bp
#  define SPI_DDR_SS   PORTB_DIR
#  define SPI_DDR_SCK  PORTC_DIR
#  define SPI_DDR_MOSI PORTC_DIR
#  define SPI_DDR_MISO PORTC_DIR
#else
#  error "Unknown architecture."
#endif


static void serial_eeprom_init(void) {
	// Initialize Pins.
	SPI_DDR_SS |= _BV(SPI_DD_SS);      //OUTPUT
	SPI_DDR_SCK |= _BV(SPI_DD_SCK);    //OUTPUT
	SPI_DDR_MOSI |= _BV(SPI_DD_MOSI);  //OUTPUT
	SPI_DDR_MISO &= ~_BV(SPI_DD_MISO); //INPUT
#if (ARCH == ARCH_AVR8)
	SPI_PORT_SS |= _BV(SPI_BIT_SS); // Keep slave inactive (set EEPROM CS to high).
	// Make sure that original ATMega32u4 SS (PB0) is set to output, otherwise
	// we could get MSTR bit reset to 0 (see ATMega32U4.pdf, page 182).
	DDRB |= _BV(DDB0); // for KATY, PB0 is CLK1, which is output, so this is OK
	// Initialize SPI subsystem in master mode, clock set to 4 MHz
	SPCR = _BV(SPE) | _BV(MSTR); //ATMega32U4.pdf:182
	SPSR &= ~_BV(SPI2X); // do not raise clock from 4 to 8 MHz
#elif  (ARCH == ARCH_XMEGA)
	SPI_PORT_SS.OUTSET = SPI_BIT_SS_bm; // Keep slave inactive (set EEPROM CS to high).
	// Initialize SPI subsystem: master, 8 MHz (DIV4_gc)
	SPIC_CTRL = SPI_ENABLE_bm \
	          | SPI_MASTER_bm \
	          | SPI_CLK2X_bm \
	          | SPI_PRESCALER_DIV4_gc \
	          | SPI_MODE_0_gc;
#else
#  error "Unknown architecture."
#endif
	// check writing is enabled into to whole EEPROM
	serial_eeprom_enable_write_everywhere();
}


#if (ARCH == ARCH_XMEGA)

static uint8_t read_calibration_byte(uint8_t index) {
	uint8_t result;
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result = pgm_read_byte(index);
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return result;
}

static void photosensor_init(void) {
	// we need to know time progression to ramp-up photoPwr -> use TCC0
	TCC0.CTRLA = TC_CLKSEL_DIV64_gc; // increment time every 2 µs
	TCC0.PER = 0xffff; // wrap timer every 65536*2 µs
	// setup photoDrv (preserve the original DAC-CH1 settings
	PORTB.DIRSET = PIN2_bm; // photoDrv is output
	DACB.CTRLA |= DAC_CH0EN_bm; // enable channel 0 too
	DACB.CTRLB = DAC_CHSEL1_bm; // dual channel operation
	DACB.CH0DATA = 0x0; // photoPwr is switched off at boot
	// Set up ADC for photoSns
	//PORTB.DIRCLR = PIN0_bm; // photoSns is input
	//PORTB.PIN0CTRL = PORT_OPC_TOTEM_gc; // No pull up nor pull down on photoSns
	// First read is wrong, discard it
	ADCA.CALL = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	ADCA.CALL = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	ADCA.CALH = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));
	ADCA.CALH = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc; // slow down to increas precision
	ADCA.REFCTRL = ADC_REFSEL_INTVCC_gc; // set ADC reference to Vcc/1.6
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc; // single-ended, no gain
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN8_gc; //PB0 == ADC8 (a4u datasheet, page 58)
	ADCA.CTRLA = ADC_ENABLE_bm;
}

enum photosensor_state {
	PHOTOSENSOR_OFF,
	PHOTOSENSOR_STARTING,
	PHOTOSENSOR_STARTED,
	PHOTOSENSOR_WAITING,
	PHOTOSENSOR_RUNNING
};

void set_all_leds_ex(uint8_t led_mask, int16_t lux_val);
inline uint16_t get_photoDrv_voltage_code( float u ) {
	return (uint16_t)( u * 4096.0f / 3.3f );
}

bool run_photosensor(uint32_t cur_time_ms) {
	static enum photosensor_state state = PHOTOSENSOR_OFF;
	static uint32_t next_step_time_ms = 500;
	static uint16_t next_step_time_2us;
	static uint16_t adc_rv[20];
	static uint8_t index;
	const uint8_t adc_rv_array_size = sizeof(adc_rv)/sizeof(adc_rv[0]);
	const uint16_t time_2us_inc = 7; // 14 µs time increment

	switch (state) {
	case PHOTOSENSOR_OFF:
		if (cur_time_ms < next_step_time_ms) return false;
		DACB.CH0DATA = get_photoDrv_voltage_code(1.1f);
		next_step_time_2us = TCC0.CNT + time_2us_inc;
		state = PHOTOSENSOR_STARTING;
		return false;
	case PHOTOSENSOR_STARTING:
		if ( TCC0.CNT < next_step_time_2us ) return false;
		if ( TCC0.CNT - next_step_time_2us > 0x8000 ) return false;
		if ( DACB.CH0DATA >= get_photoDrv_voltage_code(2.6f) ) {
			DACB.CH0DATA = 0xfff;
			next_step_time_2us = TCC0.CNT + 51; // 102 ms increment
			state = PHOTOSENSOR_STARTED;
			return false; }
		DACB.CH0DATA += get_photoDrv_voltage_code(0.115f);
		next_step_time_2us = TCC0.CNT + time_2us_inc;
		return false;
	case PHOTOSENSOR_STARTED:
		if ( TCC0.CNT < next_step_time_2us ) return false;
		if ( TCC0.CNT - next_step_time_2us > 0x8000 ) return false;
		index = 0;
		next_step_time_ms = cur_time_ms + 1;
		state = PHOTOSENSOR_WAITING;
		return false;
	case PHOTOSENSOR_WAITING:
		if (cur_time_ms < next_step_time_ms) return false;
		ADCA.CH0.CTRL |= ADC_CH_START_bm; // start the ADC
		next_step_time_ms = cur_time_ms + 1;
		state = PHOTOSENSOR_RUNNING;
		return false;
	case PHOTOSENSOR_RUNNING:
		if (!(ADCA.CH0.INTFLAGS & ADC_CH0IF_bm)) return false; // Wait for conversion to be finished.
		adc_rv[index] = ADCA.CH0RES & 0x0FFF; // get the 12bit value
		if (++index < adc_rv_array_size) {
			next_step_time_ms = cur_time_ms + 1;
			state = PHOTOSENSOR_WAITING;
			return false;
		}
		// OK, we finished reading the whole photo sensor data set
		{
			uint32_t adc_average = 0;
#ifdef KATY_DEBUG
			uint16_t adc_max = 0;
			uint16_t adc_min = -1;
			static char adc_string[12];
#endif
			for (uint8_t i = 0; i < adc_rv_array_size; ++i) {
				adc_average += adc_rv[i];
#ifdef KATY_DEBUG
				if (adc_max < adc_rv[i]) adc_max = adc_rv[i];
				if (adc_min > adc_rv[i]) adc_min = adc_rv[i];
#endif
			}
			adc_average /= adc_rv_array_size;
			//int16_t lux_val = (int16_t)(adc_average*0.54945055f - 100.0f); // lux estimate from spec
			int16_t lux_val = adc_average - 182; // use raw value (remove only the ADC zero shift)
			set_all_leds_ex(LEDMASK_NOP, lux_val);
#ifdef KATY_DEBUG
			sprintf(adc_string, "%d %d\n", lux_val, adc_max-adc_min);
			printing_set_buffer(adc_string,BUF_MEM);
			PORTE.DIRTGL = PIN3_bm;
#endif
		}
		DACB.CH0DATA = get_photoDrv_voltage_code(1.1f); // power down the photosensor
		next_step_time_ms = cur_time_ms + 1000; // plan reading of the next data set in 1 second
		state = PHOTOSENSOR_OFF;
#ifdef KATY_DEBUG
		return true;
#else
		return false;
#endif
	}//switch
	return false;
}

#endif

#if (ARCH == ARCH_AVR8)
#define red_led_off()
#define lit_red_led(l)
#define yellow_led_off()
#define lit_yellow_led(l)
#define green_led_off()
#define lit_green_led(l)
#define blue_led_off()
#define lit_blue_led(l)
#elif (ARCH == ARCH_XMEGA)
float const lux_val_q = 0.96;
float const lux_val_k = 40;
float const red_led_base = 128;
float const yellow_led_base = 144;
float const green_led_base = 21;
float const blue_led_base = 69;

inline uint16_t round_to_uint16( float x ) {
	if (x < 0) return 0;
	if (x > 0xffff) return 0xffff;
	return (uint16_t)(x+0.5);
}

inline void red_led_off(void) {PORTE.DIRCLR=PIN0_bm;}
inline void lit_red_led(uint16_t lux_val) {
	TCE0.CCA = round_to_uint16(lux_val * red_led_base/lux_val_k + red_led_base*lux_val_q); PORTE.DIRSET=PIN0_bm; }
inline void yellow_led_off(void) {PORTE.DIRCLR=PIN1_bm;}
inline void lit_yellow_led(uint16_t lux_val) {
	TCE0.CCB = round_to_uint16(lux_val * yellow_led_base/lux_val_k + yellow_led_base*lux_val_q); PORTE.DIRSET=PIN1_bm; }
inline void green_led_off(void) {PORTE.DIRCLR=PIN2_bm;}
inline void lit_green_led(uint16_t lux_val) {
	TCE0.CCC = round_to_uint16(lux_val * green_led_base/lux_val_k + green_led_base*lux_val_q); PORTE.DIRSET=PIN2_bm; }
inline void blue_led_off(void) {PORTE.DIRCLR=PIN3_bm;}
inline void lit_blue_led(uint16_t lux_val) {
	TCE0.CCD = round_to_uint16(lux_val * blue_led_base/lux_val_k + blue_led_base*lux_val_q); PORTE.DIRSET=PIN3_bm; }
#else
# error "Unknown architecture."
#endif

void ports_init(void){
#if (ARCH == ARCH_AVR8)
	// Set up internal pull-ups for input pins, scanning by pulling low
	RIGHT_MATRIX_IN_1_DDR  &= ~RIGHT_MATRIX_IN_1_MASK;
	RIGHT_MATRIX_IN_1_PORT |=  RIGHT_MATRIX_IN_1_MASK;
	RIGHT_MATRIX_IN_2_DDR  &= ~RIGHT_MATRIX_IN_2_MASK;
	RIGHT_MATRIX_IN_2_PORT |=  RIGHT_MATRIX_IN_2_MASK;
	RIGHT_MATRIX_IN_3_DDR  &= ~RIGHT_MATRIX_IN_3_MASK;
	RIGHT_MATRIX_IN_3_PORT |=  RIGHT_MATRIX_IN_3_MASK;
	RIGHT_MATRIX_IN_4_DDR  &= ~RIGHT_MATRIX_IN_4_MASK;
	RIGHT_MATRIX_IN_4_PORT |=  RIGHT_MATRIX_IN_4_MASK;

	// Set up output columns, initialize into high-z state
	RIGHT_MATRIX_OUT_1_DDR  &= ~RIGHT_MATRIX_OUT_1_MASK; // 0 = input
	RIGHT_MATRIX_OUT_1_PORT &= ~RIGHT_MATRIX_OUT_1_MASK; // 0 = high-z
	RIGHT_MATRIX_OUT_2_DDR  &= ~RIGHT_MATRIX_OUT_2_MASK; // 0 = input
	RIGHT_MATRIX_OUT_2_PORT &= ~RIGHT_MATRIX_OUT_2_MASK; // 0 = high-z

	// Set up left hand side
	LEFT_O_LOAD_DDR  |= LEFT_O_LOAD_MASK;  //oLoad is output pin
	LEFT_I_DATA_DDR  &= ~LEFT_I_DATA_MASK; //iData is input pin

	LEFT_O_LOAD_HIGH; //set oLoad to high (do not reset register 165)

	LEFT_CLK0_DDR |= LEFT_CLK0_MASK;  //CLK0 is output pin
	LEFT_CLK1_DDR |= LEFT_CLK1_MASK;  //CLK1 is output pin
	LEFT_CLK0_LOW;
	LEFT_CLK1_LOW;
	for (int8_t i = 0; i <= 7; ++i) {
		LEFT_CLK0_HIGH;
		LEFT_CLK0_LOW;
	}
#elif (ARCH == ARCH_XMEGA)
	// Katy on atxmega uses PD0-PD4 (5 pins) for rows (input) and shares
	// columns with LCD as output, lines LcdD0-LcdD7.

	// First, set up rows.
	PORTD.DIRCLR = 0b00011111;           // PD0-PD4 as input
	PORTCFG.MPCMASK = 0b00011111;        // Set up multi-pin port configuration.
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc; // Set pull-up on input pins.

	// Now set up columns. Columns share pins with LCD, lines LcdD0-LcdD7.
	PORTA.DIRCLR = 0b11111111;             // PA0-PA7 as input.
	PORTCFG.MPCMASK = 0b11111111;          // Multi pin configuration...

	// Set up left hand side.
	// Set oLoad as output ...
	PORTC.DIRSET = PIN3_bm;
	// ... and set it to high (do not reset register 165)
	PORTC.OUTSET = PIN3_bm;
	// Set up iData as input with internal pullup
	PORTC.DIRCLR = PIN0_bm;
	PORTC.PIN0CTRL = PORT_OPC_PULLUP_gc;
	// Set up CLK0 and CLK1 as output pins ...
	PORTC.DIRSET = PIN1_bm | PIN2_bm;
	PORTC.OUTCLR = PIN1_bm | PIN2_bm;
	for (int8_t i = 0; i <= 7; ++i) {
		PORTC.OUTSET = PIN1_bm;
		PORTC.OUTCLR = PIN1_bm;
	}
	// Set up ATXMEGA related to LCD
	// Set up LcdV
	DACB.CH0OFFSETCAL = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, DACB0OFFCAL));
	DACB.CH0GAINCAL   = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, DACB0GAINCAL));
	// channel 1 calibration data are not valid
	//DACB.CH1OFFSETCAL = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, DACB1OFFCAL));
	//DACB.CH1GAINCAL   = read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, DACB1GAINCAL));
	PORTB.DIRSET = PIN3_bm; //PORTB.OUTCLR = PIN3_bm;
	DACB.CTRLA = DAC_CH1EN_bm | DAC_LPMODE_bm | DAC_ENABLE_bm;
	DACB.CTRLB = DAC_CHSEL0_bm; // single channel operation on channel 1
	DACB.CTRLC = DAC_REFSEL0_bm; // DAC reference set to AVcc
	//DACB.CH1DATA = 0x0000; // 0.0308V - display is readable, too black however
	DACB.CH1DATA = 0x0300; // 0.5925V - display is well readable
	//DACB.CH1DATA = 0x0380; // 0.6944V - display is readable
	//DACB.CH1DATA = 0x0400; // 0.7960V - display is readable ok
	//DACB.CH1DATA = 0x0480; // 0.8979V - visible, not ideal
	//DACB.CH1DATA = 0x0500; // 0.9995V - badly visible
	//DACB.CH1DATA = 0x0600; // 1.2033V - veeery badly visible
	//DACB.CH1DATA = 0x0800; // too high, 1.6108V - display is empty
	//DACB.CH1DATA = 0x0FFF;

	// Set up LcdLED
	PORTD.DIRSET = PIN5_bm;
	//PORTD.OUTSET = PIN5_bm;
	//PORTD.OUTCLR = PIN5_bm;
	TCD1.CTRLB = TC_WGMODE_DS_B_gc | TC1_CCBEN_bm;
	TCD1.PER = 0xFFFF;
	TCD1.CCB = 0x1000;
	TCD1.CTRLA = TC_CLKSEL_DIV1_gc;

	// Setup LEDs
	//PORTE.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
	TCE0.CTRLB = TC_WGMODE_DS_B_gc | TC0_CCAEN_bm | TC0_CCBEN_bm | TC0_CCCEN_bm | TC0_CCDEN_bm;
	TCE0.PER = 0xFFFF;
	lit_red_led(lux_val_k);
	lit_yellow_led(lux_val_k);
	lit_green_led(lux_val_k);
	lit_blue_led(lux_val_k);
	TCE0.CTRLA = TC_CLKSEL_DIV1_gc;

	// Set up Photo Transistor
	photosensor_init();
#else
# error "Unknown architecture."
#endif

	// Set up LEDs.
	lcd_init();
	lcd_print_position(0, 0, "K A T Y");
	lcd_print_position(1, 0, "Keyboard");

#if USE_BUZZER
	buzzer_init();;
#endif

	serial_eeprom_init();
}


static uint8_t processing_row = 0;

void matrix_select_row(uint8_t matrix_row){
#if (ARCH == ARCH_AVR8)
	// KATY: we actually select cols here (replace rows with cols here)
	if (matrix_row < MATRIX_ROWS/2) {
		// handle left hand side
		if (matrix_row == 0) {
			LEFT_O_LOAD_LOW;
			LEFT_CLK0_HIGH;
			LEFT_CLK0_LOW;
			_delay_us(1);
		}
		LEFT_O_LOAD_LOW;
		LEFT_O_LOAD_HIGH;
		LEFT_CLK0_HIGH;
		LEFT_CLK0_LOW;
	} else {
		// handle right hand side
		// Reset all matrix rows to input (high-z)
		RIGHT_MATRIX_OUT_1_DDR  &= ~RIGHT_MATRIX_OUT_1_MASK; // 0 = input
		RIGHT_MATRIX_OUT_2_DDR  &= ~RIGHT_MATRIX_OUT_2_MASK;
		// Set the selected row to output
		if(matrix_row < 12){
			// Port F handling
			RIGHT_MATRIX_OUT_1_DDR |= (1 << (matrix_row - 4)); // set to output
			RIGHT_MATRIX_OUT_1_PORT &= ~(1 << (matrix_row - 4)); // set to low
		} else {
			// Port D handling
			RIGHT_MATRIX_OUT_2_DDR |= (1 << (matrix_row - 12)); // set to output
			RIGHT_MATRIX_OUT_2_PORT &= ~(1 << (matrix_row - 12)); // set to low
		}
	}
#elif (ARCH == ARCH_XMEGA)
	if (matrix_row < MATRIX_ROWS/2) {
		// handling lef hand side of the keyboard
		if (matrix_row == 0) {
			PORTC.OUTCLR = PIN3_bm; // oLoad low
			PORTC.OUTSET = PIN1_bm; // CLK0 high
			PORTC.OUTCLR = PIN1_bm; // CLK0 low
			_delay_us(1);
		}
		PORTC.OUTCLR = PIN3_bm; // oLoad low
		PORTC.OUTSET = PIN3_bm; // oLoad high
		PORTC.OUTSET = PIN1_bm; // CLK0 high
		PORTC.OUTCLR = PIN1_bm; // CLK0 low
	} else {
		// handling right hand side of the keyboard
		uint8_t pin = (1 << (matrix_row - (MATRIX_ROWS/2)));
		// Set all pins to input.
		PORTA.DIRCLR = 0b11111111;
		// And only selected pin to output driven to low.
		PORTA.DIRSET = pin;
		PORTA.OUTCLR = pin;
	}
#else
# error "Unknown architecture."
#endif
	// Used in matrix_read_column() to determine handling of L or R side
	processing_row = matrix_row;
}

uint8_t matrix_read_column(uint8_t matrix_column){
	// KATY: we actually read rows here...
	uint8_t value;
#if (ARCH == ARCH_AVR8)
	if (processing_row < MATRIX_ROWS/2) {
		// handling left hand side
		static uint8_t register_165_state = 0; // parallel to serial state
		if (matrix_column == 0) {
			// read 7th bit of register 165
			register_165_state = (LEFT_I_DATA_PIN & LEFT_I_DATA_MASK) ? 1 : 0;
			// 0, 1, 2 bits of register 165 are not used
			for (int8_t i = 6; i > 2; --i) {
				LEFT_CLK1_HIGH;
				LEFT_CLK1_LOW;
				register_165_state <<= 1;
				register_165_state |=
					(LEFT_I_DATA_PIN & LEFT_I_DATA_MASK) ? 1 : 0;
			}
		}
		value = ((1 << matrix_column) & register_165_state) ? 1 : 0;
	} else {
		// handling right hand side
		switch (matrix_column) {
			case 0:
				value = RIGHT_MATRIX_IN_1_PIN & RIGHT_MATRIX_IN_1_MASK;
				break;
			case 1:
				value = RIGHT_MATRIX_IN_2_PIN & RIGHT_MATRIX_IN_2_MASK;
				break;
			case 2:
				value = RIGHT_MATRIX_IN_3_PIN & RIGHT_MATRIX_IN_3_MASK;
				break;
			case 3:
				value = RIGHT_MATRIX_IN_4_PIN & (1 << 4);
				break;
			case 4:
				value = RIGHT_MATRIX_IN_4_PIN & (1 << 7);
				break;
			default:
				value = 1; // nothing hit (inverse logic)
				break;
		}
	}
#elif (ARCH == ARCH_XMEGA)
	if (processing_row < MATRIX_ROWS/2) {
		// handling left hand side
		static uint8_t register_165_state = 0; // parallel to serial state
		if (matrix_column == 0) {
			// read 7th bit of register 165
			register_165_state = (PORTC.IN & PIN0_bm) ? 1 : 0; // read iData
			// 0, 1, 2 bits of register 165 are not used
			for (int8_t i = 6; i > 2; --i) {
				PORTC.OUTSET = PIN2_bm; // CLK1 high
				PORTC.OUTCLR = PIN2_bm; // CLK1 low
				register_165_state <<= 1;
				register_165_state |= (PORTC.IN & PIN0_bm) ? 1 : 0; // read iData
			}
		}
		value = ((1 << matrix_column) & register_165_state) ? 1 : 0;
	} else {
		// handling right hand side
		value = (PORTD.IN >> matrix_column) & 0x01;
	}
#else
# error "Unknown architecture."
#endif
	return (value == 0); // Key is pressed when pin is low (inverse logic)
}


void set_all_leds(uint8_t led_mask){ set_all_leds_ex(led_mask, -1); }

char const * uitoa(uint16_t x) {
	static char buff[6] = {'\0'};
	if (x == 0) { buff[4]='0'; return buff+4; }
	uint8_t i = 5;
	do {
		buff[--i] = '0' + x % 10;
		x /= 10;
	}while ( x > 0 );
	return buff+i;
}

char const * get_lux_str(int16_t l) {
	static char rv[5];
	if (l < 0) {
		strcpy(rv, "    "); return rv; }
	if ( l <= 9999) {
		char const* raw_rv = uitoa((uint16_t)l);
		uint8_t raw_rv_len = 4 - strlen(raw_rv);
		uint8_t i = 0;
		for (; i < raw_rv_len; ++i) rv[i] = ' ';
		strcpy(rv+i, raw_rv);
	} else {
		char const* raw_rv = uitoa((uint16_t)(l/100));
		rv[0] = raw_rv[0];
		rv[1] = raw_rv[1];
		rv[2] = 'k';
		rv[3] = raw_rv[2];
		rv[4] = '\0';
	}
	return rv;
}

void set_all_leds_ex(uint8_t led_mask, int16_t lux_val){
	static uint8_t prev_led_mask = 0;
	static int16_t prev_lux_val = 0;
	bool no_led_change = led_mask == prev_led_mask || led_mask == LEDMASK_NOP;
	bool no_lux_change = lux_val == prev_lux_val || lux_val < 0;
	if ( no_led_change && no_lux_change ) return;
	if ( no_led_change ) led_mask = prev_led_mask;
	if ( no_lux_change ) lux_val = prev_lux_val;
	prev_led_mask = led_mask; prev_lux_val = lux_val;
	// decode USB status
	if ( led_mask & 0x80 ) {
		switch (led_mask) {
			case LEDMASK_USB_NOTREADY:
				lcd_print_position(0, 0, "Usb Wait"); break;
			case LEDMASK_USB_ENUMERATING:
				lcd_print_position(0, 0, "Usb Enum"); break;
			case LEDMASK_USB_READY:
				lcd_print_position(0, 0, "Usb OK  "); break;
			case LEDMASK_USB_ERROR:
				lcd_print_position(0, 0, "Usb Err "); break;
			default:
				lcd_print_position(0, 0, "BAD Msg!"); break;
		}
		return;
	}
	// decode Layer
	if (led_mask & LED_KEYPAD) {
		lcd_print_position(0, 0, "Keypad  "); lit_blue_led(lux_val); }
	else {
		lcd_print_position(0, 0, "Normal  "); blue_led_off(); }
	// decode Lock LEDs
	char ledMsg[9];
	uint8_t i = 0;
	if (led_mask & LED_CAPS) {
		ledMsg[i++] = 'C'; lit_red_led(lux_val);
	} else red_led_off();
	if (led_mask & LED_NUM) {
		ledMsg[i++] = 'N'; lit_yellow_led(lux_val);
	} else yellow_led_off();
	if (led_mask & LED_SCROLL) {
		ledMsg[i++] = 'S'; lit_green_led(lux_val);
	} else green_led_off();
	while( i<4 ) ledMsg[i++] = ' ';
	// decode remap/macro state
	switch ( led_mask & 0xf0 ) {
		case 0:
			strcpy(&ledMsg[i], get_lux_str(lux_val)); break;
		case LEDMASK_PROGRAMMING_SRC:
			strcpy(&ledMsg[i], "Src?"); break;
		case LEDMASK_PROGRAMMING_DST:
			strcpy(&ledMsg[i], "Dst?"); break;
		case LEDMASK_MACRO_TRIGGER:
			strcpy(&ledMsg[i], "Trg?"); break;
		case LEDMASK_MACRO_RECORD:
			strcpy(&ledMsg[i], "Rec?"); break;
		default:
			strcpy(&ledMsg[i], "ERR!"); break;
	}
	lcd_print_position(1, 0, ledMsg);
}

void test_leds(void){
	for(int8_t i = 0; i < 2; ++i){
		set_all_leds(0); _delay_ms(1000);
		set_all_leds(LEDMASK_USB_NOTREADY); _delay_ms(1000);
		set_all_leds(LEDMASK_USB_ENUMERATING); _delay_ms(1000);
		set_all_leds(LEDMASK_USB_READY); _delay_ms(1000);
		set_all_leds(LEDMASK_USB_ERROR); _delay_ms(1000);
		set_all_leds(LED_CAPS); _delay_ms(1000);
		set_all_leds(LED_NUM); _delay_ms(1000);
		set_all_leds(LED_SCROLL); _delay_ms(1000);
		set_all_leds(LED_KEYPAD); _delay_ms(1000);
		set_all_leds(LEDMASK_PROGRAMMING_SRC); _delay_ms(1000);
		set_all_leds(LEDMASK_PROGRAMMING_DST); _delay_ms(1000);
		set_all_leds(LEDMASK_MACRO_TRIGGER); _delay_ms(1000);
		set_all_leds(LEDMASK_MACRO_RECORD); _delay_ms(1000);
		set_all_leds(0xf0); _delay_ms(1000);
	}
}
