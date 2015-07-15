// Copyright (c) 2014 Juraj Hercek
//                    <other authors go here>
//  Licensed under the GNU GPL v2 (see GPL2.txt).
// TODO: Possibly add more stuff here (related to LUFA/VUSB copyright.


// NOTE: Make sure LCD is manipulated out of the matrix processing
//       because it shares data bits with OutRColX pins.

#include "Lcd.h"
#include "LiquidCrystal/Pin.h"
#include "LiquidCrystal/LiquidCrystal.h"

#define PIN(O,X,Y) Pin O( P##X##Y, &DDR##X, &PORT##X, &PIN##X )
// See keyboard.txt for data pin assignment
static PIN(LcdRS,D,4);
static PIN(LcdRW,B,5);
static PIN(LcdEn,C,7);
static PIN(LcdD0,F,4);
static PIN(LcdD1,F,5);
static PIN(LcdD2,F,6);
static PIN(LcdD3,F,7);
static PIN(LcdD4,D,0);
static PIN(LcdD5,D,1);
static PIN(LcdD6,D,2);
static PIN(LcdD7,D,3);
#undef PIN

static LiquidCrystal
	 lcd( &LcdRS, /*&LcdRW,*/ &LcdEn
//		, &LcdD0, &LcdD1, &LcdD2, &LcdD3
		, &LcdD4, &LcdD5, &LcdD6, &LcdD7
		);

void lcd_init(void) {
	LcdRW.setOutput();
	LcdRW.setLow();
	lcd.begin(8, 2);
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
