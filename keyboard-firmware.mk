# Copyright (c) Chris Andreae
# Copyright (c) Juraj Hercek
# Copyright (c) Peter Hercek

SRC += \
    Descriptors.c \
    printing.c \
    keystate.c \
    config.c \
    buzzer.c \
    hardware.c \
    interpreter.c \
    macro_index.c \
    macro.c \
    extrareport.c \
    sort.c \
    storage.c \
    storage/spi_eeprom.c \
    storage/avr_eeprom.c \
    storage/avr_pgm.c \
    lufa/lufa_main.c \
    lufa/spi_eeprom_endpoint_stream.c \
    LiquidCrystal/WString.cpp \
    LiquidCrystal/Print.cpp \
    LiquidCrystal/LiquidCrystal.cpp \
    Lcd.cpp

CC_FLAGS += \
    -I$(LUFA_PATH) \
    -ILiquidCrystal \
    -DBUILD_FOR_LUFA \
    -DHARDWARE_VARIANT=K84CS
#    -DKATY_DEBUG
#    -DNO_EXTERNAL_STORAGE


CPP_FLAGS += \
    -std=c++11 \
    -Wc++0x-compat
