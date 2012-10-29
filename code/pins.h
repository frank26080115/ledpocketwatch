/*
LED Pocket Watch
http://www.frank-zhao.com/ledpocketwatch/

Copyright (C) 2012 Frank Zhao

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef pins_h
#define pins_h

#include <avr/io.h>

// pin definitions for LEDs, according to the pocket watch schematic
extern uint16_t ports[30];
extern uint8_t pins[30];

// pin definitions for all other pins
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
#define button1_is_up()   bit_is_set(PINx_BUTTON1, PIN_BUTTON1)
#define button2_is_down() bit_is_clear(PINx_BUTTON2, PIN_BUTTON2)
#define button2_is_up()   bit_is_set(PINx_BUTTON2, PIN_BUTTON2)
#define battery_is_low()  bit_is_clear(PORTA, 3)
#define buzzer_on()       do { DDRx_BUZZER  |=  _BV(PIN_BUZZER); } while (0)
#define buzzer_off()      do { PORTx_BUZZER |=  _BV(PIN_BUZZER); DDRx_BUZZER &= ~_BV(PIN_BUZZER); } while (0) // disable the output on buzzer but leave pull-up resistor on to save power
#define buzzer_high()     do { PORTx_BUZZER |=  _BV(PIN_BUZZER); } while (0)
#define buzzer_low()      do { PORTx_BUZZER &= ~_BV(PIN_BUZZER); } while (0)
#define motor_on()        do { PORTx_MOTOR  |=  _BV(PIN_MOTOR);  DDRx_MOTOR  |=  _BV(PIN_MOTOR); } while (0)
#define motor_off()       do { PORTx_MOTOR  &= ~_BV(PIN_MOTOR);  DDRx_MOTOR  &= ~_BV(PIN_MOTOR); } while (0) // depend on external pull-down resistor

#endif