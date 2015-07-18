/* Copyright (c) 2014 Juraj Hercek et. al.
   Author(s): Juraj Hercek
              ...

   Licensed under the GNU GPL v2 (see GPL2.txt).
   KATY-TODO: Add another copyright stuff (lufa/vusb...)
*/

/*
KATY-TODO: encapsulate serial_eeprom_errno, (used from interpreter.c) by
returning it via get_spi_eeprom_errno() function.
*/

/* Include generic serial eeprom interface. */
#include "serial_eeprom.h"

/* Include specific spic eeprom interface. */
#include "serial_eeprom_spi.h"

#include "usb.h"
#include "printing.h"

/* Expects to have following defined:
   - DDR_MOSI - DDR SPI Master Out, Slave In
   - DDR_MISO - DDR SPI Master In, Slave Out
   - DDR_SCK  - DDR SPI clock
   - DDR_SS   - DDR slave/chip select
*/

/* Following defines should be in hardware specific header file */
#define SPI_DD_SS   DDE2
#define SPI_DD_SCK  DDB1
#define SPI_DD_MOSI DDB2
#define SPI_DD_MISO DDB3
#define SPI_DDR_SS   DDRE
#define SPI_DDR_SCK  DDRB
#define SPI_DDR_MOSI DDRB
#define SPI_DDR_MISO DDRB
#define SPI_PORT_SS PORTE
#define SPI_BIT_SS  PORTE2

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
	SPI_PORT_SS &= ~_BV(SPI_BIT_SS); // SS is active low.
	// SS setup time is 100 ns -> one NOP here and one instruction to actually use SPI will do.
	asm volatile( "nop\n\t" ::);
}

static inline void spi_slave_off(void) {
	// SS hold time is 200 ns -> 3 instructions and 2 instructions here will do.
	SPI_PORT_SS |= _BV(SPI_BIT_SS); // SS is active low.
}

