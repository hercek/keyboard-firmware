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
#include "hardware.h"
#include "Descriptors.h"
#include "config.h"
#include "buzzer.h"
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

#define DEBOUNCE_LEN 3 // care about last 3 physical reports when debouncing
// keystate bitfields are adressed by the physical keycode
typedef uint8_t bitfield_word_t;
#define BITFIELD_WORD_BITS (8*sizeof(bitfield_word_t))
#define BITFIELD_WORDS ((KEYPAD_LAYER_SIZE+BITFIELD_WORD_BITS-1)/BITFIELD_WORD_BITS)
static uint8_t active_debounce_index; // currently active index into debounce_bitfields
static uint8_t debounce_bitfields[DEBOUNCE_LEN][BITFIELD_WORDS]; // debouncing information
static uint8_t debounced_on_bitfield[BITFIELD_WORDS]; // which keys changed from off->on
static uint8_t debounced_bitfield[BITFIELD_WORDS]; // which keys are in active state

static inline void set_bit(bitfield_word_t* words, uint8_t n) {
	words[n/BITFIELD_WORD_BITS] |= 1 << n%BITFIELD_WORD_BITS; }
static inline bool get_bit(bitfield_word_t* words, uint8_t n) {
	return 0 != (words[n/BITFIELD_WORD_BITS] & 1<<n%BITFIELD_WORD_BITS); }

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
	active_debounce_index = (active_debounce_index+1) % DEBOUNCE_LEN;
	for(uint8_t w = 0; w < BITFIELD_WORDS; ++w)
		debounce_bitfields[active_debounce_index][w] = 0;
	// for each entry in the matrix
	for(uint8_t matrix_row = 0; matrix_row < MATRIX_ROWS; ++matrix_row){
		matrix_select_row(matrix_row);
		for(uint8_t matrix_col = 0; matrix_col < MATRIX_COLS; ++matrix_col){
			// look up the physical key for the matrix code
			// Note that only one matrix position should map to any given
			// physical code: otherwise we won't register a keypress unless both
			// are pressed: one position will be debouncing up and the other down.
			logical_keycode const p_key = storage_read_byte( CONSTANT_STORAGE,
				&matrix_to_logical_map[matrix_row][matrix_col] );
			if (p_key != NO_KEY) { // this position in the matrix is used
				if (matrix_read_column(matrix_col))
					set_bit(debounce_bitfields[active_debounce_index], p_key);
				// TODO: add bunch of NOPs to the else branch which take as much
				// time as set_bit() to avoid timing attacks on matrix scanning
			}
		}// forall cols
	}// forall rows
	// debounce the matrix readings
	for(uint8_t w = 0; w < BITFIELD_WORDS; ++w){
		bitfield_word_t all_time_active = ~0;
		bitfield_word_t any_time_active = 0;
		for(uint8_t i = 0; i < DEBOUNCE_LEN; ++i) {
			all_time_active &= debounce_bitfields[i][w];
			any_time_active |= debounce_bitfields[i][w];
		}
		debounced_on_bitfield[w] = ~debounced_bitfield[w] & all_time_active;
		debounced_bitfield[w] = (debounced_bitfield[w] | all_time_active) & any_time_active;
	}
	// first mark for removal any keys from keystate which were released
	for(uint8_t j = 0; j < KEYSTATE_COUNT; ++j){
		key_state* const key = &key_states[j];
		if (key->p_key == NO_KEY) continue;
		if (!get_bit(debounced_bitfield, key->p_key))
			key->state = 0;
	}
	// then add any keys from debounced_on_bitfield (the new keypresses)
	uint8_t free_slot = 0;
	for(uint8_t w = 0; w < BITFIELD_WORDS; ++w){
		bitfield_word_t const debounced_on_word = debounced_on_bitfield[w];
		if (0 == debounced_on_word) continue;
		for(uint8_t b = 0; b < BITFIELD_WORD_BITS; ++b){
			if (!(debounced_on_word & 1<<b)) continue;
			logical_keycode const p_key = w*BITFIELD_WORD_BITS + b;
			// p_key just debounced up -> record it to the nearest free slot
			while(free_slot < KEYSTATE_COUNT && key_states[free_slot].p_key != NO_KEY)
				++free_slot;
			if (free_slot >= KEYSTATE_COUNT) goto no_free_slot_to_record_a_new_key;
			key_states[free_slot].p_key = p_key;
			key_states[free_slot].state = 1;
		}// forall bits in one bitfield word
	}// forall bitfield words
	no_free_slot_to_record_a_new_key:
	// the new physical key state is recorded now -> check for layer changes first
	for(uint8_t j = 0; j < KEYSTATE_COUNT; ++j){
		key_state const* const key = &key_states[j];
		if (key->p_key == NO_KEY) continue; // skip empty slot
		hid_keycode const h_key = config_get_definition(key->p_key);
		if (SPECIAL_HID_KEY_NOREMAP(h_key) && key->state != key->prev_state)
			update_layer(h_key, key->state);
	}
	// the new active layer is decoded now -> send notifications about logical key changes
	bool const layer_change = keystate_get_layer_id() != keystate_get_prev_layer_id();
	for(uint8_t j = 0; j < KEYSTATE_COUNT; ++j){
		key_state* const key = &key_states[j];
		if (key->p_key == NO_KEY) continue; // skip empty slot
		if (!layer_change && key->prev_state == key->state) continue; // skip slots without changes
		hid_keycode const h_key = config_get_definition(key->p_key);
		if (is_hid_key_to_notify_about(h_key))
			notify_about_key_change(layer_change, key);
		if (key->prev_state && !key->state)
			*key = empty_key_state;
		else
			key->prev_state = key->state;
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

static inline void adjust_speed_and_report(bool moving, int8_t* speed, int8_t* reportVal) {
	if (moving) {
		*speed += *reportVal;
		*reportVal = *speed;
	} else *speed = 0;
}

void keystate_Fill_MouseReport(MouseReport_Data_t* MouseReport){
	static uint8_t prev_button = 0;
	static uint8_t cnt_move = 0;
	static int8_t speed_x_move = 0;
	static int8_t speed_y_move = 0;
	static uint8_t cnt_wheel = 0;
	static int8_t speed_x_wheel = 0;
	static int8_t speed_y_wheel = 0;

	// rate limit mouse move reports
	cnt_move = (cnt_move+1) % 25;
	if (cnt_move != 1) {
		MouseReport->Button = prev_button; return; }

	bool moving_x = false;
	bool moving_y = false;
	bool wheeling_x = false;
	bool wheeling_y = false;

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
					moving_y = true;
					MouseReport->Y += -1;
					break;
				case SPECIAL_HID_KEY_MOUSE_BACK:
					moving_y = true;
					MouseReport->Y += 1;
					break;
				case SPECIAL_HID_KEY_MOUSE_LEFT:
					moving_x = true;
					MouseReport->X += -1;
					break;
				case SPECIAL_HID_KEY_MOUSE_RIGHT:
					moving_x = true;
					MouseReport->X += 1;
					break;

				case SPECIAL_HID_KEY_WHEEL_FWD:
					wheeling_y = true;
					MouseReport->VWheel += 1;
					break;
				case SPECIAL_HID_KEY_WHEEL_BACK:
					wheeling_y = true;
					MouseReport->VWheel += -1;
					break;
				case SPECIAL_HID_KEY_WHEEL_LEFT:
					wheeling_x = true;
					MouseReport->HWheel += -1;
					break;
				case SPECIAL_HID_KEY_WHEEL_RIGHT:
					wheeling_x = true;
					MouseReport->HWheel += 1;
					break;
				default:
					break;
				}
			}
		}
	}

	prev_button = MouseReport->Button;

	adjust_speed_and_report(moving_x, &speed_x_move, &MouseReport->X);
	adjust_speed_and_report(moving_y, &speed_y_move, &MouseReport->Y);

	// rate limit mouse wheel reports
	cnt_wheel = (cnt_wheel+1) % 5;
	if (cnt_wheel != 1) {
		MouseReport->VWheel = 0;
		MouseReport->HWheel = 0;
		return; }

	adjust_speed_and_report(wheeling_x, &speed_x_wheel, &MouseReport->HWheel);
	adjust_speed_and_report(wheeling_y, &speed_y_wheel, &MouseReport->VWheel);
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
