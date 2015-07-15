// Copyright (c) 2014 Peter Hercek
//  Licensed under the GNU GPL v2 (see GPL2.txt).

#ifndef Nyble_h
#define Nyble_h

#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include "Bus.h"

class PortBus : public Bus
{
	public:
		PortBus( uint8_t mask, volatile uint8_t* dirReg, volatile uint8_t* outReg, volatile uint8_t* inReg)
			: m_mask(mask) , m_dirReg(dirReg) , m_outReg(outReg) , m_inReg(inReg) {}
	public:
		bool isFullSize() { return 0xff == m_mask; }
		void setInput()  { *m_dirReg &= ~m_mask; }
		void setOutput() { *m_dirReg |= m_mask; }
		uint8_t read()   { return *m_inReg & m_mask; }
		void write(uint8_t val) {
			val &= m_mask;
			cli(); //disableInterrupts
			*m_outReg = (*m_outReg & ~m_mask) | val;
			sei(); //enableInterrupts
		}
	private:
		uint8_t const m_mask;
		volatile uint8_t* const m_dirReg;
		volatile uint8_t* const m_outReg;
		volatile uint8_t* const m_inReg;
};


class ByteBus8 : public PortBus
{
	public:
		ByteBus8(volatile uint8_t* dirReg, volatile uint8_t* outReg, volatile uint8_t* inReg)
			: PortBus(0xff, dirReg, outReg, inReg) {}
};


class NybleBus8 : public Bus
{
	public:
		NybleBus8( bool shouldSwap,
			uint8_t lowMask, volatile uint8_t* lowDirReg, volatile uint8_t* lowOutReg, volatile uint8_t* lowInReg,
			uint8_t hghMask, volatile uint8_t* hghDirReg, volatile uint8_t* hghOutReg, volatile uint8_t* hghInReg )
				: m_shouldSwap( shouldSwap )
				, m_lowNyble( lowMask, lowDirReg, lowOutReg, lowInReg )
				, m_hghNyble( hghMask, hghDirReg, hghOutReg, hghInReg ) {}
	public:
		bool isFullSize() { return true; }
		void setInput()   { m_lowNyble.setInput();  m_hghNyble.setInput(); }
		void setOutput()  { m_lowNyble.setOutput(); m_hghNyble.setOutput(); }
		uint8_t read() {
			uint8_t rv = m_lowNyble.read() | m_hghNyble.read();
			if (m_shouldSwap)
				asm volatile("swap %0" : "=r" (rv) : "0" (rv));
			return rv;
		}
		void write(uint8_t val) {
			if (m_shouldSwap)
				asm volatile("swap %0" : "=r" (val) : "0" (val));
			m_lowNyble.write(val);
			m_hghNyble.write(val);
		}
	private:
		bool const m_shouldSwap;
		PortBus m_lowNyble;
		PortBus m_hghNyble;
};

#endif
