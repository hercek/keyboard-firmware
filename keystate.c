/*
  Kinesis ergonomic keyboard firmware replacement

  Copyright 2012 Chris Andreae (chris (at) andreae.gen.nz)
  Copyright 2017 Peter Hercek

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

#include "keystate.h"

#include "Keyboard.h"
#include "hardware.h"
#include "config.h"
#include "buzzer.h"
#include "interpreter.h"
#include "storage.h"

#include <stdarg.h>

// State of active keys. Keep track of all pressed or debouncing keys.
static key_state key_states[KEYSTATE_COUNT];

static key_state const empty_key_state =  {NO_KEY,0};

static keystate_change_hook keystate_change_hook_fn;

uint8_t key_press_count;

typedef struct _layer_state_t {
	unsigned char base:1;      // force base/normal level
	unsigned char lock:1;      // is the selected layer locked?
	unsigned char id:6;        // the identifier of the active layer
} layer_t;

layer_t prev_layer, layer;

void keystate_init(void){
	for(uint8_t i = 0 ; i < KEYSTATE_COUNT; ++i)
		key_states[i] = empty_key_state;
}

uint8_t keystate_get_layer_id(void){
	if (layer.base) return 0xff;
	else return layer.id;
}

static inline uint8_t keystate_get_prev_layer_id(void){
	if (prev_layer.base) return 0xff;
	else return prev_layer.id;
}

static inline void default_beep(void){
		uint16_t const periodCnt = 2;
		buzzer_start((periodCnt*1000+(BUZZER_DEFAULT_TONE/2+1)) / BUZZER_DEFAULT_TONE);
}

static inline logical_keycode extract_keycode(layer_t* ll, key_state* kk, keycode_type ktype){
	if (ktype == PHYSICAL) return kk->p_key;
	uint8_t layer_id = ll->id;
	if (ll->base) layer_id = 0;
	logical_keycode l_key  = kk->p_key + layer_id * KEYPAD_LAYER_SIZE;
	if (ktype == LOGICAL) return l_key;
	return config_get_definition(l_key);
}

static void notify_key_pressed(layer_t* ll, key_state* kk){
	++key_press_count;
	if(keystate_change_hook_fn)
		keystate_change_hook_fn( extract_keycode(ll,kk,LOGICAL), true);
	#if USE_BUZZER
	if(config_get_flags().key_sound_enabled)
		default_beep();
	#endif
}

static void notify_key_released(layer_t* ll, key_state* kk){
	--key_press_count;
	if(keystate_change_hook_fn)
		keystate_change_hook_fn(extract_keycode(ll,kk,LOGICAL), false);
}

static inline void notify_about_key_change(bool layer_change, key_state* key) {
	if (layer_change) {
		if (key->prev_state && key->state) {
			notify_key_released(&prev_layer, key);
			notify_key_pressed(&layer, key);
		} else if (key->state)    notify_key_pressed(&layer, key);
		else if (key->prev_state) notify_key_released(&prev_layer, key);
	} else if (key->state)
		notify_key_pressed(&layer, key);
	else
		notify_key_released(&layer, key);
}

// All the layer shift/lock keys are not reported to higher levels than keystate
static inline bool is_hid_key_to_notify_about(hid_keycode k) {
	return k < SPECIAL_HID_KEY_LAYER_LOCK || k >= SPECIAL_HID_KEY_MACRO_SHIFT;
}

// Called when a layer key (either lock or shift) changes state.
static void update_layer(hid_keycode key, uint8_t state){
	switch(key){
	case SPECIAL_HID_KEY_LAYER_LOCK:
		if (state) { // we are active only on key press (key release is ignored)
			if (layer.lock) { layer.lock = 0; layer.id = 0; }
			else if (layer.id == 0) { layer.lock = 1; layer.id = 1; }
			else layer.lock = 1;
		}
		break;
	case SPECIAL_HID_KEY_KEYPAD_SHIFT:
		if(state) layer.id = 1;
		else if (!layer.lock) layer.id = 0;
		break;
	case SPECIAL_HID_KEY_FUNCTION_SHIFT:
		if(state) layer.id = 2;
		else if (!layer.lock) layer.id = 0;
		break;
	case SPECIAL_HID_KEY_MACRO_SHIFT:
	case SPECIAL_HID_KEY_PROGRAM:
		layer.base = (state != 0);
		break;
	}
	#if USE_BUZZER
	uint8_t cur_layer = keystate_get_layer_id();
	if ( cur_layer == keystate_get_prev_layer_id() ) return;
	//if (config_get_flags().key_sound_enabled)
		if (SPECIAL_HID_KEY_LAYER_LOCK==key)
			buzzer_start_f(100, cur_layer ? BUZZER_ON_TONE : BUZZER_OFF_TONE);
		else default_beep();
	#endif
}

void keystate_update(void){
	// for each entry i in the matrix
	for(uint8_t matrix_row = 0; matrix_row < MATRIX_ROWS; ++matrix_row){
		matrix_select_row(matrix_row);
		for(uint8_t matrix_col = 0; matrix_col < MATRIX_COLS; ++matrix_col){
			// look up the physical key for the matrix code
			// Note that only one matrix position should map to any given
			// physical code: otherwise we won't register a keypress unless both
			// are pressed: one position will be debouncing up and the other down.
			logical_keycode p_key = storage_read_byte( CONSTANT_STORAGE,
				&matrix_to_logical_map[matrix_row][matrix_col] );
			if(p_key == NO_KEY) goto nextMatrixPos; // empty space in the sparse matrix
			uint8_t reading = matrix_read_column(matrix_col);
			uint8_t free_slot = 0xff;
			// Scan the current keystates. If we find an entry for our key, update it.
			// If we don't, and the key is pressed, add it to a free slot.
			for(uint8_t j = 0; j < KEYSTATE_COUNT; ++j){
				key_state* key = &key_states[j];
				if(free_slot == 0xff && key->p_key == NO_KEY)
					free_slot = j; // found a free slot
				else if(key->p_key == p_key){ //found our key
					// update the debounce mask with the current reading
					key->debounce = DEBOUNCE_MASK & ((key->debounce << 1) | reading);
					if(key->debounce == 0x00)
						key->state = 0; // key is not pressed (either debounced-down or never made it up)
					else if(key->debounce == DEBOUNCE_MASK)
						key->state = 1; // key is pressed now
					goto nextMatrixPos; // when key was found then go to the next matrix position
				}
			}
			// Key was not in the state, so previously not pressed.
			// If pressed now, record a new key if there's space.
			if(reading && free_slot != 0xff){
				key_state* key = &key_states[free_slot];
				key->p_key = p_key;
				key->debounce = 0x1;
			}
		nextMatrixPos:;
		}
	}
	// the new physical key state is recorded now -> check for layer changes first
	for(uint8_t j = 0; j < KEYSTATE_COUNT; ++j){
		key_state* key = &key_states[j];
		if (key->p_key == NO_KEY) continue; // skip empty slot
		hid_keycode h_key = config_get_definition(key->p_key);
		if (SPECIAL_HID_KEY_NOREMAP(h_key) && key->state != key->prev_state)
			update_layer(h_key, key->state);
	}
	// the new active layer is decoded now -> send notifications about logical key changes
	bool layer_change = keystate_get_layer_id() != keystate_get_prev_layer_id();
	for(uint8_t j = 0; j < KEYSTATE_COUNT; ++j){
		key_state* key = &key_states[j];
		if (key->p_key == NO_KEY) continue; // skip empty slot
		if (!layer_change && key->prev_state == key->state) continue; // skip slots without changes
		hid_keycode h_key = config_get_definition(key->p_key);
		if (is_hid_key_to_notify_about(h_key))
			notify_about_key_change(layer_change, key);
		if (key->prev_state && !key->state) *key = empty_key_state;
		else key->prev_state = key->state;
	}
	prev_layer = layer;
}

void keystate_hide_key(logical_keycode l_key){
	logical_keycode p_key = l_key % KEYPAD_LAYER_SIZE;
	for(int8_t i = 0; i < KEYSTATE_COUNT; ++i)
		if (p_key == key_states[i].p_key) {
			key_states[i].hidden = 1; break; }
}

bool keystate_is_key_hidden(logical_keycode l_key){
	logical_keycode p_key = l_key % KEYPAD_LAYER_SIZE;
	for(int8_t i = 0; i < KEYSTATE_COUNT; ++i)
		if (p_key == key_states[i].p_key)
			return key_states[i].hidden;
	return true;
}

bool keystate_check_key(logical_keycode target_key, keycode_type ktype){
	for(int i = 0; i < KEYSTATE_COUNT; ++i){
		logical_keycode key = extract_keycode(&layer, &key_states[i], ktype);

		if(key == target_key){
			return key_states[i].state;
		}
	}
	return false;
}

/** returns true if all argument keys are down */
bool keystate_check_keys(uint8_t count, keycode_type ktype, ...){
	if(count > key_press_count) return false; // trivially know it's impossible

	va_list argp;
	bool success = true;
	va_start(argp, ktype);
	while(count--){
		logical_keycode target_key = va_arg(argp, int);
		bool found_key = keystate_check_key(target_key, ktype);

		if(!found_key){
			success = false;
			break;
		}
	}

	va_end(argp);
	return success;
}

