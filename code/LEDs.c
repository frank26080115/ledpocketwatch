/*
LED Pocket Watch
http://www.frank-zhao.com/ledpocketwatch/

Copyright (C) 2012 Frank Zhao

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pins.h"
#include "PocketWatch.h"

// remember the last LED being lit so it can be turned off efficiently
volatile uint8_t* last_led_port;
volatile uint8_t last_led_pin;

void set_hour_led(int8_t number)
{
	volatile uint8_t idx;
	
	// turn off last LED
	*last_led_port &= ~last_led_pin;
	
	if (number >= 0)
	{
		// put the ring low
		PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_HOUR)) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_SECOND_ODD);

		// calculate appropriate index
		idx = (number % 12) * 5;
		idx /= 2;

		// set the LED on and remember which one
		last_led_port = ports[idx];
		last_led_pin = pins[idx];
		*last_led_port |= last_led_pin;
	}
}

void set_minute_led(int8_t number)
{
	volatile uint8_t idx;
	
	// turn off last LED
	*last_led_port &= ~last_led_pin;
	
	if (number >= 0)
	{
		// put the ring low, depending on which one is needed
		if (bit_is_clear(number, 0))
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_MINUTE_EVEN)) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_MINUTE_ODD)  | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_SECOND_ODD);
		}
		else
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_MINUTE_ODD))  | _BV(PIN_RING_HOUR) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_SECOND_ODD);
		}
		
		// calculate appropriate index
		idx = number % 60;
		idx /= 2;
		
		// set the LED on and remember which one
		last_led_port = ports[idx];
		last_led_pin = pins[idx];
		*last_led_port |= last_led_pin;
	}
}

void set_second_led(int8_t number)
{
	volatile uint8_t idx;
	
	// turn off last LED
	*last_led_port &= ~last_led_pin;
	
	if (number >= 0)
	{
		// put the ring low, depending on which one is needed
		if (bit_is_clear(number, 0))
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_SECOND_EVEN)) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_ODD)  | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
		}
		else
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_SECOND_ODD))  | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
		}
		
		// calculate appropriate index
		idx = number % 60;
		idx /= 2;
		
		// set the LED on and remember which one
		last_led_port = ports[idx];
		last_led_pin = pins[idx];
		*last_led_port |= last_led_pin;
	}
}

void clear_leds()
{
	// turn off all the LEDs
	*last_led_port &= ~last_led_pin;
	PORTx_RINGS = PORTx_RINGS | _BV(PIN_RING_SECOND_ODD) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
}