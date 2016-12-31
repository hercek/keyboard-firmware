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

#ifndef __SPI_EEPROM_H
#define __SPI_EEPROM_H

#include "hardware.h"
#include "storage.h"


#define SPIMEM __attribute__((section(".spieeprom")))

#if (ARCH == ARCH_AVR8)
#define SPI_MEM_PAGE_SIZE 32
#elif (ARCH == ARCH_XMEGA)
#define SPI_MEM_PAGE_SIZE 64
#else
#error "Unknwon architecture."
#endif

#define STORAGE_SECTION_spi_eeprom SPIMEM

/**
 * Writes count bytes to serial eeprom address dst, potentially using
 * multiple page writes. Returns number of bytes written if any bytes
 * were successfully written, otherwise -1. A return value of less
 * than count indicates that an error occurred and spi_eeprom_errno
 * is set to indicate the error.
 */
int16_t spi_eeprom_write(void* dst, const void* data, size_t count);

/**
 * Writes the argument byte to serial eeprom address dst
 */
storage_err spi_eeprom_write_byte(uint8_t* dst, uint8_t b);

/**
 * Writes the argument short to serial eeprom address dst
 */
storage_err spi_eeprom_write_short(uint16_t* dst, uint16_t b);

/**
 * If there is a chance that next read or write could happen sooner than 6 ms
 * after the last write then this must be called to ensure the last write is
 * finished before attempting any new spi memory read or write.
 */
void spi_eeprom_wait_for_last_write_end(void);

size_t spi_eeprom_read(const void* addr, void* buf, size_t len);

uint8_t spi_eeprom_read_byte(const uint8_t* addr);

uint16_t spi_eeprom_read_short(const uint16_t* addr);

storage_err spi_eeprom_memmove(void* dst, const void* src, size_t count);

storage_err spi_eeprom_memset(void* dst, uint8_t c, size_t len);


// Do not use! These two calls are only for LUFA spi eeprom stream implementation.
//! Allows to start an unaligned page write.
void spi_eeprom_checked_start_write(void* addr);
/** Write page aligned data to eeprom.
 * D̲s̲t̲ must be aligned to page start or spi_eeprom_checked_start_write must be called first!
 * Oherwise page write cycle is not started (the address is not set). Page aligned writes
 * are finished when the data fill the page or l̲a̲s̲t̲ is true.
 */
storage_err spi_eeprom_write_step(void* dst, const void* data, uint8_t len, uint8_t last);


// test code (not normally linked)
uint8_t spi_eeprom_test_read(void);
uint8_t spi_eeprom_test_write(void);

#endif // __SPI_EEPROM_H
