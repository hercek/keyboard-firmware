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
#include "k84cs.h"
#include "../buzzer.h"
#include "../Lcd.h"
#include "../printing.h"
//#include "twi.h"

#define KEY_NONE NO_KEY
// Because the matrix may not be tightly packed, we want a map from matrix
// position to logical key.

//#define MATRIX_COLS 6  // 6 rows on each side, right side direct, left side via shift register)
//#define MATRIX_ROWS 14 // 7 cols on each side
const logical_keycode matrix_to_logical_map[MATRIX_ROWS][MATRIX_COLS] PROGMEM =
	// ROW 1              ROW 2               ROW 3               ROW 4               ROW 5             ROW 6
	// Left Hand Side
	// iData0 [in]        iData1 [in]        iData2 [in]         iData3 [in]         iData4 [in]        iData5 [in]
	{ {LOGICAL_KEY_PRINT, LOGICAL_KEY_CAPS,  LOGICAL_KEY_VOL,    LOGICAL_KEY_L_CTRL, LOGICAL_KEY_L_SH,  LOGICAL_KEY_L_ALT  } // COL 1 | oLoad6 [out]
	, {LOGICAL_KEY_5,     LOGICAL_KEY_T,     LOGICAL_KEY_G,      LOGICAL_KEY_B,      LOGICAL_KEY_BckSp, LOGICAL_KEY_HOME   } // COL 2 | oLoad5 [out]
	, {LOGICAL_KEY_4,     LOGICAL_KEY_R,     LOGICAL_KEY_F,      LOGICAL_KEY_V,      LOGICAL_KEY_RArr,  LOGICAL_KEY_END    } // COL 3 | oLoad4 [out]
	, {LOGICAL_KEY_3,     LOGICAL_KEY_E,     LOGICAL_KEY_D,      LOGICAL_KEY_C,      LOGICAL_KEY_LArr,  LOGICAL_KEY_L_WIN  } // COL 4 | oLoad3 [out]
	, {LOGICAL_KEY_2,     LOGICAL_KEY_W,     LOGICAL_KEY_S,      LOGICAL_KEY_X,      LOGICAL_KEY_IntK,  LOGICAL_KEY_L_KpSh } // COL 5 | oLoad2 [out]
	, {LOGICAL_KEY_1,     LOGICAL_KEY_Q,     LOGICAL_KEY_A,      LOGICAL_KEY_Z,      LOGICAL_KEY_TILDE, LOGICAL_KEY_L_MACRO} // COL 6 | oLoad1 [out]
	, {LOGICAL_KEY_EQ,    LOGICAL_KEY_TAB,   LOGICAL_KEY_ESC,    LOGICAL_KEY_DEL,    LOGICAL_KEY_PRG,   LOGICAL_KEY_L_PALM } // COL 7 | oLoad0 [out]
	// Right Hand Side
	// XD0 [in]            XD1 [in]           XD2 [in]            XD3 [in]            XD4 [in]           XB2 [in]
	, {LOGICAL_KEY_R_ALT,  LOGICAL_KEY_R_SH,  LOGICAL_KEY_R_CTRL, LOGICAL_KEY_INSERT, LOGICAL_KEY_MENU,  LOGICAL_KEY_BREAK} // COL 8  | XA0 [out]
	, {LOGICAL_KEY_PGUP,   LOGICAL_KEY_SPACE, LOGICAL_KEY_N,      LOGICAL_KEY_H,      LOGICAL_KEY_Y,     LOGICAL_KEY_6    } // COL 9  | XA1 [out]
	, {LOGICAL_KEY_PGDN,   LOGICAL_KEY_DnArr, LOGICAL_KEY_M,      LOGICAL_KEY_J,      LOGICAL_KEY_U,     LOGICAL_KEY_7    } // COL 10 | XA2 [out]
	, {LOGICAL_KEY_R_WIN,  LOGICAL_KEY_UpArr, LOGICAL_KEY_COMMA,  LOGICAL_KEY_K,      LOGICAL_KEY_I,     LOGICAL_KEY_8    } // COL 11 | XA3 [out]
	, {LOGICAL_KEY_R_KpSh, LOGICAL_KEY_OpBrc, LOGICAL_KEY_PERIOD, LOGICAL_KEY_L,      LOGICAL_KEY_O,     LOGICAL_KEY_9    } // COL 12 | XA4 [out]
	, {LOGICAL_KEY_R_MACRO,LOGICAL_KEY_ClBrc, LOGICAL_KEY_SLASH,  LOGICAL_KEY_SEMICOL,LOGICAL_KEY_P,     LOGICAL_KEY_0    } // COL 13 | XA5 [out]
	, {LOGICAL_KEY_R_PALM, LOGICAL_KEY_LaLck, LOGICAL_KEY_ENTER,  LOGICAL_KEY_QUOT,   LOGICAL_KEY_BSLASH,LOGICAL_KEY_MINUS} // COL 14 | XA6 [out]
	};
