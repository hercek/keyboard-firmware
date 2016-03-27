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

#ifndef __MACRO_INDEX_H
#define __MACRO_INDEX_H

#include <stdint.h>


#define MACRO_MAX_KEYS 4

// Public interface

typedef struct _macro_idx_key {
	logical_keycode keys[MACRO_MAX_KEYS];
} macro_idx_key;

typedef enum _macro_idx_entry_type { MACRO, PROGRAM } macro_idx_entry_type;

typedef struct _macro_idx_entry_data {
	macro_idx_entry_type type;
	uint16_t data;
} macro_idx_entry_data;

struct _macro_idx_entry;
typedef struct _macro_idx_entry macro_idx_entry;

/**
 * Get a pointer to the underlying data in storage. (To be read/written as a
 * whole by the client application)
 */
uint8_t* macro_idx_get_storage(void);

/**
 * Erases the macro index - to be called from config_reset_fully
 */
void macro_idx_reset_defaults(void);

/**
 * Given a macro_idx_key with the `keys` array populated with `key_count`
 * keycodes, format it into a lookup key and return true if it's valid.
 */
bool macro_idx_format_key(macro_idx_key* key, uint8_t key_count);

/**
 * Looks up a key combination in the macro index.  Returns opaque
 * macro_idx_entry handle if found, or NULL if not.
 */
macro_idx_entry* macro_idx_lookup(macro_idx_key* key);

/**
 * Returns a discriminated union of the contents of a found macro index entry,
 * Either a macro offset, or an integer program id
 */
macro_idx_entry_data macro_idx_get_data(macro_idx_entry* mh);

/**
 * Sets the data content of a macro index entry
 */
void macro_idx_set_data(macro_idx_entry* mh, macro_idx_entry_data data);

/**
 * Removes an entry from the macro index
 */
void macro_idx_remove(macro_idx_entry* mh);

/**
 * Adds an entry for the given key to the index, and returns a pointer
 * to the new (empty) entry, or NULL if full or error.
 */
macro_idx_entry* macro_idx_create(macro_idx_key* key);

typedef void(*macro_idx_iterator)(macro_idx_entry*, void*);

/**
 * Evaluate an iterator function for each defined macro index entry.
 */
void macro_idx_iterate(macro_idx_iterator itr, void* c);

/**
 * Storage representation for macro index entries
 */
struct _macro_idx_entry {
	// unused keycodes are NO_KEY (so an empty entry starts with NO_KEY)
	hid_keycode keys[MACRO_MAX_KEYS];
	uint16_t val; // high bit indicates whether the entry is a macro
				  // or a program. If macro, remaining bits are the
				  // offset to the macro_data in macros array
				  // (obtained via macros_get_macro_offset). If
				  // program, then they are the integer program id.
};

/**
 * Size in bytes of macro index
 */
#define MACRO_INDEX_SIZE (MACRO_INDEX_COUNT * sizeof(macro_idx_entry))


#endif // __MACRO_INDEX_H
