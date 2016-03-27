/* Copyright (c) 2014 Juraj Hercek et. al.
   Author(s): Juraj Hercek
              Peter Hercek

   Licensed under the GNU GPL v2 (see GPL2.txt).
*/

#include "storage/spi_eeprom.h"
#include "usb.h"
#include "printing.h"

#define SPI_EEPROM_OK 0

/* Expects to have following defined:
   - DDR_MOSI - DDR SPI Master Out, Slave In
   - DDR_MISO - DDR SPI Master In, Slave Out
   - DDR_SCK  - DDR SPI clock
   - DDR_SS   - DDR slave/chip select
*/

#define SPI_STATUS_WRITE_IN_PROGRESS_MASK 1
#define SPI_STATUS_WRITE_DISABLED_MASK 0b01100
typedef enum _SPI_COMMAND {
	WRSR  = 0b0001,
	WRITE = 0b0010,
	READ  = 0b0011,
	WRDI  = 0b0100,
	RDSR  = 0b0101,
	WREN  = 0b0110
} SPI_COMMAND;

static inline void spi_slave_on(void) {
	// SS is active low.
#if (ARCH == ARCH_AVR8)
	SPI_PORT_SS &= ~_BV(SPI_BIT_SS);
#elif (ARCH == ARCH_XMEGA)
	SPI_PORT_SS.OUTCLR = SPI_BIT_SS_bm;
#else
#  error "Unknown architecture."
#endif
	SPI_EEPROM_CS_SETUP_DELAY;
}

static inline void spi_slave_off(void) {
	// SS is inactive high.
#if (ARCH == ARCH_AVR8)
	SPI_PORT_SS |= _BV(SPI_BIT_SS);
#elif (ARCH == ARCH_XMEGA)
	SPI_PORT_SS.OUTSET = SPI_BIT_SS_bm;
#else
#  error "Unknown architecture."
#endif
	SPI_EEPROM_CS_HOLD_DELAY;
}

static inline uint8_t spi_transfer(volatile uint8_t byte_data) {
#if (ARCH == ARCH_AVR8)
	SPDR = byte_data;
	while (!(SPSR & _BV(SPIF)));
	return SPDR;
#elif (ARCH == ARCH_XMEGA)
	SPIC_DATA = byte_data;
	while (!(SPIC_STATUS & SPI_IF_bm));
	return SPIC_DATA;
#else
#   error "Unknown architecture."
#endif
}

// WriteEnableLatch must be set to HIGH before each page write.
// When a page write is fininshed the chip sets WriteEnableLatch to LOW.
// A page write can be in progress internally even after we deactivated SS.
static void spi_enable_write(void) {
	spi_slave_on();
	(void) spi_transfer(WREN);
	spi_slave_off();
}

// Writes one chunk of data that fits the page size.
// This cannot write behind page end. If we try to write behind page end
// then we will instead continue writing from the page start. (The internal
// address counder wraps at page boundaries.)
static int8_t spi_eeprom_continue_write(const uint8_t* buf, uint8_t len) {
	for (uint8_t i = 0; i < len; ++i)
		(void) spi_transfer(buf[i]);
	return len;
}

// Read eeprom status register.
// Usefull to check whether the last write finished internally.
static uint8_t spi_eeprom_read_status(void) {
	spi_slave_on();
	(void) spi_transfer(RDSR);
	uint8_t rv = spi_transfer(0xff);
	spi_slave_off();
	return rv;
}

// Write eeprom status register.
static void spi_eeprom_write_status(uint8_t new_status) {
	spi_slave_on();
	(void) spi_transfer(WRSR);
	(void) spi_transfer(new_status);
	spi_slave_off();
}

// Makes sure we can write to whole EEPROM address range.
void spi_eeprom_enable_write_everywhere(void) {
	uint8_t rv = spi_eeprom_read_status();
	if ( rv & SPI_STATUS_WRITE_DISABLED_MASK )
		spi_eeprom_write_status(0); // enable write to the whole EEPROM
}

// Wait till internal EEPROM write cycle finishes.
// Notice that the internal write cycle can last as much
// as 6 ms after SS from the last write was deactivated.
void spi_eeprom_wait_for_last_write_end(void) {
	for(;;) {
		uint8_t rv = spi_eeprom_read_status();
		if (! ( rv & SPI_STATUS_WRITE_IN_PROGRESS_MASK ) ) break;
		_delay_us(1); // wait a bit and check again
	}
}

