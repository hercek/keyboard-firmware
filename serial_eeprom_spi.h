/* Copyright (c) 2014 Juraj Hercek et. al.
   Author(s): Juraj Hercek
              ...

   Licensed under the GNU GPL v2 (see GPL2.txt).
   KATY-TODO: Add another copyright stuff (lufa/vusb...)
*/

/* SPI EEPROM specific code
    - contains init function (should be part of serial_eeprom.h anyways
*/

/*
 Initialize serial eeprom. Usually called from hardware specific file.
 */
void serial_eeprom_init(void);

uint8_t serial_eeprom_test_read(void);
uint8_t serial_eeprom_test_write(void);
