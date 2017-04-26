// Copyright (c) 2014 Juraj Hercek
//  Licensed under the GNU GPL v2 (see GPL2.txt).
// TODO: Possibly add more stuff here (related to LUFA/VUSB copyright.

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif

// low level interface
void lcd_init(void);
void lcd_clear(void);
void lcd_print(const char* text);
void lcd_set_position(const uint8_t row, const uint8_t col);
void lcd_print_position(const uint8_t row, const uint8_t col, const char* text);

#if defined(__cplusplus)
}
#endif