// Initializes an eeprom page for a write operation.
// Precondition: * SS is not active.
//               * caller must call this at least 6 ms after the last write or
//                 it must call spi_eeprom_wait_for_last_write_end() before this
storage_err spi_eeprom_start_write(void* addr) {
	spi_enable_write();
	spi_slave_on();
	(void) spi_transfer(WRITE);
	(void) spi_transfer((uint8_t)((uint16_t)(addr) >> 8));//high part of 16bit address
	(void) spi_transfer((uint8_t)((uint16_t)(addr) & 0xFF));//low part of 16bit address
	// at this point, SS is active (LOW), the write is in progress ...
	// ... only the data needs to be sent and then SS deactivated
	return SPI_EEPROM_OK;
}

void spi_eeprom_checked_start_write(void* addr) {
	spi_eeprom_wait_for_last_write_end();
	spi_eeprom_start_write(addr);
}

// Precondition: buffer doesn't cross page boundary
storage_err spi_eeprom_write_step(void* addr, const void* data, uint8_t len, uint8_t last) {
	if ( (((intptr_t)addr) & (SPI_MEM_PAGE_SIZE-1)) == 0 ) {
		// start write at each page start
		storage_err result;
		spi_eeprom_wait_for_last_write_end();
		if ( SPI_EEPROM_OK != (result = spi_eeprom_start_write(addr)) ) return result;
	}
	if (len != spi_eeprom_continue_write(data, len)) return storage_errno;
	if (last || ((((intptr_t)addr+len) & (SPI_MEM_PAGE_SIZE-1)) == 0))
		spi_slave_off(); // enforce page write cycle
	return SPI_EEPROM_OK;
}

// Note: this can read through page boundaries.
// Precondition: caller must call this at least 6 ms after the last write or
//               it must call spi_eeprom_wait_for_last_write_end() before this
size_t spi_eeprom_read(const void* addr, void* buf, size_t len) {
	spi_slave_on();
	(void) spi_transfer(READ);
	(void) spi_transfer((uint8_t)((uint16_t)(addr) >> 8));//high part of 16bit address
	(void) spi_transfer((uint8_t)((uint16_t)(addr) & 0xFF));//low part of 16bit address
	for (int16_t i = 0; i < len; ++i)
		((uint8_t*)buf)[i] = spi_transfer(0xff);
	spi_slave_off();
	return len;
}

uint8_t spi_eeprom_read_byte(const uint8_t* addr){
	uint8_t b;
	spi_eeprom_read(addr, &b, sizeof(uint8_t));
	return b;
}

uint16_t spi_eeprom_read_short(const uint16_t* addr){
	uint16_t s;
	spi_eeprom_read(addr, &s, sizeof(uint16_t));
	return s;
}

// Precondition: caller must call this at least 6 ms after the last write or
//               it must call spi_eeprom_wait_for_last_write_end() before this
int8_t spi_eeprom_write_page(void* addr, const void* buf, uint8_t len) {
	int8_t result;
	if ( SPI_EEPROM_OK != (result = spi_eeprom_start_write(addr)) ) {
		storage_errno = result; result = 0;
	} else
		result = spi_eeprom_continue_write(buf, len);
	spi_slave_off(); // Finish the page write by invoking of write cycle.
	return result;
}

int16_t spi_eeprom_write(void* dst, const void* buf, size_t count) {
	int16_t written = 0;
	while (count) {
		uint8_t dst_page_off = ((uintptr_t)dst) & (SPI_MEM_PAGE_SIZE-1);
		uint8_t dst_page_remaining = SPI_MEM_PAGE_SIZE - dst_page_off;
		uint8_t n = (count < dst_page_remaining) ? (uint8_t)count : dst_page_remaining;
		spi_eeprom_wait_for_last_write_end();
		int8_t w = spi_eeprom_write_page(dst, buf, n);
		written += w;
		if(w != n)
			return (written==0) ? written : -1; // impossible error: incomplete write
		buf += n;
		dst += n;
		count -= n;
		USB_KeepAlive(true);
	}
	return written;
}

