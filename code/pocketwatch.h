/*
LED Pocket Watch
http://www.frank-zhao.com/ledpocketwatch/

Copyright (C) 2012 Frank Zhao

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef pocketwatch_h
#define pocketwatch_h

#include <util/delay.h>
#include <avr/io.h>

// pin definitions
#define PORTx_BUTTON1 PORTB
#define DDRx_BUTTON1 DDRB
#define PINx_BUTTON1 PINB
#define PIN_BUTTON1 3
#define PORTx_BUTTON2 PORTB
#define DDRx_BUTTON2 DDRB
#define PINx_BUTTON2 PINB
#define PIN_BUTTON2 1
#define PORTx_LOWBATTDET PORTA
#define DDRx_LOWBATTDET DDRA
#define PINx_LOWBATTDET PINA
#define PIN_LOWBATTDET 3
#define PORTx_BUZZER PORTA
#define DDRx_BUZZER DDRA
#define PINx_BUZZER PINA
#define PIN_BUZZER 4
#define PORTx_MOTOR PORTA
#define DDRx_MOTOR DDRA
#define PINx_MOTOR PINA
#define PIN_MOTOR 6
#define PORTx_RINGS PORTF
#define DDRx_RINGS DDRF
#define PIN_RING_HOUR 2
#define PIN_RING_MINUTE_EVEN 5
#define PIN_RING_MINUTE_ODD 4
#define PIN_RING_SECOND_EVEN 1
#define PIN_RING_SECOND_ODD 0

// useful macros
#define button1_is_down() bit_is_clear(PINx_BUTTON1, PIN_BUTTON1)
#define button1_is_up() bit_is_set(PINx_BUTTON1, PIN_BUTTON1)
#define button2_is_down() bit_is_clear(PINx_BUTTON2, PIN_BUTTON2)
#define button2_is_up() bit_is_set(PINx_BUTTON2, PIN_BUTTON2)
#define battery_is_low() bit_is_clear(PORTA, PIN_LOWBATTDET)

#define debounce() _delay_us(100)
#define HOLD_CNT_THRESH (10 * 1000 / 50)
#define TIMEOUT_THRESH (10 * 1000 / 50)

extern uint16_t portx[30];
extern uint16_t ddrx[30];
extern uint8_t pins[30];

typedef enum
{
	MODE_SLEEPING = 0,
	MODE_CURTIME_SHOWING,
	MODE_SETTIME_HOUR,
	MODE_SETTIME_MINUTE,
	MODE_SETALARM_HOUR,
	MODE_SETALARM_MINUTE,
	MODE_SETALARM_ONOFF,
	MODE_ALARMING,
} mode_t;

#endif /* POCKETWATCH_H_ */