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

typedef enum _SPI_COMMAND
{
    WRSR  = 0b0001,
    WRITE = 0b0010,
    READ  = 0b0011,
    WRDI  = 0b0100,
    RDSR  = 0b0101,
    WREN  = 0b0110
} SPI_COMMAND;

static inline void spi_slave_on(void)
{
    // SS is active low.
    SPI_PORT_SS &= ~_BV(SPI_BIT_SS);
}

static inline void spi_slave_off(void)
{
    // SS is active low.
    SPI_PORT_SS |= _BV(SPI_BIT_SS);
}

static inline uint8_t spi_transfer(volatile uint8_t byte_data)
{
    SPDR = byte_data;
    while (!(SPSR & _BV(SPIF)));
    return SPDR;
}

// 1787108.pdf:9
static void spi_enable_write(void)
{
    spi_slave_on();
    (void) spi_transfer(WREN);
    spi_slave_off();
    _delay_ms(1); // KATY-TODO: fine tune this
}

// writes one chunk of data that fits the page size.
static int8_t serial_eeprom_continue_write(const uint8_t* buf, uint8_t len)
{
    // prerequisites:
    // - write is within page boundary
    for (int i = 0; i < len; ++i)
    {
        (void) spi_transfer(buf[i]);
    }

    return len;
}

// Errno for SPI EEPROM functions.
serial_eeprom_err serial_eeprom_errno;

// Called from:
// - ergodox.c
void serial_eeprom_init(void)
{
    // Initialize Pins.
    SPI_DDR_SS |= _BV(SPI_DD_SS);      //OUTPUT
    SPI_DDR_SCK |= _BV(SPI_DD_SCK);    //OUTPUT
    SPI_DDR_MOSI |= _BV(SPI_DD_MOSI);  //OUTPUT
    SPI_DDR_MISO &= ~_BV(SPI_DD_MISO); //INPUT

    // Keep slave inactive.
    spi_slave_off();

    // Make sure that original ATMega32u4 SS (PB0) is set to output, otherwise
    // we could get MSTR bit reset to 0 (see ATMega32U4.pdf, page 182).
    DDRB |= _BV(DDB0); // for KATY, PB0 is CLK1

    // Initialize SPI subsystem in master mode, clock set to 4MHz
    SPCR = _BV(SPE) | _BV(MSTR); //ATMega32U4.pdf:182
    SPSR &= ~_BV(SPI2X);
}

// Called from:
// - lufa/eext_endpoint_stream.c
serial_eeprom_err serial_eeprom_start_write(uint8_t* addr)
{
    // Precondition:
    // - the spi_slave_off() was called prior this call

    // The write/read was executed, we need to enable write latch before any
    // writing can take place.
    spi_enable_write();
    // Begin write sequence now...
    spi_slave_on();
    (void) spi_transfer(WRITE);
    // at this point, SS is low, write is in progress...
    return SUCCESS;
}

// Called from:
// - lufa/eext_endpoint_stream.c
serial_eeprom_err serial_eeprom_write_step(uint8_t* addr, uint8_t* data, uint8_t len, uint8_t last)
{
    // Precondition:
    // - buffer doesn't cross page boundary (because it is written in one go
    //   without checking for page boundary within
    //   seriall_eeprom_continue_write() function)

    serial_eeprom_err result;

    if (((intptr_t)addr & (EEEXT_PAGE_SIZE-1)) == 0)
    {
        // page aligned, start write
        if (result = serial_eeprom_start_write(addr), result != SUCCESS)
        {
            return result;
        }
    }

    if (len != serial_eeprom_continue_write(data, len))
    {
        return serial_eeprom_errno;
    }

    if (last || ((((intptr_t)addr+len) & (EEEXT_PAGE_SIZE-1)) == 0))
    {
        // enforce page write cycle
        spi_slave_off();
    }

    return DATA_ERROR;
}