#undef KEY_NONE

const hid_keycode logical_to_hid_map_default[NUM_LOGICAL_KEYS] PROGMEM = {
	// normal layer
	SPECIAL_HID_KEY_PROGRAM,                             // LOGICAL_KEY_PRG
	SPECIAL_HID_KEY_LAYER_LOCK,                          // LOGICAL_KEY_LaLck
	SPECIAL_HID_KEY_KEYPAD_SHIFT,                        // LOGICAL_KEY_L_KpSh
	SPECIAL_HID_KEY_KEYPAD_SHIFT,                        // LOGICAL_KEY_R_KpSh
	SPECIAL_HID_KEY_MACRO_SHIFT,                         // LOGICAL_KEY_L_MACRO
	SPECIAL_HID_KEY_MACRO_SHIFT,                         // LOGICAL_KEY_R_MACRO
	SPECIAL_HID_KEY_FUNCTION_SHIFT,                      // LOGICAL_KEY_L_PALM
	SPECIAL_HID_KEY_FUNCTION_SHIFT,                      // LOGICAL_KEY_R_PALM
	//---------------------------------------------- on-the-fly remapable
	HID_KEYBOARD_SC_A,                                   // LOGICAL_KEY_A
	HID_KEYBOARD_SC_B,                                   // LOGICAL_KEY_B
	HID_KEYBOARD_SC_C,                                   // LOGICAL_KEY_C
	HID_KEYBOARD_SC_D,                                   // LOGICAL_KEY_D
	HID_KEYBOARD_SC_E,                                   // LOGICAL_KEY_E
	HID_KEYBOARD_SC_F,                                   // LOGICAL_KEY_F
	HID_KEYBOARD_SC_G,                                   // LOGICAL_KEY_G
	HID_KEYBOARD_SC_H,                                   // LOGICAL_KEY_H
	HID_KEYBOARD_SC_I,                                   // LOGICAL_KEY_I
	HID_KEYBOARD_SC_J,                                   // LOGICAL_KEY_J
	HID_KEYBOARD_SC_K,                                   // LOGICAL_KEY_K
	HID_KEYBOARD_SC_L,                                   // LOGICAL_KEY_L
	HID_KEYBOARD_SC_M,                                   // LOGICAL_KEY_M
	HID_KEYBOARD_SC_N,                                   // LOGICAL_KEY_N
	HID_KEYBOARD_SC_O,                                   // LOGICAL_KEY_O
	HID_KEYBOARD_SC_P,                                   // LOGICAL_KEY_P
	HID_KEYBOARD_SC_Q,                                   // LOGICAL_KEY_Q
	HID_KEYBOARD_SC_R,                                   // LOGICAL_KEY_R
	HID_KEYBOARD_SC_S,                                   // LOGICAL_KEY_S
	HID_KEYBOARD_SC_T,                                   // LOGICAL_KEY_T
	HID_KEYBOARD_SC_U,                                   // LOGICAL_KEY_U
	HID_KEYBOARD_SC_V,                                   // LOGICAL_KEY_V
	HID_KEYBOARD_SC_W,                                   // LOGICAL_KEY_W
	HID_KEYBOARD_SC_X,                                   // LOGICAL_KEY_X
	HID_KEYBOARD_SC_Y,                                   // LOGICAL_KEY_Y
	HID_KEYBOARD_SC_Z,                                   // LOGICAL_KEY_Z
	HID_KEYBOARD_SC_1_AND_EXCLAMATION,                   // LOGICAL_KEY_1
	HID_KEYBOARD_SC_2_AND_AT,                            // LOGICAL_KEY_2
	HID_KEYBOARD_SC_3_AND_HASHMARK,                      // LOGICAL_KEY_3
	HID_KEYBOARD_SC_4_AND_DOLLAR,                        // LOGICAL_KEY_4
	HID_KEYBOARD_SC_5_AND_PERCENTAGE,                    // LOGICAL_KEY_5
	HID_KEYBOARD_SC_6_AND_CARET,                         // LOGICAL_KEY_6
	HID_KEYBOARD_SC_7_AND_AMPERSAND,                     // LOGICAL_KEY_7
	HID_KEYBOARD_SC_8_AND_ASTERISK,                      // LOGICAL_KEY_8
	HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,           // LOGICAL_KEY_9
	HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,           // LOGICAL_KEY_0
	HID_KEYBOARD_SC_SEMICOLON_AND_COLON,                 // LOGICAL_KEY_SEMICOL
	HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,            // LOGICAL_KEY_COMMA
	HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,           // LOGICAL_KEY_PERIOD
	HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,             // LOGICAL_KEY_SLASH
	// Left hand extra key well keys
	HID_KEYBOARD_SC_EQUAL_AND_PLUS,                      // LOGICAL_KEY_EQ
	HID_KEYBOARD_SC_TAB,                                 // LOGICAL_KEY_TAB
	HID_KEYBOARD_SC_ESCAPE,                              // LOGICAL_KEY_ESC
	HID_KEYBOARD_SC_DELETE,                              // LOGICAL_KEY_DEL
	HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,              // LOGICAL_KEY_TILDE
	HID_KEYBOARD_SC_NON_US_BACKSLASH_AND_PIPE,           // LOGICAL_KEY_IntK
	HID_KEYBOARD_SC_LEFT_ARROW,                          // LOGICAL_KEY_LArr
	HID_KEYBOARD_SC_RIGHT_ARROW,                         // LOGICAL_KEY_RArr
	HID_KEYBOARD_SC_VOLUME_DOWN,                         // LOGICAL_KEY_VOL
	HID_KEYBOARD_SC_CAPS_LOCK,                           // LOGICAL_KEY_CAPS
	HID_KEYBOARD_SC_PRINT_SCREEN,                        // LOGICAL_KEY_PRINT
	// Right hand extra key well keys
	HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,                // LOGICAL_KEY_MINUS
	HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,                  // LOGICAL_KEY_BSLASH
	HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,                // LOGICAL_KEY_QUOT
	HID_KEYBOARD_SC_ENTER,                               // LOGICAL_KEY_ENTER
	HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE,   // LOGICAL_KEY_ClBrc
	HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE,   // LOGICAL_KEY_OpBrc
	HID_KEYBOARD_SC_UP_ARROW,                            // LOGICAL_KEY_UpArr
	HID_KEYBOARD_SC_DOWN_ARROW,                          // LOGICAL_KEY_DnArr
	HID_KEYBOARD_SC_INSERT,                              // LOGICAL_KEY_INSERT
	0x65/*WinContextMenu (not in LUFA header)*/,         // LOGICAL_KEY_MENU
	HID_KEYBOARD_SC_PAUSE,                               // LOGICAL_KEY_BREAK
	// Left hand thumbpad
	HID_KEYBOARD_SC_LEFT_CONTROL,                        // LOGICAL_KEY_L_CTRL
	HID_KEYBOARD_SC_LEFT_ALT,                            // LOGICAL_KEY_L_ALT
	HID_KEYBOARD_SC_HOME,                                // LOGICAL_KEY_HOME
	HID_KEYBOARD_SC_END,                                 // LOGICAL_KEY_END
	HID_KEYBOARD_SC_LEFT_GUI,                            // LOGICAL_KEY_L_WIN
	HID_KEYBOARD_SC_LEFT_SHIFT,                          // LOGICAL_KEY_L_SH
	HID_KEYBOARD_SC_BACKSPACE,                           // LOGICAL_KEY_BckSp
	// Right hand thumb pad
	HID_KEYBOARD_SC_RIGHT_CONTROL,                       // LOGICAL_KEY_R_CTRL
	HID_KEYBOARD_SC_RIGHT_ALT,                           // LOGICAL_KEY_R_ALT
	HID_KEYBOARD_SC_PAGE_UP,                             // LOGICAL_KEY_PGUP
	HID_KEYBOARD_SC_PAGE_DOWN,                           // LOGICAL_KEY_PGDN
	HID_KEYBOARD_SC_RIGHT_GUI,                           // LOGICAL_KEY_R_WIN
	HID_KEYBOARD_SC_RIGHT_SHIFT,                         // LOGICAL_KEY_R_SH
	HID_KEYBOARD_SC_SPACE,                               // LOGICAL_KEY_SPACE
	/////////////////////////////////////////////////
	// keypad layer                                      // * keypad mode default differs from base
	SPECIAL_HID_KEY_PROGRAM,                             // LOGICAL_KEY_PRG
	SPECIAL_HID_KEY_LAYER_LOCK,                          // LOGICAL_KEY_LaLck
	SPECIAL_HID_KEY_KEYPAD_SHIFT,                        // LOGICAL_KEY_L_KpSh
	SPECIAL_HID_KEY_KEYPAD_SHIFT,                        // LOGICAL_KEY_R_KpSh
	SPECIAL_HID_KEY_MACRO_SHIFT,                         // LOGICAL_KEY_L_MACRO
	SPECIAL_HID_KEY_MACRO_SHIFT,                         // LOGICAL_KEY_R_MACRO
	SPECIAL_HID_KEY_FUNCTION_SHIFT,                      // LOGICAL_KEY_L_PALM
	SPECIAL_HID_KEY_FUNCTION_SHIFT,                      // LOGICAL_KEY_R_PALM
	//---------------------------------------------- on-the-fly remapable
	SPECIAL_HID_KEY_MOUSE_BTN1,                          // LOGICAL_KEY_A *
	HID_KEYBOARD_SC_B,                                   // LOGICAL_KEY_B
	HID_KEYBOARD_SC_C,                                   // LOGICAL_KEY_C
	SPECIAL_HID_KEY_MOUSE_BACK,                          // LOGICAL_KEY_D *
	SPECIAL_HID_KEY_MOUSE_FWD,                           // LOGICAL_KEY_E *
	SPECIAL_HID_KEY_MOUSE_RIGHT,                         // LOGICAL_KEY_F *
	HID_KEYBOARD_SC_G,                                   // LOGICAL_KEY_G
	HID_KEYBOARD_SC_KEYPAD_SLASH,                        // LOGICAL_KEY_H *
	HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW,               // LOGICAL_KEY_I *
	HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW,             // LOGICAL_KEY_J *
	HID_KEYBOARD_SC_KEYPAD_5,                            // LOGICAL_KEY_K *
	HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW,            // LOGICAL_KEY_L *
	HID_KEYBOARD_SC_KEYPAD_1_AND_END,                    // LOGICAL_KEY_M *
	HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT,                 // LOGICAL_KEY_N *
	HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP,                // LOGICAL_KEY_O *
	HID_KEYBOARD_SC_KEYPAD_PLUS,                         // LOGICAL_KEY_P *
	SPECIAL_HID_KEY_MOUSE_BTN2,                          // LOGICAL_KEY_Q *
	SPECIAL_HID_KEY_MOUSE_BTN4,                          // LOGICAL_KEY_R *
	SPECIAL_HID_KEY_MOUSE_LEFT,                          // LOGICAL_KEY_S *
	SPECIAL_HID_KEY_MOUSE_BTN5,                          // LOGICAL_KEY_T *
	HID_KEYBOARD_SC_KEYPAD_7_AND_HOME,                   // LOGICAL_KEY_U *
	HID_KEYBOARD_SC_V,                                   // LOGICAL_KEY_V
	SPECIAL_HID_KEY_MOUSE_BTN3,                          // LOGICAL_KEY_W *
	HID_KEYBOARD_SC_X,                                   // LOGICAL_KEY_X
	HID_KEYBOARD_SC_KEYPAD_ASTERISK,                     // LOGICAL_KEY_Y *
	HID_KEYBOARD_SC_Z,                                   // LOGICAL_KEY_Z
	HID_KEYBOARD_SC_F1,                                  // LOGICAL_KEY_1 *
	HID_KEYBOARD_SC_F2,                                  // LOGICAL_KEY_2 *
	HID_KEYBOARD_SC_F3,                                  // LOGICAL_KEY_3 *
	HID_KEYBOARD_SC_F4,                                  // LOGICAL_KEY_4 *
	HID_KEYBOARD_SC_F5,                                  // LOGICAL_KEY_5 *
	HID_KEYBOARD_SC_F6,                                  // LOGICAL_KEY_6 *
	HID_KEYBOARD_SC_F7,                                  // LOGICAL_KEY_7 *
	HID_KEYBOARD_SC_F8,                                  // LOGICAL_KEY_8 *
	HID_KEYBOARD_SC_F9,                                  // LOGICAL_KEY_9 *
	HID_KEYBOARD_SC_F10,                                 // LOGICAL_KEY_0 *
	HID_KEYBOARD_SC_KEYPAD_MINUS,                        // LOGICAL_KEY_SEMICOL *
	HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW,             // LOGICAL_KEY_COMMA *
	HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN,              // LOGICAL_KEY_PERIOD *
	HID_KEYBOARD_SC_KEYPAD_ENTER,                        // LOGICAL_KEY_SLASH *
	// Left hand extra keys
	HID_KEYBOARD_SC_F11,                                 // LOGICAL_KEY_EQ *
	HID_KEYBOARD_SC_TAB,                                 // LOGICAL_KEY_TAB
	HID_KEYBOARD_SC_ESCAPE,                              // LOGICAL_KEY_ESC
	HID_KEYBOARD_SC_DELETE,                              // LOGICAL_KEY_DEL
	HID_KEYBOARD_SC_MUTE,                                // LOGICAL_KEY_TILDE *
	HID_KEYBOARD_SC_NON_US_BACKSLASH_AND_PIPE,           // LOGICAL_KEY_IntK
	HID_KEYBOARD_SC_LEFT_ARROW,                          // LOGICAL_KEY_LArr
	HID_KEYBOARD_SC_RIGHT_ARROW,                         // LOGICAL_KEY_RArr
	HID_KEYBOARD_SC_VOLUME_UP,                           // LOGICAL_KEY_VOL *
	HID_KEYBOARD_SC_CAPS_LOCK,                           // LOGICAL_KEY_CAPS
	HID_KEYBOARD_SC_SCROLL_LOCK,                         // LOGICAL_KEY_PRINT *
	// Right hand extra keys
	HID_KEYBOARD_SC_F12,                                 // LOGICAL_KEY_MINUS *
	SPECIAL_HID_KEY_MOUSE_BTN1,                          // LOGICAL_KEY_BSLASH *
	SPECIAL_HID_KEY_MOUSE_BTN2,                          // LOGICAL_KEY_QUOT *
	SPECIAL_HID_KEY_MOUSE_BTN3,                          // LOGICAL_KEY_ENTER *
	HID_KEYBOARD_SC_ENTER,                               // LOGICAL_KEY_ClBrc *
	HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE,               // LOGICAL_KEY_OpBrc *
	HID_KEYBOARD_SC_UP_ARROW,                            // LOGICAL_KEY_UpArr
	HID_KEYBOARD_SC_DOWN_ARROW,                          // LOGICAL_KEY_DnArr
	HID_KEYBOARD_SC_INSERT,                              // LOGICAL_KEY_INSERT
	0x65/*WinContextMenu (not in LUFA header)*/,         // LOGICAL_KEY_MENU
	HID_KEYBOARD_SC_NUM_LOCK,                            // LOGICAL_KEY_BREAK *
	// Left hand thumbpad
	HID_KEYBOARD_SC_LEFT_CONTROL,                        // LOGICAL_KEY_L_CTRL
	HID_KEYBOARD_SC_LEFT_ALT,                            // LOGICAL_KEY_L_ALT
	HID_KEYBOARD_SC_HOME,                                // LOGICAL_KEY_HOME
	HID_KEYBOARD_SC_END,                                 // LOGICAL_KEY_END
	HID_KEYBOARD_SC_LEFT_GUI,                            // LOGICAL_KEY_L_WIN
	HID_KEYBOARD_SC_LEFT_SHIFT,                          // LOGICAL_KEY_L_SH
	HID_KEYBOARD_SC_BACKSPACE,                           // LOGICAL_KEY_BckSp
	// Right hand thumbpad
	HID_KEYBOARD_SC_RIGHT_CONTROL,                       // LOGICAL_KEY_R_CTRL
	HID_KEYBOARD_SC_RIGHT_ALT,                           // LOGICAL_KEY_R_ALT
	HID_KEYBOARD_SC_PAGE_UP,                             // LOGICAL_KEY_PGUP
	HID_KEYBOARD_SC_PAGE_DOWN,                           // LOGICAL_KEY_PGDN
	HID_KEYBOARD_SC_RIGHT_GUI,                           // LOGICAL_KEY_R_WIN
	HID_KEYBOARD_SC_RIGHT_SHIFT,                         // LOGICAL_KEY_R_SH
	HID_KEYBOARD_SC_SPACE,                               // LOGICAL_KEY_SPACE
	/////////////////////////////////////////////////
	// function layer                                    // it is the same as the keypad layer (for now)
	SPECIAL_HID_KEY_PROGRAM,                             // LOGICAL_KEY_PRG
	SPECIAL_HID_KEY_LAYER_LOCK,                          // LOGICAL_KEY_LaLck
	SPECIAL_HID_KEY_KEYPAD_SHIFT,                        // LOGICAL_KEY_L_KpSh
	SPECIAL_HID_KEY_KEYPAD_SHIFT,                        // LOGICAL_KEY_R_KpSh
	SPECIAL_HID_KEY_MACRO_SHIFT,                         // LOGICAL_KEY_L_MACRO
	SPECIAL_HID_KEY_MACRO_SHIFT,                         // LOGICAL_KEY_R_MACRO
	SPECIAL_HID_KEY_FUNCTION_SHIFT,                      // LOGICAL_KEY_L_PALM
	SPECIAL_HID_KEY_FUNCTION_SHIFT,                      // LOGICAL_KEY_R_PALM
	//---------------------------------------------- on-the-fly remapable
	SPECIAL_HID_KEY_MOUSE_BTN1,                          // LOGICAL_KEY_A
	HID_KEYBOARD_SC_B,                                   // LOGICAL_KEY_B
	HID_KEYBOARD_SC_C,                                   // LOGICAL_KEY_C
	SPECIAL_HID_KEY_MOUSE_BACK,                          // LOGICAL_KEY_D
	SPECIAL_HID_KEY_MOUSE_FWD,                           // LOGICAL_KEY_E
	SPECIAL_HID_KEY_MOUSE_RIGHT,                         // LOGICAL_KEY_F
	HID_KEYBOARD_SC_G,                                   // LOGICAL_KEY_G
	HID_KEYBOARD_SC_KEYPAD_SLASH,                        // LOGICAL_KEY_H
	HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW,               // LOGICAL_KEY_I
	HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW,             // LOGICAL_KEY_J
	HID_KEYBOARD_SC_KEYPAD_5,                            // LOGICAL_KEY_K
	HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW,            // LOGICAL_KEY_L
	HID_KEYBOARD_SC_KEYPAD_1_AND_END,                    // LOGICAL_KEY_M
	HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT,                 // LOGICAL_KEY_N
	HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP,                // LOGICAL_KEY_O
	HID_KEYBOARD_SC_KEYPAD_PLUS,                         // LOGICAL_KEY_P
	SPECIAL_HID_KEY_MOUSE_BTN2,                          // LOGICAL_KEY_Q
	SPECIAL_HID_KEY_MOUSE_BTN4,                          // LOGICAL_KEY_R
	SPECIAL_HID_KEY_MOUSE_LEFT,                          // LOGICAL_KEY_S
	SPECIAL_HID_KEY_MOUSE_BTN5,                          // LOGICAL_KEY_T
	HID_KEYBOARD_SC_KEYPAD_7_AND_HOME,                   // LOGICAL_KEY_U
	HID_KEYBOARD_SC_V,                                   // LOGICAL_KEY_V
	SPECIAL_HID_KEY_MOUSE_BTN3,                          // LOGICAL_KEY_W
	HID_KEYBOARD_SC_X,                                   // LOGICAL_KEY_X
	HID_KEYBOARD_SC_KEYPAD_ASTERISK,                     // LOGICAL_KEY_Y
	HID_KEYBOARD_SC_Z,                                   // LOGICAL_KEY_Z
	HID_KEYBOARD_SC_F1,                                  // LOGICAL_KEY_1
	HID_KEYBOARD_SC_F2,                                  // LOGICAL_KEY_2
	HID_KEYBOARD_SC_F3,                                  // LOGICAL_KEY_3
	HID_KEYBOARD_SC_F4,                                  // LOGICAL_KEY_4
	HID_KEYBOARD_SC_F5,                                  // LOGICAL_KEY_5
	HID_KEYBOARD_SC_F6,                                  // LOGICAL_KEY_6
	HID_KEYBOARD_SC_F7,                                  // LOGICAL_KEY_7
	HID_KEYBOARD_SC_F8,                                  // LOGICAL_KEY_8
	HID_KEYBOARD_SC_F9,                                  // LOGICAL_KEY_9
	HID_KEYBOARD_SC_F10,                                 // LOGICAL_KEY_0
	HID_KEYBOARD_SC_KEYPAD_MINUS,                        // LOGICAL_KEY_SEMICOL
	HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW,             // LOGICAL_KEY_COMMA
	HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN,              // LOGICAL_KEY_PERIOD
	HID_KEYBOARD_SC_KEYPAD_ENTER,                        // LOGICAL_KEY_SLASH
	// Left hand extra keys
	HID_KEYBOARD_SC_F11,                                 // LOGICAL_KEY_EQ
	HID_KEYBOARD_SC_TAB,                                 // LOGICAL_KEY_TAB
	HID_KEYBOARD_SC_ESCAPE,                              // LOGICAL_KEY_ESC
	HID_KEYBOARD_SC_DELETE,                              // LOGICAL_KEY_DEL
	HID_KEYBOARD_SC_MUTE,                                // LOGICAL_KEY_TILDE
	HID_KEYBOARD_SC_NON_US_BACKSLASH_AND_PIPE,           // LOGICAL_KEY_IntK
	HID_KEYBOARD_SC_LEFT_ARROW,                          // LOGICAL_KEY_LArr
	HID_KEYBOARD_SC_RIGHT_ARROW,                         // LOGICAL_KEY_RArr
	HID_KEYBOARD_SC_VOLUME_UP,                           // LOGICAL_KEY_VOL
	HID_KEYBOARD_SC_CAPS_LOCK,                           // LOGICAL_KEY_CAPS
	HID_KEYBOARD_SC_SCROLL_LOCK,                         // LOGICAL_KEY_PRINT
	// Right hand extra keys
	HID_KEYBOARD_SC_F12,                                 // LOGICAL_KEY_MINUS
	SPECIAL_HID_KEY_MOUSE_BTN1,                          // LOGICAL_KEY_BSLASH
	SPECIAL_HID_KEY_MOUSE_BTN2,                          // LOGICAL_KEY_QUOT
	SPECIAL_HID_KEY_MOUSE_BTN3,                          // LOGICAL_KEY_ENTER
	HID_KEYBOARD_SC_ENTER,                               // LOGICAL_KEY_ClBrc
	HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE,               // LOGICAL_KEY_OpBrc
	HID_KEYBOARD_SC_UP_ARROW,                            // LOGICAL_KEY_UpArr
	HID_KEYBOARD_SC_DOWN_ARROW,                          // LOGICAL_KEY_DnArr
	HID_KEYBOARD_SC_INSERT,                              // LOGICAL_KEY_INSERT
	0x65/*WinContextMenu (not in LUFA header)*/,         // LOGICAL_KEY_MENU
	HID_KEYBOARD_SC_NUM_LOCK,                            // LOGICAL_KEY_BREAK
	// Left hand thumbpad
	HID_KEYBOARD_SC_LEFT_CONTROL,                        // LOGICAL_KEY_L_CTRL
	HID_KEYBOARD_SC_LEFT_ALT,                            // LOGICAL_KEY_L_ALT
	HID_KEYBOARD_SC_HOME,                                // LOGICAL_KEY_HOME
	HID_KEYBOARD_SC_END,                                 // LOGICAL_KEY_END
	HID_KEYBOARD_SC_LEFT_GUI,                            // LOGICAL_KEY_L_WIN
	HID_KEYBOARD_SC_LEFT_SHIFT,                          // LOGICAL_KEY_L_SH
	HID_KEYBOARD_SC_BACKSPACE,                           // LOGICAL_KEY_BckSp
	// Right hand thumbpad
	HID_KEYBOARD_SC_RIGHT_CONTROL,                       // LOGICAL_KEY_R_CTRL
	HID_KEYBOARD_SC_RIGHT_ALT,                           // LOGICAL_KEY_R_ALT
	HID_KEYBOARD_SC_PAGE_UP,                             // LOGICAL_KEY_PGUP
	HID_KEYBOARD_SC_PAGE_DOWN,                           // LOGICAL_KEY_PGDN
	HID_KEYBOARD_SC_RIGHT_GUI,                           // LOGICAL_KEY_R_WIN
	HID_KEYBOARD_SC_RIGHT_SHIFT,                         // LOGICAL_KEY_R_SH
	HID_KEYBOARD_SC_SPACE,                               // LOGICAL_KEY_SPACE
};


