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

#include "config.h"

#include "hardware.h"
#include "usb.h"
#include "Keyboard.h"
#include "printing.h"
#include "keystate.h"
#include "storage.h"
#include "interpreter.h"
#include "buzzer.h"
#include "macro_index.h"
#include "macro.h"
#include "storage.h"

#include <stdlib.h>
#include <util/delay.h>

// Eeprom sentinel value - if this is not set at startup, re-initialize the eeprom.
#define EEPROM_SENTINEL 43
uint8_t eeprom_sentinel_byte STORAGE(MAPPING_STORAGE);

// Persistent configuration (e.g. sound enabled)
configuration_flags eeprom_flags STORAGE(MAPPING_STORAGE);

// Key configuration is stored in eeprom. If the sentinel is not valid, initialize from the defaults.
hid_keycode logical_to_hid_map[NUM_LOGICAL_KEYS] STORAGE(MAPPING_STORAGE);

hid_keycode* config_get_mapping(void){
	return &logical_to_hid_map[0];
}

// We support saving up to 10 keyboard remappings as their differences from the default.
// These (variable sized) mappings are stored in the fixed-size buffer saved_key_mappings,
// indexed by saved_key_mapping_indices. The buffer is kept packed (subsequent mappings
// moved down on removal)
typedef struct _key_mapping_idx { uint8_t start; uint8_t end; } key_mapping_idx;
key_mapping_idx saved_key_mapping_indices[NUM_KEY_MAPPING_INDICES] STORAGE(SAVED_MAPPING_STORAGE);

// Key mappings are saved as a list of (logical_keycode, hid_keycode)
// pairs. Indexed by uint8_t, so must be <= 256 long.
typedef struct _key_mapping { logical_keycode l_key; hid_keycode h_key; } key_mapping;
key_mapping saved_key_mappings[SAVED_MAPPING_COUNT] STORAGE(SAVED_MAPPING_STORAGE);

_Static_assert(sizeof(key_mapping_idx)==sizeof(int16_t), "Config key mapping index saving/reading needs code update." );
_Static_assert(sizeof(key_mapping)==sizeof(int16_t), "Config key mapping data saving/reading needs code update." );
#define AS_SHORT(x) (*(uint16_t*)&x)

// Programs are stored in external eeprom.
static uint8_t programs[PROGRAM_SIZE] STORAGE(PROGRAM_STORAGE);

typedef struct _program_idx { uint16_t offset; uint16_t len; } program_idx;
static program_idx *const programs_index = (program_idx*) programs;

static uint8_t *const programs_data = programs + (PROGRAM_COUNT * sizeof(program_idx));

uint8_t* config_get_programs(){
	return &programs[0];
}

hid_keycode config_get_definition(logical_keycode l_key){
	return storage_read_byte(MAPPING_STORAGE, &logical_to_hid_map[l_key]);
}

hid_keycode config_get_default_definition(logical_keycode l_key){
	return storage_read_byte(CONSTANT_STORAGE, &logical_to_hid_map_default[l_key]);
}

void config_save_definition(logical_keycode l_key, hid_keycode h_key){
	storage_write_byte(MAPPING_STORAGE, &logical_to_hid_map[l_key], h_key);
}

// reset the current layout to the default layout
void config_reset_defaults(void){
	hid_keycode default_keys[32];
	buzzer_start_f(1000, 100); // Start buzzing at low pitch
	_delay_ms(20); // delay so that the two tones are always heard, even if no writes need be done
	for(size_t i = 0; i < NUM_LOGICAL_KEYS; i += sizeof(default_keys) ){
		size_t bs = (NUM_LOGICAL_KEYS-i);
		if (bs > sizeof(default_keys)) bs = sizeof(default_keys);
		storage_wait_for_last_write_end(MAPPING_STORAGE);
		storage_read(CONSTANT_STORAGE, &logical_to_hid_map_default[i], default_keys, bs);
		storage_write(MAPPING_STORAGE, &logical_to_hid_map[i], default_keys, bs);
		USB_KeepAlive(false);
	}

	buzzer_start_f(200, 80); // finish at high to signify end
}