// Called from:
// - config.c
// - interpreter.c
// - interpreter_harness.c
// - vusb/vusb_main.c
int16_t serial_eeprom_read(const uint8_t* addr, uint8_t* buf, uint16_t len)
{
    spi_slave_on();

    (void) spi_transfer(READ);
    (void) spi_transfer((uint8_t)((uint16_t)(addr) >> 8));//high part of 16bit address
    (void) spi_transfer((uint8_t)((uint16_t)(addr) & 0xFF));//low part of 16bit address

    int16_t i;
    for (i = 0; i < len; ++i)
    {
        buf[i] = spi_transfer(0xff);
    }

    spi_slave_off();

    return i;
}

// Called from:
// - config.c
// tag: higher level function
int8_t serial_eeprom_write_page(uint8_t* addr, const uint8_t* buf, uint8_t len)
{
    int8_t result;

    if (result = serial_eeprom_start_write(addr), result != SUCCESS)
    {
        serial_eeprom_errno = result;
        result = 0;
    }
    else
    {
        result = serial_eeprom_continue_write(buf, len);
    }
    // Finish the page write by invoking of write cycle.
    spi_slave_off();

    return result;
}

// Called from:
// - macro.c
// Implementation equal to the one in serial_eeprom.c file.
// tag: higher level function
int16_t serial_eeprom_write(uint8_t* dst, const uint8_t* buf, uint16_t count)
{
    int16_t written = 0;
    while(count){
        uint8_t dst_page_off = ((intptr_t) dst) & (EEEXT_PAGE_SIZE - 1);
        uint8_t dst_page_remaining = EEEXT_PAGE_SIZE - dst_page_off;
        uint8_t n = (count < dst_page_remaining) ? (uint8_t)count : dst_page_remaining;

        int8_t w = serial_eeprom_write_page(dst, buf, n);
        written += w;
        if(w != n){
            // error: incomplete write
            return (written == 0) ? written : -1;
        }
        buf += n;
        dst += n;
        count -= n;
        USB_KeepAlive(true);
    }
    return written;
}

// Called from:
// - macro.c
// Implementation equal to the one in serial_eeprom.c file.
// tag: higher level function
serial_eeprom_err serial_eeprom_memmove(uint8_t* dst, uint8_t* src, size_t count)
{
    uint8_t buf[EEEXT_PAGE_SIZE];
    // copy in page aligned chunks

    int8_t direction = src < dst ? -1 : 1;

    while(count){
        // offset into page
        uint8_t dst_page_off = ((intptr_t) dst) & (EEEXT_PAGE_SIZE - 1);
        // either (0..page_off) or (page_off..15) inclusive
        uint8_t dst_page_remaining = direction > 0 ? (EEEXT_PAGE_SIZE - dst_page_off) : (dst_page_off + 1);
        uint8_t n = count < dst_page_remaining ? count : dst_page_remaining;
        if(direction < 0){
            src -= n;
            dst -= n;
        }
        if(serial_eeprom_read(src, buf, n) != n){
            return serial_eeprom_errno;
        }
        if(serial_eeprom_write_page(dst, buf, n) != n){
            return serial_eeprom_errno;
        }
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
	static uint8_t* addr = 0;
	static uint8_t buf[8]; // read 8 at a time to prove multi-read
	static uint8_t bufp = sizeof(buf);

	uint8_t next_byte;
	if(bufp >= sizeof(buf)){
		int16_t r = serial_eeprom_read(addr, buf, sizeof(buf));
		if(r != sizeof(buf)){
			addr = 0;
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
		addr += sizeof(buf);
		bufp = 0;
	}

	next_byte = buf[bufp++];

	printing_set_buffer(byte_to_str(next_byte), BUF_MEM);
	return 1;
}

// KATY-TODO: remove in release
static uint16_t block[8] = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e };
uint8_t serial_eeprom_test_write(void){
	static uint8_t* addr = 0;

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
