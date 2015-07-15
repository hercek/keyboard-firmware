// Copyright (c) 2014 Juraj Hercek
//                    <other authors go here>
//  Licensed under the GNU GPL v2 (see GPL2.txt).
// TODO: Possibly add more stuff here (related to LUFA/VUSB copyright.


// NOTE: Make sure LCD is manipulated out of the matrix processing
//       because it shares data bits with OutRColX pins.

//#define USE_PIN_BUS

#include "Lcd.h"
#include "LiquidCrystal/Pin.h"
#ifndef USE_PIN_BUS
#include "LiquidCrystal/Nyble.h"
#endif
#include "LiquidCrystal/LiquidCrystal.h"

#define PIN(O,X,Y) Pin O( P##X##Y, &DDR##X, &PORT##X, &PIN##X )
static PIN(LcdRS,D,4);
static PIN(LcdRW,B,5);
static PIN(LcdEn,C,7);
#ifdef USE_PIN_BUS
// See keyboard.txt for data pin assignment
#define P(X,Y) Pin( P##X##Y, &DDR##X, &PORT##X, &PIN##X )
static Pin BusPins[8] = {P(F,4), P(F,5), P(F,6), P(F,7), P(D,0), P(D,1), P(D,2), P(D,3) };
static PinBus<8> PinBus8(BusPins);
#undef P
static LiquidCrystal lcd( &LcdRS, /*&LcdRW,*/ &LcdEn, &PinBus8 );
#else // USE_PIN_BUS
#define NYBLE(m,p) m,&DDR##p,&PORT##p,&PIN##p
static NybleBus8 NybleBus( true, NYBLE(0xf0,F), NYBLE(0x0f,D) );
static LiquidCrystal lcd( &LcdRS, /*&LcdRW,*/ &LcdEn, &NybleBus );
#undef NYBLE
#endif // USE_PIN_BUS
#undef PIN

void lcd_init(void) {
	LcdRW.setOutput();
	LcdRW.setLow();
	//lcd.begin(8, 2);
}

void lcd_clear(void)
{
	lcd.clear();
}

void lcd_print(const char* text) {
	lcd.print(text);
}

void lcd_print_position(const uint8_t row, const uint8_t col, const char* text) {
	lcd.setCursor(col, row);
	lcd_print(text);
}