// SPI EEPROM related stuf:
#define SPI_DD_SS    PIN1_bp
#define SPI_DD_SCK   PIN7_bp
#define SPI_DD_MOSI  PIN5_bp
#define SPI_DD_MISO  PIN6_bp
#define SPI_DDR_SS   PORTB_DIR
#define SPI_DDR_SCK  PORTC_DIR
#define SPI_DDR_MOSI PORTC_DIR
#define SPI_DDR_MISO PORTC_DIR


static void serial_eeprom_init(void) {
	// Initialize Pins.
	SPI_DDR_SS |= _BV(SPI_DD_SS);      //OUTPUT
	SPI_DDR_SCK |= _BV(SPI_DD_SCK);    //OUTPUT
	SPI_DDR_MOSI |= _BV(SPI_DD_MOSI);  //OUTPUT
	SPI_DDR_MISO &= ~_BV(SPI_DD_MISO); //INPUT
	SPI_PORT_SS.OUTSET = SPI_BIT_SS_bm; // Keep slave inactive (set EEPROM CS to high).
	// Initialize SPI subsystem: master, 8 MHz (DIV4_gc)
	SPIC_CTRL = SPI_ENABLE_bm \
	          | SPI_MASTER_bm \
	          | SPI_CLK2X_bm \
	          | SPI_PRESCALER_DIV4_gc \
	          | SPI_MODE_0_gc;
	// check writing is enabled into to whole EEPROM
	spi_eeprom_enable_write_everywhere();
}


