// Copyright (c) 2014 Juraj Hercek
//                    <other authors go here>
//  Licensed under the GNU GPL v2 (see GPL2.txt).
// TODO: Possibly add more stuff here (related to LUFA/VUSB copyright.


// NOTE: Make sure LCD is manipulated out of the matrix processing
//       because it shares data bits with OutRColX pins.

//#define USE_PIN_BUS

#include "LUFA/Common/Common.h" // to get ARCH_* macros defined

#include "Lcd.h"
#include "LiquidCrystal/Pin.h"
#ifndef USE_PIN_BUS
#include "LiquidCrystal/Nyble.h"
#endif
#include "LiquidCrystal/LiquidCrystal.h"

#if (ARCH == ARCH_AVR8)
#  define PIN(O,X,Y) Pin O( P##X##Y, &DDR##X, &PORT##X, &PIN##X )
static PIN(LcdRS,D,4);
static PIN(LcdRW,B,5);
static PIN(LcdEn,C,7);
#elif (ARCH == ARCH_XMEGA)
#  define PIN(O,X,Y) Pin O( PIN##Y##_bp, &PORT##X##_DIR, &PORT##X##_OUT, &PORT##X##_IN )
static PIN(LcdRS,R,0);
static PIN(LcdEn,R,1);
#else
#  error "Unknown architecture."
#endif

#ifdef USE_PIN_BUS
// See keyboard.txt for data pin assignment
#if (ARCH == ARCH_AVR8)
#  define P(X,Y) Pin( P##X##Y, &DDR##X, &PORT##X, &PIN##X )
#elif (ARCH == ARCH_XMEGA)
#  define P(X,Y) Pin( PIN##Y##_bp, &PORT##X##_DIR, &PORT##X##_OUT, &PORT##X##_IN )
#else
#  error "Unknown architecture."
#endif
static Pin BusPins[8] = {P(A,0), P(A,1), P(A,2), P(A,3), P(A,4), P(A,5), P(A,6), P(A,7) };
static PinBus<8> PinBus8(BusPins);
#undef P
static LiquidCrystal lcd( &LcdRS, /*&LcdRW,*/ &LcdEn, &PinBus8 );

#else // USE_PIN_BUS

#if (ARCH == ARCH_AVR8)
#define NYBLE(m,p) m,&DDR##p,&PORT##p,&PIN##p
static NybleBus8 NybleBus( true, NYBLE(0xf0,F), NYBLE(0x0f,D) );
static LiquidCrystal lcd( &LcdRS, /*&LcdRW,*/ &LcdEn, &NybleBus );
#undef NYBLE
#elif (ARCH == ARCH_XMEGA)
static ByteBus8 LcdByteBus(&PORTA_DIR, &PORTA_OUT, &PORTA_IN);
static LiquidCrystal lcd( &LcdRS, /*&LcdRW,*/ &LcdEn, &LcdByteBus );
#else
#  error "Unknown architecture."
#endif

#endif // USE_PIN_BUS

#undef PIN

void lcd_init(void) {
#if (ARCH == ARCH_AVR8)
	LcdRW.setOutput();
	LcdRW.setLow();
#elif (ARCH == ARCH_XMEGA)
   // XMega has R/W tied to UsbGND (i.e. always low)
#else
#  error "Unknown architecture."
#endif
	lcd.begin(8, 2);
}

void lcd_clear(void)
{
	lcd.clear();
}

void lcd_print(const char* text) {
	lcd.print(text);
}

void lcd_set_position(const uint8_t row, const uint8_t col) {
	lcd.setCursor(col, row);
}

void lcd_print_position(const uint8_t row, const uint8_t col, const char* text) {
	lcd.setCursor(col, row);
	lcd_print(text);
}
