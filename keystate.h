/*
  Kinesis ergonomic keyboard firmware replacement

  Copyright 2012 Chris Andreae (chris (at) andreae.gen.nz)

  Licensed under the GNU GPL v2 (see GPL2.txt).

  See Kinesis.h for keyboard hardware documentation.

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

#ifndef __KEYSTATE_H
#define __KEYSTATE_H

#include "Keyboard.h"

// types

// Keycodes go through four transformations.
// A raw keyboard matrix coordinates map to a index in the (keyboard specific) logical key positions
// table. It's first block (up to KEYPAD_LAYER_SIZE) is the physical keycode. Physical keycode is equal
// to the logical keycode in the normal/base layer. Additional layers (keypad, function, ...) increase
// the physical keycode by a whole multiple of KEYPAD_LAYER_SIZE. This way we get logical keycode
// We then look up a HID keycode for this logical keycode using the key_defaults/key_config tables.
typedef uint8_t logical_keycode; // this is the biggest type (depending on maximum logical keys)
typedef uint8_t hid_keycode; // this is always uint8_t per USB standard

typedef struct _key_state {
	logical_keycode p_key;
	unsigned char prev_state:1;
	unsigned char state:1;
	unsigned char hidden:1;
	unsigned char debounce:6; // bit vector of last n physical reports: debounced state asserted when they're all the same
} key_state;

// constants

#define KEYSTATE_COUNT 14 // maximum keys we track at once

#define DEBOUNCE_MASK 0x07 // care about last 3 physical reports

#define NO_KEY 0xFF

/* Logical keys are mapped to HID codes. We want to be able to assign some extra actions
   that don't correspond to valid HID codes, so we assign some extra codes for our use
   at the end of the HID range, after E7, the last HID key. As these aren't valid keycodes,
   they'll never be sent via USB.
*/
#define SPECIAL_HID_KEYS_START 0xE8

enum SPECIAL_HID_KEYS {
	SPECIAL_HID_KEY_MOUSE_BTN1   = SPECIAL_HID_KEYS_START,
	SPECIAL_HID_KEY_MOUSE_BTN2,
	SPECIAL_HID_KEY_MOUSE_BTN3,
	SPECIAL_HID_KEY_MOUSE_BTN4,
	SPECIAL_HID_KEY_MOUSE_BTN5,
	SPECIAL_HID_KEY_MOUSE_FWD,
	SPECIAL_HID_KEY_MOUSE_BACK,
	SPECIAL_HID_KEY_MOUSE_LEFT,
	SPECIAL_HID_KEY_MOUSE_RIGHT,
	// And the special non-remappable program and keypad keys
	SPECIAL_HID_KEY_LAYER_LOCK = 0xFA,
	SPECIAL_HID_KEY_KEYPAD_SHIFT,
	SPECIAL_HID_KEY_FUNCTION_SHIFT,
	SPECIAL_HID_KEY_MACRO_SHIFT,
	SPECIAL_HID_KEY_PROGRAM    = 0xFE // Must be last: we rely on sort order
	// 0xFF represents NO_KEY
};

#define SPECIAL_HID_KEYS_MOUSE_START SPECIAL_HID_KEY_MOUSE_BTN1
#define SPECIAL_HID_KEYS_MOUSE_END SPECIAL_HID_KEY_MOUSE_RIGHT

// Program, keypad and layer shift keys are special: they can't be remapped using
// onboard remapping since they themselves control the meaning of other keys.
#define SPECIAL_HID_KEY_NOREMAP(k) ((k) >= SPECIAL_HID_KEY_LAYER_LOCK && (k) != NO_KEY)

// fields

extern uint8_t key_press_count;

void keystate_init(void);

void keystate_update(void);

uint8_t keystate_get_layer_id(void);

/**
 * Types of keycode:
 * LOGICAL:  `logical_keycode` corresponding to a given key
 * PHYSICAL: `logical_keycode` in the base/normal layer at the position matching a given key.
 * HID:      `hid_keycode` in the current mapping for a given key
 */
typedef enum _keycode_type { PHYSICAL, LOGICAL, HID } keycode_type;

/**
 * Marks given logical_key as hidden so that it does not appear in
 * the KeyboardReport till it is not released and pressed again. */
void keystate_hide_key(logical_keycode logical_key);
bool keystate_is_key_hidden(logical_keycode logical_key);

/** Checks if the argument key is down. */
bool keystate_check_key(logical_keycode l_key, keycode_type ktype);

/** returns true if all argument keys are down */
bool keystate_check_keys(uint8_t count, keycode_type ktype, ...);

/** returns true if any argument keys are down */
bool keystate_check_any_key(uint8_t count, keycode_type ktype, ...);

/** writes up to key_press_count currently pressed key indexes to the
 * output buffer keys. */
void keystate_get_keys(logical_keycode* keys, keycode_type ktype);

void keystate_Fill_KeyboardReport(KeyboardReport_Data_t* KeyboardReport);

void keystate_Fill_MouseReport(MouseReport_Data_t* MouseReport);

/**
 * Checks whether a key mapped to the argument keycode (or any key if
 * argument 0) is pressed, returns keycode of first pressed key or
 * NO_KEY.
 */
hid_keycode keystate_check_hid_key(hid_keycode key);

/**
 * A keystate change hook function is invoked whenever the logical key
 * state is changed, passing the logical keycode and the type of event.
 */
typedef void (*keystate_change_hook)(logical_keycode key, bool press);

/**
 * Sets the given function as the keystate change hook function. Set
 * null to unregister.
 */
void keystate_register_change_hook(keystate_change_hook hook);

#endif // __KEYSTATE_H