static uint8_t read_calibration_byte(uint8_t index) {
	uint8_t result;
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result = pgm_read_byte(index);
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return result;
}

static void photosensor_init(void) {
	// Set up ADC for photoSns
	PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc; // disable digital input buffer on photoSns
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


void set_all_leds_ex(uint8_t led_mask, uint16_t lux_val);

bool run_photosensor(uint32_t cur_time_ms) {
	static uint32_t next_step_time_ms = 500;
	static uint16_t adc_rv[20];
	const uint8_t adc_rv_array_size = sizeof(adc_rv)/sizeof(adc_rv[0]);
	static bool adc_started = false;
	static uint8_t index = 0;

	if (cur_time_ms < next_step_time_ms)
		return false;
	if ( !adc_started ) {
		ADCA.CH0.CTRL |= ADC_CH_START_bm; // start the ADC
		adc_started = true;
		return false;
	}
	if (!(ADCA.CH0.INTFLAGS & ADC_CH0IF_bm))
		return false; // Wait for conversion to be finished.
	adc_started = false;
	adc_rv[index] = ADCA.CH0RES & 0x0FFF; // get the 12bit value
	if (++index < adc_rv_array_size) {
		next_step_time_ms = cur_time_ms + 1;
		return false;
	}
	index = 0;
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
		//uint16_t lux_val = (uint16_t)(adc_average*0.54945055f - 100.0f); // lux estimate from spec
		uint16_t lux_val = adc_average - 182; // use raw value (remove only the ADC zero shift)
		set_all_leds_ex(LEDMASK_NOP, lux_val);
#ifdef KATY_DEBUG
		sprintf(adc_string, "%d %d\t", lux_val, adc_max-adc_min);
		printing_set_buffer(adc_string, sram);
		PORTE.DIRTGL = PIN3_bm;
#endif
	}
	next_step_time_ms = cur_time_ms + 1000; // plan reading of the next data set in 1 second
#ifdef KATY_DEBUG
	return true;
#else
	return false;
#endif
}


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

