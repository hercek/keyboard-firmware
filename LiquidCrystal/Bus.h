// Copyright (c) 2014 Peter Hercek
//  Licensed under the GNU GPL v2 (see GPL2.txt).

#ifndef Bus_h
#define Bus_h

// Interface to up to 8 bit bus.
class Bus
{
	public:
		virtual bool isFullSize();
		virtual void setInput();
		virtual void setOutput();
		virtual uint8_t read();
		virtual void write(uint8_t x);
};

#endif