bool keystate_check_any_key(uint8_t count, keycode_type ktype, ...){
	if(key_press_count == 0) return false;

	va_list argp;
	bool success = false;
	va_start(argp, ktype);
	while(count--){
		logical_keycode target_key = va_arg(argp, int);
		bool found_key = keystate_check_key(target_key, ktype);

		if(found_key){
			success = true;
			break;
		}
	}

	va_end(argp);
	return success;
}

/**
 * writes up to key_press_count currently pressed key indexes to the
 * output buffer keys.
 */
void keystate_get_keys(logical_keycode* keys, keycode_type ktype){
	int ki = 0;
	for(int i = 0; i < KEYSTATE_COUNT && ki < key_press_count; ++i){
		if (!key_states[i].state) continue;
		hid_keycode h_key = extract_keycode(&layer, &key_states[i], HID);
		if(!is_hid_key_to_notify_about(h_key)) continue;
		keys[ki++] = extract_keycode(&layer, &key_states[i], ktype);
	}
}

void keystate_Fill_KeyboardReport(KeyboardReport_Data_t* KeyboardReport){
	uint8_t UsedKeyCodes = 0;
	uint8_t rollover = false;
	// check key state
	for(int i = 0; i < KEYSTATE_COUNT; ++i){
		if(key_states[i].state && !(key_states[i].hidden)){
			if(UsedKeyCodes == KEYBOARDREPORT_KEY_COUNT){
				rollover = true;
				break;
			}
			hid_keycode h_key = extract_keycode(&layer, &key_states[i], HID);

			if(h_key == SPECIAL_HID_KEY_PROGRAM) rollover = true; // Simple way to ensure program key combinations never cause typing

			// check for special and modifier keys
			if(h_key >= SPECIAL_HID_KEYS_START){
				// There's no output for a special key
				continue;
			}
			else if(h_key >= HID_KEYBOARD_SC_LEFT_CONTROL){
				uint8_t shift = h_key - HID_KEYBOARD_SC_LEFT_CONTROL;
				KeyboardReport->Modifier |= (1 << shift);
			}
			else{
				KeyboardReport->KeyCode[UsedKeyCodes++] = h_key;
			}
		}
	}
	if(rollover){
		for(int i = 0; i < KEYBOARDREPORT_KEY_COUNT; ++i)
			KeyboardReport->KeyCode[i] = HID_KEYBOARD_SC_ERROR_ROLLOVER;
	 }
}