__attribute__((weak)) macro_def_info const STORAGE(CONSTANT_STORAGE) macro_def_infos[] = {{NO_KEY,{0}}};

// reset the keyboard, including saved layouts
void config_reset_fully(void){
	buzzer_start_f(2000, 120); // start buzzing low

	// reset configuration flags
	storage_write_byte(MAPPING_STORAGE, (uint8_t*)&eeprom_flags, 0x0);

	// reset key mapping index
	storage_wait_for_last_write_end(SAVED_MAPPING_STORAGE);
	storage_memset(SAVED_MAPPING_STORAGE, (uint8_t*)saved_key_mapping_indices, NO_KEY, sizeof(saved_key_mapping_indices));
	USB_KeepAlive(true);

	// reset program
	config_reset_program_defaults();

	// and macros
	macro_idx_reset_defaults();
	macros_reset_defaults();

	// now reset the layout and configuration defaults
	config_reset_defaults();

	// Once all reset, update the sentinel
	storage_wait_for_last_write_end(MAPPING_STORAGE);
	storage_write_byte(MAPPING_STORAGE, &eeprom_sentinel_byte, EEPROM_SENTINEL);

	// Higher pitched buzz to signify full reset
	buzzer_start_f(200, 60);
}


configuration_flags config_get_flags(void){
	union {
		uint8_t b;
		configuration_flags s;
	} r;
	r.b = storage_read_byte(MAPPING_STORAGE, (uint8_t*)&eeprom_flags);
	return r.s;
}

void config_save_flags(configuration_flags state){
	union {
		uint8_t b;
		configuration_flags s;
	} r;
	r.s = state;
	storage_write_byte(MAPPING_STORAGE, (uint8_t*)&eeprom_flags, r.b);
}


static const char MSG_NO_LAYOUT[] PROGMEM = "No layout";

bool config_delete_layout(uint8_t num){
	if(num >= NUM_KEY_MAPPING_INDICES){
		printing_set_buffer(MSG_NO_LAYOUT, CONSTANT_STORAGE);
		return false;
	}
	uint8_t start = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[num].start);
	if(start == NO_KEY){
		printing_set_buffer(MSG_NO_LAYOUT, CONSTANT_STORAGE);
		return false;
	}
	uint8_t end = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[num].end); // start and end are inclusive

	uint8_t length = end - start + 1;

	// clear this entry
	storage_write_short(SAVED_MAPPING_STORAGE, (uint16_t*)&saved_key_mapping_indices[num], -1);

	// now scan the other entries, subtracting length from each entry indexed after end
	// update the end position so we can move down only necessary data.
	uint8_t max_end = end;
	for(int i = 0; i < NUM_KEY_MAPPING_INDICES; ++i){
		key_mapping_idx idx;
		storage_wait_for_last_write_end(SAVED_MAPPING_STORAGE);
		idx.start = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[i].start);
		if(idx.start != NO_KEY && idx.start > end){
			idx.end = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[i].end);
			if (idx.end > max_end) max_end = idx.end;
			idx.start -= length; idx.end -= length;
			storage_write_short(SAVED_MAPPING_STORAGE, (uint16_t*)&saved_key_mapping_indices[i], AS_SHORT(idx));
			USB_KeepAlive(false);
		}
	}

	// and move down the data.
	if ( end < max_end ) {
		storage_wait_for_last_write_end(SAVED_MAPPING_STORAGE);
		storage_memmove( SAVED_MAPPING_STORAGE,
			&saved_key_mappings[end+1-length],
			&saved_key_mappings[end+1],
			(max_end-end)*sizeof(key_mapping) );
	}
	storage_wait_for_last_write_end(SAVED_MAPPING_STORAGE);

	return true;
}