void ports_init(void){
	// Katy on atxmega uses PD0-PD4 (5 pins) for rows (input) and shares
	// columns with LCD as output, lines LcdD0-LcdD7.

	// First, set up rows.
	PORTD.DIRCLR = 0b00011111;           // PD0-PD4 as input
	PORTCFG.MPCMASK = 0b00011111;        // Set up multi-pin port configuration.
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc; // Set pull-up on input pins.
	PORTB.DIRCLR = PIN2_bm;              // PB2 is input (the 6th bit of row scan)
	PORTB.PIN2CTRL = PORT_OPC_PULLUP_gc; // Set pull-up on input pins.

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

	// Set up LEDs.
	lcd_init();
	lcd_print_position(0, 0, "K A T Y");
	lcd_print_position(1, 0, "Keyboard");

#if USE_BUZZER
	buzzer_init();;
#endif

	// set period of code timing counter TCC0 to maximum
	TCC0.PER = 0xffff; // wrap timer as little as possible

	serial_eeprom_init();
}

void start_2us_timer(void) {
	TCC0.CTRLFSET = TC_CMD_RESTART_gc;
	TCC0.CTRLA = TC_CLKSEL_DIV64_gc;
}
void stop_2us_timer(void) { TCC0.CTRLA = TC_CLKSEL_OFF_gc; }
inline uint16_t get_2us_time(void) { return TCC0.CNT; }

