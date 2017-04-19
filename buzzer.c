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

#include "buzzer.h"

// Pull in this beast so that we can conditionally compile for AVR8/XMEGA.
#include <LUFA/Common/Common.h>

#if USE_BUZZER
#define TIMER4_RESOLUTION 1023UL
#define PLL_FREQ 48000000UL

unsigned long hfPwmPeriod = 0;

static void initHfPwm(void) {
#if (ARCH == ARCH_AVR8)
	// Init the internal PLL (ATMega32U4.pdf:40)
	// However, since LUFA sets the PLL to 96MHz due to USB, just properly set up
	// the PLLTM0:1 prescaler bits.
	//  PLLFRQ = _BV(PDIV2);           // 48MHz PLL
	//  PLLCSR = _BV(PLLE);            // enable PLL
	//  while(!(PLLCSR & _BV(PLOCK))); // wait till PLL is stable
	//  PLLFRQ |= _BV(PLLTM0);         // 48MHz PCK
	PLLFRQ |= (_BV(PLLTM0)|_BV(PLLTM1)); // 96MHz/2 = 48MHz PCK

	DDRD |= _BV(PD6); // set PD6 (NOT(OC4D)) as output

	// Init timer counter 4 (ATMega32U4.pdf:166)
	TCCR4C &= _BV(COM4D1); // keep original value for positive output (OC4D)
	TCCR4C |= _BV(PWM4D);  // enable PWM mode based on comparator OCR4D
	TCCR4E = _BV(ENHC4);   // set to enhanced compare/PWM mode (10 bit mode)
	TCCR4D = _BV(WGM40);   // use phase and frequency correct mode
	                       // ATMeta32u4.pdf:152
#elif (ARCH == ARCH_XMEGA)
	PORTC.DIRSET = PIN4_bm; // PORTC, PIN4 set to output
	TCC1.CTRLB = TC_WGMODE_DS_B_gc | TC1_CCAEN_bm; // dual slope, CCA enable
#else
#   error "Unknown architecture."
#endif
}

static void hfPwmStart(void) {
#if (ARCH == ARCH_AVR8)
	TCCR4C |= _BV(COM4D0); // enable NOT(OC4D) pin
#elif (ARCH == ARCH_XMEGA)
	TCC1.CTRLA = TC_CLKSEL_DIV64_gc;
#else
#   error "Unknown architecture."
#endif
}
static void hfPwmStop(void)  {
#if (ARCH == ARCH_AVR8)
	TCCR4C &= ~(_BV(COM4D0)); // disable NOT(OC4D) pin
#elif (ARCH == ARCH_XMEGA)
	TCC1.CTRLA = TC_CLKSEL_OFF_gc;
#else
#   error "Unknown architecture."
#endif
}

static void hfPwmSetDuty(unsigned int duty) {
#if (ARCH == ARCH_AVR8)
	unsigned long dutyCycle = hfPwmPeriod;
	dutyCycle *= duty;
	dutyCycle >>= 9;
	TC4H = (dutyCycle) >> 8;   // high 2 bits of duty cycle (comparator OCR4D)
	OCR4D = (dutyCycle) & 255; // low 8 bits of duty cycle (comparator OCR4D)
#elif (ARCH == ARCH_XMEGA)
	TCC1.CCA = duty;
#else
#   error "Unknown architecture."
#endif
}

static void hfPwmSetPeriod(unsigned long freq)  {
#if (ARCH == ARCH_AVR8)
	unsigned long cycles = PLL_FREQ / 2 / freq; // ATMega32U4.pdf:153
	unsigned char clockSelectBits = 0;

	// Check first 14 bits
	for (int i = 0; i < 15; ++i) {
		if (cycles < TIMER4_RESOLUTION * _BV(i)) {
			hfPwmPeriod = cycles / _BV(i);
			// CS40-3 map to first LSB of TCCR4B
			clockSelectBits = i + 1;
			break;
		}
	}

	TCCR4B = clockSelectBits;
	TC4H = hfPwmPeriod >> 8;// high 2 bits of Timer/Counter TOP value
	OCR4C = hfPwmPeriod;    // low 8 bits of Timer/Counter TOP value
							// (ATMega32U4.pdf:{140,152})
	hfPwmSetDuty((TIMER4_RESOLUTION+1)/2); // hardcoded 50% duty cycle
#elif (ARCH == ARCH_XMEGA)
	unsigned long period = F_CPU/2/64/freq;
	TCC1.PER = period;
	hfPwmSetDuty(period/2);
#else
#   error "Unknown architecture."
#endif
}

// Remaining time to keep buzzer on
static uint16_t buzzer_ms;

void buzzer_init()
{
	initHfPwm();
}

void buzzer_start(uint16_t ms) {
	buzzer_start_f(ms, BUZZER_DEFAULT_TONE);
}

void buzzer_start_f(uint16_t ms, uint8_t freq) {
	buzzer_ms = ms;
	hfPwmSetPeriod(freq);
	hfPwmStart();
}
void buzzer_update(uint8_t increment) {
	if(buzzer_ms) {
		buzzer_ms = (increment >= buzzer_ms) ? 0 : buzzer_ms - increment;
		if(buzzer_ms == 0){
			hfPwmStop();
		}
	}
}

#endif