bool config_save_layout(uint8_t num){
	if(num >= NUM_KEY_MAPPING_INDICES){
		printing_set_buffer(MSG_NO_LAYOUT, CONSTANT_STORAGE);
		return false;
	}

	// remove old layout
	config_delete_layout(num);

	// find last offset
	int16_t old_end = -1;
	for(int i = 0; i < NUM_KEY_MAPPING_INDICES; ++i){
		uint8_t i_start = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[i].start);
		if(i_start == NO_KEY) continue;
		uint8_t i_end = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[i].end);
		if(i_end > old_end) old_end = i_end;
	}

	uint8_t start = (uint8_t) (old_end + 1);
	uint8_t cursor = start;

	for(logical_keycode l = 0; l < NUM_LOGICAL_KEYS; ++l){
		hid_keycode h = storage_read_byte(MAPPING_STORAGE, &logical_to_hid_map[l]);
		hid_keycode d = storage_read_byte(CONSTANT_STORAGE, &logical_to_hid_map_default[l]);
		if(h != d){
			if(cursor >= SAVED_MAPPING_COUNT - 1){
				printing_set_buffer(CONST_MSG("Fail: no space"), CONSTANT_STORAGE);
				return false; // no space!
			}
			key_mapping m = { .l_key = l, .h_key = h };
			storage_write_short(SAVED_MAPPING_STORAGE, (uint16_t*)&saved_key_mappings[cursor], AS_SHORT(m));
			USB_KeepAlive(false);
			++cursor;
			storage_wait_for_last_write_end(SAVED_MAPPING_STORAGE);
		}
	}
	if(start != cursor){
		key_mapping_idx idx = { .start = start, .end = cursor-1 };
		storage_write_short(SAVED_MAPPING_STORAGE, (uint16_t*)&saved_key_mapping_indices[num], AS_SHORT(idx));
		return true;
	}
	else{
		// same as default layout: nothing to save.
		printing_set_buffer(CONST_MSG("No change"), CONSTANT_STORAGE);
		return false;
	}
}

bool config_load_layout(uint8_t num){
	if(num >= NUM_KEY_MAPPING_INDICES){
		printing_set_buffer(MSG_NO_LAYOUT, CONSTANT_STORAGE);
		return false;
	}

	uint8_t start = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[num].start);
	if(start == NO_KEY){
		printing_set_buffer(MSG_NO_LAYOUT, CONSTANT_STORAGE);
		return false;
	}
	uint8_t end = storage_read_byte(SAVED_MAPPING_STORAGE, &saved_key_mapping_indices[num].end);

	uint8_t offset = start;

	key_mapping m;
	AS_SHORT(m) = storage_read_short(SAVED_MAPPING_STORAGE, (uint16_t*)&saved_key_mappings[offset]);
	++offset;

	for(logical_keycode lkey = 0; lkey < NUM_LOGICAL_KEYS; ++lkey){
		if(lkey != m.l_key){
			// use default
			hid_keycode def_val = storage_read_byte(CONSTANT_STORAGE, &logical_to_hid_map_default[lkey]);
			storage_write_byte(MAPPING_STORAGE, &logical_to_hid_map[lkey], def_val);
		}
		else{
			// use saved
			storage_write_byte(MAPPING_STORAGE, &logical_to_hid_map[lkey], m.h_key);
			if(offset <= end){
				AS_SHORT(m) = storage_read_short(SAVED_MAPPING_STORAGE, (uint16_t*)&saved_key_mappings[offset]);
				++offset;
			}
		}
		USB_KeepAlive(false);
		storage_wait_for_last_write_end(MAPPING_STORAGE);
	}

	return true;
}

const program* config_get_program(uint8_t idx){
	//index range is not checked as this can't be called from user input
	uint16_t program_offset;
	if(-1 == storage_read(PROGRAM_STORAGE,
						  (uint8_t*)&programs_index[idx].offset,
						  (uint8_t*)&program_offset,
						  sizeof(uint16_t))){
		return 0;
	}
	if(program_offset == 0xffff){
		return 0;
	}
	return (const program*) &programs_data[program_offset];
}

void config_reset_program_defaults(){
	// reset program index
	uint8_t sz = PROGRAM_COUNT * sizeof(program_idx);
	storage_wait_for_last_write_end(PROGRAM_STORAGE);
	storage_memset(PROGRAM_STORAGE, (uint8_t*) programs_index, NO_KEY, sz);
}

void config_init(void){
	uint8_t sentinel = storage_read_byte(MAPPING_STORAGE, &eeprom_sentinel_byte);
	if(sentinel != EEPROM_SENTINEL){
		config_reset_fully();
	}
}