static uint8_t processing_row = 0;

void matrix_select_row(uint8_t matrix_row){
	// KATY: we actually selecting columns here...
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
	// Used in matrix_read_column() to determine handling of L or R side
	processing_row = matrix_row;
}

uint8_t matrix_read_column(uint8_t matrix_column){
	// KATY: we actually read rows here...
	uint8_t value;
	if (processing_row < MATRIX_ROWS/2) {
		// handling left hand side
		static uint8_t register_165_state = 0; // parallel to serial state
		if (matrix_column == 0) {
			// read 7th bit of register 165
			register_165_state = (PORTC.IN & PIN0_bm) ? 1 : 0; // read iData
			// 0, 1 bits of register 165 are not used
			for (int8_t i = 6; i > 1; --i) {
				PORTC.OUTSET = PIN2_bm; // CLK1 high
				PORTC.OUTCLR = PIN2_bm; // CLK1 low
				register_165_state <<= 1;
				register_165_state |= (PORTC.IN & PIN0_bm) ? 1 : 0; // read iData
			}
		}
		value = ((1 << matrix_column) & register_165_state) ? 1 : 0;
	} else {
		// handling right hand side
		if (matrix_column > 4 ) value = (PORTB.IN >> 2) & 0x01;
		else value = (PORTD.IN >> matrix_column) & 0x01;
	}
	// Key is pressed when pin is low (inverse logic)
	if (value == 0 && processing_row >= MATRIX_ROWS/2)
		return 1;
	return (value == 0);
}


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

