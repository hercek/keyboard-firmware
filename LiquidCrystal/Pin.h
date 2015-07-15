// Copyright (c) 2014 Juraj Hercek
//  Licensed under the GNU GPL v2 (see GPL2.txt).
// TODO: Possibly add more stuff here (related to LUFA/VUSB copyright.

#ifndef Pin_h
#define Pin_h

#include <stdint.h>
#include <avr/sfr_defs.h>
#include "Bus.h"

class Pin
{
	public:
		Pin(uint8_t bit, volatile uint8_t* dirReg, volatile uint8_t* dataReg, volatile uint8_t* pinReg)
			: m_bit(bit)
			, m_dirReg(dirReg)
			, m_dataReg(dataReg)
			, m_pinReg(pinReg)
		{}
	public:
		void setInput()  { *m_dirReg &= ~_BV(m_bit); }
		void setOutput() { *m_dirReg |= _BV(m_bit); }
		void setLow()    { *m_dataReg &= ~_BV(m_bit); }
		void setHigh()   { *m_dataReg |= _BV(m_bit); }
		uint8_t value()  { return *m_pinReg & ~_BV(m_bit) ? 1 : 0; }
	private:
		uint8_t m_bit;
		volatile uint8_t* m_dirReg;
		volatile uint8_t* m_dataReg;
		volatile uint8_t* m_pinReg;
};


template < int8_t width >
class PinBus : public Bus
{
	public:
		PinBus( Pin pins[width] ) : m_pins(pins) {
			static_assert(0<width && width<=8, "bus width must be from 1 to 8"); }
	public:
		bool isFullSize() { return 8 == width; }
		void setInput()   { for (int8_t i = 0; i < width; ++i) m_pins[i].setInput(); }
		void setOutput()  { for (int8_t i = 0; i < width; ++i) m_pins[i].setOutput(); }
		uint8_t read() {
			uint8_t rv = 0;
			for (int8_t i = width-1; i >= 0; --i) {
				rv <<= 1;
				if (m_pins[i].value()) rv |= 1;
			}
			return rv;
		}
		void write(uint8_t x) {
			for (int8_t i = 0; i < width; ++i) {
				if (x & 0x01) m_pins[i].setHigh();
				else m_pins[i].setLow();
				x >>= 1;
			}
		}
	private:
		Pin* m_pins;
};

#endif