void keystate_Fill_MouseReport(MouseReport_Data_t* MouseReport){
	uint8_t const time_inc = 25;
	static uint16_t cur_time = 0;
	static uint16_t next_active_time = 0;
	static uint8_t prev_button = 0;

	// rate limit mouse reports
	if (++cur_time < next_active_time) {
		MouseReport->Button = prev_button; return; }
	next_active_time += time_inc;

	bool moving = false;
	// simple linear acceleration
	uint16_t move_len = cur_time/time_inc;
	if (move_len > 127) move_len = 127;

	// check mouse key states
	for(uint8_t i = 0; i < KEYSTATE_COUNT; ++i){
		if(key_states[i].state && !(key_states[i].hidden)){
			hid_keycode h_key = extract_keycode(&layer, &key_states[i], HID);
			if(h_key >= SPECIAL_HID_KEYS_MOUSE_START && h_key <= SPECIAL_HID_KEYS_MOUSE_END){
				switch(h_key){
				case SPECIAL_HID_KEY_MOUSE_BTN1:
					MouseReport->Button |= 1;
					break;
				case SPECIAL_HID_KEY_MOUSE_BTN2:
					MouseReport->Button |= 1<<1;
					break;
				case SPECIAL_HID_KEY_MOUSE_BTN3:
					MouseReport->Button |= 1<<2;
					break;
				case SPECIAL_HID_KEY_MOUSE_BTN4:
					MouseReport->Button |= 1<<3;
					break;
				case SPECIAL_HID_KEY_MOUSE_BTN5:
					MouseReport->Button |= 1<<4;
					break;

				case SPECIAL_HID_KEY_MOUSE_FWD:
					moving = true;
					MouseReport->Y = -move_len;
					break;
				case SPECIAL_HID_KEY_MOUSE_BACK:
					moving = true;
					MouseReport->Y = move_len;
					break;
				case SPECIAL_HID_KEY_MOUSE_LEFT:
					moving = true;
					MouseReport->X = -move_len;
					break;
				case SPECIAL_HID_KEY_MOUSE_RIGHT:
					moving = true;
					MouseReport->X = move_len;
					break;
				default:
					break;
				}
			}
		}
	}

	// measure key press time for accelration only when moving
	if(!moving) {
		cur_time = 0;
		next_active_time = time_inc;
	}
	prev_button = MouseReport->Button;
}

hid_keycode keystate_check_hid_key(hid_keycode key){
	for(int i = 0; i < KEYSTATE_COUNT; ++i){
		if(key_states[i].state){
			hid_keycode h_key = extract_keycode(&layer, &key_states[i], HID);
			if(key == 0 || key == h_key) return h_key;
		}
	}
	return 0xFF;
}

void keystate_register_change_hook(keystate_change_hook hook){
	keystate_change_hook_fn = hook;
}