storage_err spi_eeprom_memset(void* dst, uint8_t c, size_t len) {
	uint8_t buf[SPI_MEM_PAGE_SIZE];
	memset(buf, c, SPI_MEM_PAGE_SIZE);
	while (len) {
		uint8_t dst_page_off = ((uintptr_t)dst) & (SPI_MEM_PAGE_SIZE-1);
		uint8_t dst_page_remaining = SPI_MEM_PAGE_SIZE - dst_page_off;
		uint8_t n = (len < dst_page_remaining) ? (uint8_t)len : dst_page_remaining;
		spi_eeprom_wait_for_last_write_end();
		spi_eeprom_write_page(dst, buf, n);
		dst += n;
		len -= n;
		USB_KeepAlive(true);
	}
	return SPI_EEPROM_OK;
}

storage_err spi_eeprom_write_byte(uint8_t* dst, uint8_t b) {
	spi_eeprom_wait_for_last_write_end();
	spi_eeprom_write_page(dst, &b, 1);
	return SPI_EEPROM_OK;
}

storage_err spi_eeprom_write_short(uint16_t* dst, uint16_t s) {
	if ( (((uintptr_t)dst) & (SPI_MEM_PAGE_SIZE-1)) == (SPI_MEM_PAGE_SIZE-1) )
		spi_eeprom_write(dst, &s, 2);
	else {
		spi_eeprom_wait_for_last_write_end();
		spi_eeprom_write_page(dst, &s, 2);
	}
	return SPI_EEPROM_OK;
}

storage_err spi_eeprom_memmove(void* dst, const void* src, size_t count) {
	uint8_t buf[SPI_MEM_PAGE_SIZE];
	// copy in page aligned chunks
	int8_t direction = src < dst ? -1 : 1;
	while (count) {
		// offset into page
		uint8_t dst_page_off = ((uintptr_t)dst) & (SPI_MEM_PAGE_SIZE-1);
		// either (0..page_off) or (page_off..SPI_MEM_PAGE_SIZE-1) inclusive
		uint8_t dst_page_remaining = direction > 0 ? (SPI_MEM_PAGE_SIZE - dst_page_off) : (dst_page_off + 1);
		uint8_t n = count < dst_page_remaining ? count : dst_page_remaining;
		if(direction < 0){
			src -= n;
			dst -= n;
		}
		spi_eeprom_wait_for_last_write_end();
		if(spi_eeprom_read(src, buf, n) != n)
			return storage_errno;
		if(spi_eeprom_write_page(dst, buf, n) != n)
			return storage_errno;
		if(direction > 0){
		    src += n;
		    dst += n;
		}
		count -= n;
		USB_KeepAlive(true);
	}
	return SPI_EEPROM_OK;
}


uint8_t spi_eeprom_test_read(void){
	static uint8_t* next_addr = 0;
	static uint8_t buf[8]; // read 8 at a time to prove multi-read
	static uint8_t bufp = 0;
	if (0 == bufp && 0 == next_addr) {
		uint8_t rv = spi_eeprom_read_status();
		printing_set_buffer(byte_to_str(rv|0x70), sram);
		bufp = sizeof(buf);
		return 1;
	}

	if(bufp >= sizeof(buf)){
		int16_t r = spi_eeprom_read(next_addr, buf, sizeof(buf));
		if(r != sizeof(buf)){
			next_addr = 0;
			switch(storage_errno){
			case SPI_EEPROM_OK:
				printing_set_buffer(CONST_MSG("SPI_EEPROM_OK\n"), CONSTANT_STORAGE);
				break;
			default:
				printing_set_buffer(CONST_MSG("WTF_ERROR"), CONSTANT_STORAGE);
			}
			return 0;
		}
		next_addr += sizeof(buf);
		bufp = 0;
	}

	uint8_t this_byte = buf[bufp++];
	printing_set_buffer(byte_to_str(this_byte), sram);
	return 1;
}

uint8_t spi_eeprom_test_write(void){
	static uint16_t block[8] = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e };
	static uint8_t* addr = 0;

	//spi_enable_write(); return 1;
	int8_t r = spi_eeprom_write_page(addr, block, 16);
	if(r != 16){
		addr = 0;
		switch(storage_errno){
		case SPI_EEPROM_OK:
			printing_set_buffer(CONST_MSG("SPI_EEPROM_OK\n"), CONSTANT_STORAGE);
			break;
		default:
			printing_set_buffer(CONST_MSG("WTF_ERROR\n"), CONSTANT_STORAGE);
		}
		return 0;
	}
	else{
		printing_set_buffer(byte_to_str(((uint16_t)addr)/16), sram);
		addr += 16;
		for(int i = 0; i < 8; ++i){
			block[i] += 0x10;
		}
		return 1;
	}
}

