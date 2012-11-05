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