/*
LED Pocket Watch
http://www.frank-zhao.com/ledpocketwatch/

Copyright (C) 2012 Frank Zhao

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pins.h"

uint16_t portx[30] = {
	&PORTF,
	&PORTF,
	&PORTF,
	&PORTA,
	&PORTA,
	
	&PORTA,
	&PORTC,
	&PORTC,
	&PORTC,
	&PORTC,
	
	&PORTG,
	&PORTG,
	&PORTD,
	&PORTD,
	&PORTD,
	
	&PORTG,
	&PORTG,
	&PORTB,
	&PORTB,
	&PORTB,
	
	&PORTB,
	&PORTB,
	&PORTE,
	&PORTE,
	&PORTE,
	
	&PORTE,
	&PORTE,
	&PORTE,
	&PORTE,
	&PORTE,
};

uint16_t ddrx[30] = {
	&DDRF,
	&DDRF,
	&DDRF,
	&DDRA,
	&DDRA,
	
	&DDRA,
	&DDRC,
	&DDRC,
	&DDRC,
	&DDRC,
	
	&DDRG,
	&DDRG,
	&DDRD,
	&DDRD,
	&DDRD,
	
	&DDRG,
	&DDRG,
	&DDRB,
	&DDRB,
	&DDRB,
	
	&DDRB,
	&DDRB,
	&DDRE,
	&DDRE,
	&DDRE,
	
	&DDRE,
	&DDRE,
	&DDRE,
	&DDRE,
	&DDRE,
};

uint8_t pins[30] = {
	_BV(3),
	_BV(6),
	_BV(7),
	_BV(1),
	_BV(2),
	_BV(5),
	_BV(3),
	_BV(2),
	_BV(1),
	_BV(0),
	_BV(1),
	_BV(0),
	_BV(7),
	_BV(6),
	_BV(5),
	_BV(4),
	_BV(3),
	_BV(7),
	_BV(6),
	_BV(5),
	_BV(4),
	_BV(0),
	_BV(7),
	_BV(6),
	_BV(5),
	_BV(4),
	_BV(3),
	_BV(2),
	_BV(1),
	_BV(0),
};