void set_all_leds_ex(uint8_t led_mask, uint16_t lux_val){
	static uint8_t prev_led_mask = 0;
	static int16_t prev_lux_val = 0;
	bool no_led_change = led_mask == prev_led_mask || led_mask == LEDMASK_NOP;
	bool no_lux_change = lux_val == prev_lux_val || lux_val == 0xFFFF;
	if ( no_led_change && no_lux_change ) return;
	if ( no_led_change ) led_mask = prev_led_mask;
	if ( no_lux_change ) lux_val = prev_lux_val;
	prev_led_mask = led_mask; prev_lux_val = lux_val;
	// decode USB status
	if ( (led_mask & 0xE0) == 0xE0 ) {
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
	else if (led_mask & LED_FUNCTION) {
		lcd_print_position(0, 0, "Function"); lit_blue_led(lux_val); }
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
	while( i<3 ) ledMsg[i++] = ' ';
	uint8_t add_msg = led_mask & 0xE0;
	ledMsg[i++] = (LEDMASK_MACROS_ENABLED == add_msg) ? 'Q' : ' ';
	// decode remap/macro state
	switch ( add_msg ) {
		case 0:
		case LEDMASK_MACROS_ENABLED:
			if (get_2us_time() > 0) lux_val = get_2us_time();
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

void set_all_leds(uint8_t led_mask){ set_all_leds_ex(led_mask, 0xFFFF); }

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