static inline uint8_t spi_transfer(volatile uint8_t byte_data) {
	SPDR = byte_data;
	while (!(SPSR & _BV(SPIF)));
	return SPDR;
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
static int8_t serial_eeprom_continue_write(const uint8_t* buf, uint8_t len) {
	for (uint8_t i = 0; i < len; ++i)
		(void) spi_transfer(buf[i]);
	return len;
}

// Errno for SPI EEPROM functions.
serial_eeprom_err serial_eeprom_errno;

// Read eeprom status register.
// Usefull to check whether the last write finished internally.
static uint8_t serial_eeprom_read_status(void) {
	spi_slave_on();
	(void) spi_transfer(RDSR);
	uint8_t rv = spi_transfer(0xff);
	spi_slave_off();
	return rv;
}

// Write eeprom status register.
static void serial_eeprom_write_status(uint8_t new_status) {
	spi_slave_on();
	(void) spi_transfer(WRSR);
	(void) spi_transfer(new_status);
	spi_slave_off();
}

// Called from:
// - ergodox.c
void serial_eeprom_init(void) {
	// Initialize Pins.
	SPI_DDR_SS |= _BV(SPI_DD_SS);      //OUTPUT
	SPI_DDR_SCK |= _BV(SPI_DD_SCK);    //OUTPUT
	SPI_DDR_MOSI |= _BV(SPI_DD_MOSI);  //OUTPUT
	SPI_DDR_MISO &= ~_BV(SPI_DD_MISO); //INPUT
	// Keep slave inactive.
	spi_slave_off();
	// Make sure that original ATMega32u4 SS (PB0) is set to output, otherwise
	// we could get MSTR bit reset to 0 (see ATMega32U4.pdf, page 182).
	DDRB |= _BV(DDB0); // for KATY, PB0 is CLK1, which is output, so this is OK
	// Initialize SPI subsystem in master mode, clock set to 4 MHz
	SPCR = _BV(SPE) | _BV(MSTR); //ATMega32U4.pdf:182
	SPSR &= ~_BV(SPI2X); // do not raise clock from 4 to 8 MHz
	// check writing is enabled into to whole EEPROM
	uint8_t rv = serial_eeprom_read_status();
	if ( rv & SPI_STATUS_WRITE_DISABLED_MASK )
		serial_eeprom_write_status(0); // enable write to the whole EEPROM
}

// Wait till internal EEPROM write cycle finishes.
// Notice that the internal write cycle can last as much
// as 6 ms after SS from the last write was deactivated.
void serial_eeprom_wait_for_last_write_end() {
	for(;;) {
		uint8_t rv = serial_eeprom_read_status();
		if (! ( rv & SPI_STATUS_WRITE_IN_PROGRESS_MASK ) ) break;
		_delay_us(1); // wait a bit and check again
	}
}

// Initializes an eeprom page for a write operation.
// Called from: lufa/eext_endpoint_stream.c
// Precondition: * SS is not active.
//               * caller must call this at least 6 ms after the last write or
//                 it must call serial_eeprom_wait_for_last_write_end() before this
serial_eeprom_err serial_eeprom_start_write(uint8_t* addr) {
	spi_enable_write();
	spi_slave_on();
	(void) spi_transfer(WRITE);
	(void) spi_transfer((uint8_t)((uint16_t)(addr) >> 8));//high part of 16bit address
	(void) spi_transfer((uint8_t)((uint16_t)(addr) & 0xFF));//low part of 16bit address
	// at this point, SS is active (LOW), the write is in progress ...
	// ... only the data needs to be sent and then SS deactivated
	return SUCCESS;
}

// Called from: lufa/eext_endpoint_stream.c
// Precondition: buffer doesn't cross page boundary
serial_eeprom_err serial_eeprom_write_step(uint8_t* addr, uint8_t* data, uint8_t len, uint8_t last) {
	if ( (((intptr_t)addr) & (EEEXT_PAGE_SIZE-1)) == 0 ) {
		// start write at each page start
		serial_eeprom_err result;
		serial_eeprom_wait_for_last_write_end();
		if ( SUCCESS != (result = serial_eeprom_start_write(addr)) ) return result;
	}
	if (len != serial_eeprom_continue_write(data, len)) return serial_eeprom_errno;
	if (last || ((((intptr_t)addr+len) & (EEEXT_PAGE_SIZE-1)) == 0))
		spi_slave_off(); // enforce page write cycle
	return SUCCESS;
}

// Called from:
// - config.c
// - interpreter.c
// - interpreter_harness.c
// - vusb/vusb_main.c
// Note: this can read through page boundaries.
// Precondition: caller must call this at least 6 ms after the last write or
//               it must call serial_eeprom_wait_for_last_write_end() before this
int16_t serial_eeprom_read(const uint8_t* addr, uint8_t* buf, uint16_t len) {
	spi_slave_on();
	(void) spi_transfer(READ);
	(void) spi_transfer((uint8_t)((uint16_t)(addr) >> 8));//high part of 16bit address
	(void) spi_transfer((uint8_t)((uint16_t)(addr) & 0xFF));//low part of 16bit address
	for (int16_t i = 0; i < len; ++i)
		buf[i] = spi_transfer(0xff);
	spi_slave_off();
	return len;
}

// Called from: config.c
// Precondition: caller must call this at least 6 ms after the last write or
//               it must call serial_eeprom_wait_for_last_write_end() before this
// tag: higher level function
int8_t serial_eeprom_write_page(uint8_t* addr, const uint8_t* buf, uint8_t len) {
	int8_t result;
	if ( SUCCESS != (result = serial_eeprom_start_write(addr)) ) {
		serial_eeprom_errno = result; result = 0;
	} else
		result = serial_eeprom_continue_write(buf, len);
	spi_slave_off(); // Finish the page write by invoking of write cycle.
	return result;
}

// Called from: macro.c
// Implementation equal to the one in serial_eeprom.c file.
// tag: higher level function
int16_t serial_eeprom_write(uint8_t* dst, const uint8_t* buf, uint16_t count) {
	int16_t written = 0;
	while (count) {
		uint8_t dst_page_off = ((intptr_t)dst) & (EEEXT_PAGE_SIZE-1);
		uint8_t dst_page_remaining = EEEXT_PAGE_SIZE - dst_page_off;
		uint8_t n = (count < dst_page_remaining) ? (uint8_t)count : dst_page_remaining;
		serial_eeprom_wait_for_last_write_end();
		int8_t w = serial_eeprom_write_page(dst, buf, n);
		written += w;
		if(w != n)
			return (written==0) ? written : -1; // error: incomplete write
		buf += n;
		dst += n;
		count -= n;
		USB_KeepAlive(true);
	}
	return written;
}

// Called from: macro.c
// Implementation equal to the one in serial_eeprom.c file.
// tag: higher level function
serial_eeprom_err serial_eeprom_memmove(uint8_t* dst, uint8_t* src, size_t count) {
	uint8_t buf[EEEXT_PAGE_SIZE];
	// copy in page aligned chunks
	int8_t direction = src < dst ? -1 : 1;
	while (count) {
		// offset into page
		uint8_t dst_page_off = ((intptr_t)dst) & (EEEXT_PAGE_SIZE-1);
		// either (0..page_off) or (page_off..EEEXT_PAGE_SIZE-1) inclusive
		uint8_t dst_page_remaining = direction > 0 ? (EEEXT_PAGE_SIZE - dst_page_off) : (dst_page_off + 1);
		uint8_t n = count < dst_page_remaining ? count : dst_page_remaining;
		if(direction < 0){
			src -= n;
			dst -= n;
		}
		serial_eeprom_wait_for_last_write_end();
		if(serial_eeprom_read(src, buf, n) != n)
			return serial_eeprom_errno;
		if(serial_eeprom_write_page(dst, buf, n) != n)
			return serial_eeprom_errno;
		if(direction > 0){
		    src += n;
		    dst += n;
		}
		count -= n;
		USB_KeepAlive(true);
	}
	return SUCCESS;
}


// KATY-TODO: remove in release
uint8_t serial_eeprom_test_read(void){
	static uint8_t* next_addr = 0;
	static uint8_t buf[8]; // read 8 at a time to prove multi-read
	static uint8_t bufp = 0;
	if (0 == bufp && 0 == next_addr) {
		uint8_t rv = serial_eeprom_read_status();
		printing_set_buffer(byte_to_str(rv|0x70), BUF_MEM);
		bufp = sizeof(buf);
		return 1;
	}

	if(bufp >= sizeof(buf)){
		int16_t r = serial_eeprom_read(next_addr, buf, sizeof(buf));
		if(r != sizeof(buf)){
			next_addr = 0;
			switch(serial_eeprom_errno){
			case RSELECT_ERROR:
				printing_set_buffer(PGM_MSG("RSELECT_ERROR"), BUF_PGM);
				break;
			case WSELECT_ERROR:
				printing_set_buffer(PGM_MSG("WSELECT_ERROR"), BUF_PGM);
				break;
			case ADDRESS_ERROR:
				printing_set_buffer(PGM_MSG("ADDRESS_ERROR"), BUF_PGM);
				break;
			case DATA_ERROR:
				printing_set_buffer(PGM_MSG("DATA_ERROR"), BUF_PGM);
				break;
			case SUCCESS:
				printing_set_buffer(PGM_MSG("SUCCESS:WHAT_ERROR?\n"), BUF_PGM);
				break;
			default:
				printing_set_buffer(PGM_MSG("WTF_ERROR"), BUF_PGM);
			}
			return 0;
		}
		next_addr += sizeof(buf);
		bufp = 0;
	}

	uint8_t this_byte = buf[bufp++];
	printing_set_buffer(byte_to_str(this_byte), BUF_MEM);
	return 1;
}

// KATY-TODO: remove in release
uint8_t serial_eeprom_test_write(void){
	static uint16_t block[8] = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e };
	static uint8_t* addr = 0;

	//spi_enable_write(); return 1;
	int8_t r = serial_eeprom_write_page(addr, (uint8_t*)block, 16);
	if(r != 16){
		addr = 0;
		switch(serial_eeprom_errno){
		case RSELECT_ERROR:
			printing_set_buffer(PGM_MSG("RSELECT_ERROR\n"), BUF_PGM);
			break;
		case WSELECT_ERROR:
			printing_set_buffer(PGM_MSG("WSELECT_ERROR\n"), BUF_PGM);
			break;
		case ADDRESS_ERROR:
			printing_set_buffer(PGM_MSG("ADDRESS_ERROR\n"), BUF_PGM);
			break;
		case DATA_ERROR:
			printing_set_buffer(PGM_MSG("DATA_ERROR\n"), BUF_PGM);
			break;
		case SUCCESS:
			printing_set_buffer(PGM_MSG("SUCCESS:WHAT_ERROR?\n"), BUF_PGM);
			break;
		default:
			printing_set_buffer(PGM_MSG("WTF_ERROR\n"), BUF_PGM);
		}
		return 0;
	}
	else{
		printing_set_buffer(byte_to_str(((uint16_t)addr)/16), BUF_MEM);
		addr += 16;
		for(int i = 0; i < 8; ++i){
			block[i] += 0x10;
		}
		return 1;
	}
}

