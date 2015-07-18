# Copyright (c) Chris Andreae
# Copyright (c) Juraj Hercek
# Copyright (c) Peter Hercek

SRC += \
    Descriptors.c \
    printing.c \
    keystate.c \
    config.c \
    serial_eeprom_spi.c \
    buzzer.c \
    hardware.c \
    interpreter.c \
    macro_index.c \
    macro.c \
    extrareport.c \
    sort.c \
    lufa/lufa_main.c \
    lufa/eeext_endpoint_stream.c \
    LiquidCrystal/WString.cpp \
    LiquidCrystal/Print.cpp \
    LiquidCrystal/LiquidCrystal.cpp \
    Lcd.cpp

CC_FLAGS += \
    -I$(LUFA_PATH) \
    -ILiquidCrystal \
    -DBUILD_FOR_LUFA \
    -DHARDWARE_VARIANT=KATY \
    -DHAS_EXTERNAL_STORAGE=0 \
    -DMACROS_IN_INTERNAL_STORAGE

CPP_FLAGS += \
    -std=c++11 \
    -Wc++0x-compat
