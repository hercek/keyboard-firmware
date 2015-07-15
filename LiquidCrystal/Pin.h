// Copyright (c) 2014 Juraj Hercek
//  Licensed under the GNU GPL v2 (see GPL2.txt).
// TODO: Possibly add more stuff here (related to LUFA/VUSB copyright.

#include <stdint.h>
#include <avr/sfr_defs.h>

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
		void setInput()
		{
			*m_dirReg &= ~_BV(m_bit);
		}
		void setOutput()
		{
			*m_dirReg |= _BV(m_bit);
		}
		void setLow()
		{
			*m_dataReg &= ~_BV(m_bit);
		}
		void setHigh()
		{
			*m_dataReg |= _BV(m_bit);
		}
		uint8_t value()
		{
			return *m_pinReg & ~_BV(m_bit) ? 1 : 0;
		}
	private:
		uint8_t m_bit;
		volatile uint8_t* m_dirReg;
		volatile uint8_t* m_dataReg;
		volatile uint8_t* m_pinReg;
};
