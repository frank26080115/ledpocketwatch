/*
LED Pocket Watch
http://www.frank-zhao.com/ledpocketwatch/

Copyright (C) 2012 Frank Zhao

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "pins.h"
#include "LEDs.h"
#include "PocketWatch.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <util/delay.h>

volatile uint8_t cur_hour;
volatile uint8_t cur_minute;
volatile uint8_t cur_second;

volatile uint8_t alarm_hour;
volatile uint8_t alarm_minute;
volatile uint8_t alarm_already_off;
volatile uint8_t alarm_active;
volatile uint8_t alarm_enabled;

volatile int8_t display_second;
volatile int8_t display_minute;
volatile int8_t display_hour;

volatile uint8_t to_sleep = 0;
volatile mode_t mode;

ISR(TIMER2_OVF_vect)
{
	// one second event, add to time
	cur_second++;
	if (cur_second >= 60)
	{
		cur_second = 0;
		cur_minute++;
		if (cur_minute >= 60)
		{
			cur_minute = 0;
			cur_hour++;
			if (cur_hour >= 24)
			{
				cur_hour = 0;
			}
		}
	}
	
	if (battery_is_low() == 0 && mode == MODE_CURTIME_SHOWING)
	{
		motor_on();
		TIMSK2 |= _BV(OCIE2A); // set timer to turn off the motor after 1/8th of a second
	}
	
	// check if alarm should go off
	if (alarm_enabled != 0 && alarm_hour == cur_hour && alarm_minute == cur_minute)
	{
		if (alarm_already_off == 0)
		{
			alarm_active = 1;
			alarm_already_off = 1;
			buzzer_on();
			if (battery_is_low() == 0)
			{
				motor_on();
				TIMSK2 &= ~_BV(OCIE2A); // do not turn off motor automatically
			}				
			
			if (mode == MODE_WAS_SLEEPING)
			{
				mode = MODE_ALARMING;
				to_sleep = 0;
			}
		}
	}
	else
	{
		alarm_already_off = 0;
	}
	
	if (mode == MODE_WAS_SLEEPING)
	{
		// should go back to sleep, nothing to do
		to_sleep = 1;
	}
}

ISR(TIMER2_COMP_vect)
{
	// if enabled, this vector happens 1/8th of a second after the overflow interrupt
	TIMSK2 &= ~_BV(OCIE2A); // turn off this interrupt
	motor_off();
	
	if (mode == MODE_WAS_SLEEPING)
	{
		// should go back to sleep, nothing to do
		clear_leds();
		to_sleep = 1;
	}
}

volatile uint8_t ovf_cnt = 0;

ISR(TIMER0_OVF_vect)
{
	// 2.05 KHz event
	
	if (alarm_active || mode == MODE_TESTING)
	{
		// buzz the alarm buzzer, set pin high, use the compare match to set pin low for 4.1 KHz
		buzzer_high();
	}
	
	if (mode != MODE_WAS_SLEEPING && mode != MODE_ALARMING)
	{
		// display LEDs if not sleeping

		uint8_t which_ring = ovf_cnt % 3; // each ring will only get 1/3 duty cycle
		
		if (which_ring == 0) // hour
		{
			set_hour_led(display_hour);
		}
		else if (which_ring == 1) // minute
		{
			set_minute_led(display_minute);
		}
		else if (which_ring == 2) // second
		{
			set_second_led(display_second);
		}
		
		if (ovf_cnt >= 3*58) // roll-over on multiple of 3 but under 255
		{
			ovf_cnt = 0;
		}
		else
		{
			ovf_cnt++;
		}
	}
	else
	{
		// should go back to sleep, nothing to do
		clear_leds();
		to_sleep = 1;
	}
}

ISR(TIMER0_COMP_vect)
{
	if (alarm_active || mode == MODE_TESTING)
	{
		// toggle the buzzer pin again, this makes the buzzer frequency about 4.1 KHz
		buzzer_low();
		
		
	}
	else if (mode == MODE_WAS_SLEEPING)
	{
		// should go back to sleep, nothing to do
		clear_leds();
		to_sleep = 1;
	}
}

ISR(PCINT1_vect)
{
	// at this point, the processor has woken up
	
	// turn off alarm
	alarm_active = 0;
	
	buzzer_off();
	motor_off();
}

void timer0_enable()
{
	TCCR0A = _BV(CS00); // start timer with clk div 1
	OCR0A = 0x80; // setup compare match at 50% duty cycle
	TIMSK0 = _BV(TOIE0) | _BV(OCIE0A); // enable overflow and compare match interrupts
}

void timer0_disable()
{
	TIMSK0 = 0; // disable the interrupts
	TCCR0A = 0; // stop the timer
	TCNT0 = 0;
}

int main()
{
#ifdef USE_EEPROM
	// note, addresses must match the stored addresses used later on
	
	// read in time from eeprom, to keep things interesting during development
	cur_second = eeprom_read_byte(0) % 60;
	cur_minute = eeprom_read_byte(1) % 60;
	cur_hour   = eeprom_read_byte(2) % 24;
	
	// read in alarm settings from eeprom
	alarm_minute  = eeprom_read_byte(3) % 60;
	alarm_hour    = eeprom_read_byte(4) % 24;
	alarm_enabled = eeprom_read_byte(5);
#endif
	
	// setup inputs and outputs
	DDRx_BUTTON1 &= ~_BV(PIN_BUTTON1);
	DDRx_BUTTON2 &= ~_BV(PIN_BUTTON2);
	PORTx_BUTTON1 |= _BV(PIN_BUTTON1);
	PORTx_BUTTON2 |= _BV(PIN_BUTTON2);
	PORTx_BUZZER |= _BV(PIN_BUZZER);
	DDRx_BUZZER &= ~_BV(PIN_BUZZER);
	PORTx_LOWBATTDET |= _BV(PIN_LOWBATTDET);
	DDRx_LOWBATTDET &= ~_BV(PIN_LOWBATTDET);
	for (uint8_t i = 0; i < 30; i++)
	{
		uint8_t* p = ports[i]; // PORTx register
		*p &= ~pins[i]; // turn off
		p++; // corresponding DDRx register
		*p |= pins[i]; // set as output
	}
	PORTx_RINGS = PORTx_RINGS | _BV(PIN_RING_SECOND_ODD) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD); // turn off
	DDRx_RINGS =  DDRx_RINGS  | _BV(PIN_RING_SECOND_ODD) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD); // set output
	last_led_port = ports[0];
	last_led_pin = pins[0];
	
	// setup async timer 2
	TCNT2 = 0; // reset it
	ASSR = _BV(AS2); // enable async xtal input
	TCCR2A = _BV(CS22) | 0 | _BV(CS20); // start timer with clock div 128
	TIMSK2 = _BV(TOIE2); // enable overflow interrupt
	OCR2A = MOTOR_PULSE_LENGTH; // this means the motor will pulse for 1/8th of a second
	
	// setup button interrupts
	PCMSK1 |= _BV(PCINT11) | _BV(PCINT9);
	EIMSK |= _BV(PCIE1);
	
	// setup power saving features
	power_all_disable(); // PRR = 0x0F; // disable all (ADC, USART, SPI, TMR1)
	SMCR = _BV(SM1) | _BV(SM0); // set power-save mode
	
	sei(); // enable global interrupts
	
	if (button1_is_down())
	{
		mode = MODE_TESTING; // hold button at reset to enter test mode
	}		
	else
	{
		_delay_ms(10); // delay to guarantee TCNT2 to have a small value
		if (TCNT2 == 0) mode = MODE_TESTING; // this means the oscillator is disconnected
	}
	
	uint32_t held_cnt = 0;
	uint32_t timeout_cnt = 0;
	uint32_t animation_cnt = 0;
	char button_was_down = 0;
	
	mode = MODE_WAS_SLEEPING;
	to_sleep = 1;
	
	for (;;)
	{
		if (mode == MODE_ALARMING || mode == MODE_TESTING || button2_is_down() || button1_is_down()) timer0_enable();
		
		if (mode == MODE_WAS_SLEEPING || mode == MODE_ALARMING)
		{
			if (button2_is_down())
			{
				// go directly into settings mode
				mode = MODE_SETALARM_HOUR;
				to_sleep = 0;
				
				display_second = -1;
				display_minute = -1;
				display_hour = -1;
				
				debounce();
				
				while (button2_is_down())
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 0 : -1;
					
					// indicate AM or PM
					display_minute = (((cur_hour >= 12) ? 58 : 28) + (cur_second % 5)) % 60;
					
					// indicate current settings
					display_hour = cur_hour;
				}
				
				debounce();
			}
			else if (button1_is_down())
			{
				// go into time display mode
				mode = MODE_CURTIME_ENTERING;
				animation_cnt = 0;
				
				display_second = -1;
				display_minute = -1;
				display_hour = -1;
				
				to_sleep = 0;
				
				debounce();
				held_cnt = 0;
			}
			else if (mode != MODE_ALARMING)
			{
				to_sleep = 1;
			}
		}
		else if (mode == MODE_CURTIME_ENTERING)
		{
			if (battery_is_low())
			{
				mode = MODE_CURTIME_SHOWING; // save power by ignoring animation
			}
			else
			{
				// show a nice startup animation
				
				timeout_cnt = 0; // no timeout during animation
				
				if (button1_is_down())
				{
					held_cnt = (held_cnt > HOLD_CNT_THRESH) ? held_cnt : (held_cnt + 1);
					to_sleep = 0;
				}
				else
				{
					held_cnt = 0;
				}
				
				// move the LEDs if it's time to do so according to the animation speed
				if (animation_cnt == 0)
				{
					if (display_second < cur_second) display_second = (display_second + 1) % 60;
					if (display_minute < cur_minute) display_minute = (display_minute + 1) % 60;
					if (display_hour < cur_hour)     display_hour =   (display_hour + 1) % 24;
				}				
				
				// the animation finishes when the displayed time matches the current time
				if (display_second == cur_second && display_minute == cur_minute && display_hour == cur_hour) mode = MODE_CURTIME_SHOWING;
				
				animation_cnt = (animation_cnt + 1) % ANIMATION_SPEED; // this controls the animation speed
			}
		}
		else if (mode == MODE_CURTIME_SHOWING)
		{
			// indicate current time
			display_minute = cur_minute;
			display_hour = cur_hour;
			
			if (battery_is_low())
			{
				// save some power by not showing seconds
				// this also indicates low battery status to the user
				display_second = -1;
				
			}
			else
			{
				display_second = cur_second;
			}
			
			debounce(); // this delay both debounces and also helps with pacing the counters
			
			if (button1_is_down())
			{
				held_cnt = (held_cnt > HOLD_CNT_THRESH) ? held_cnt : (held_cnt + 1);
				timeout_cnt = 0;
				to_sleep = 0;
			}
			else
			{
				if (battery_is_low())
				{
					// save power if battery is low, go directly to sleep
					mode = MODE_WAS_SLEEPING;
					to_sleep = 1;
				}
				else if (held_cnt >= HOLD_CNT_THRESH)
				{
					// if user holds down the button for longer than 5 seconds
					mode = MODE_CURTIME_EXITING;
					animation_cnt = 0;
				}
				else
				{
					timeout_cnt = (timeout_cnt > TIMEOUT_THRESH) ? timeout_cnt : (timeout_cnt + 1);
					if (timeout_cnt >= TIMEOUT_THRESH)
					{
						// button has been release for a sufficiently long enough time to sleep
						mode = MODE_CURTIME_EXITING;
						animation_cnt = 0;
					}
					else
					{
						// did not timeout yet, do not sleep
						to_sleep = 0;
					}
				}
				
				// no longer held down
				held_cnt = 0;
			}
		}
		else if (mode == MODE_CURTIME_EXITING)
		{
			if (battery_is_low())
			{
				// save power by ignoring animation
				mode = MODE_WAS_SLEEPING;
				to_sleep = 1;
			}
			else
			{
				// show a nice exit animation
				
				timeout_cnt = 0; // no timeout during animation
				
				if (button1_is_down())
				{
					held_cnt = (held_cnt > HOLD_CNT_THRESH) ? held_cnt : (held_cnt + 1);
					to_sleep = 0;
					mode = MODE_CURTIME_ENTERING; // button is down, so undo the animation
				}
				else
				{
					held_cnt = 0;
				}
				
				// move the LEDs if it's time to do so according to the animation speed
				if (animation_cnt == 0)
				{
					if (display_second >= 0) display_second++;
					if (display_minute >= 0) display_minute++;
					if (display_hour >= 0) display_hour++;
					
					// the animation finishes when the displayed time ends up at the top of the clock
					if (display_second >= 60) display_second = -1;
					if (display_minute >= 60) display_minute = -1;
					if (display_hour == 12 || display_hour >= 24) display_hour = -1;
				}
				
				animation_cnt = (animation_cnt + 1) % ANIMATION_SPEED; // this controls the animation speed
				
				// go to sleep once animation is finished
				if (display_second < 0 && display_minute < 0 && display_hour < 0)
				{
					mode = MODE_WAS_SLEEPING;
					to_sleep = 1;
				}					
			}
		}
		else if (mode == MODE_SETTIME_HOUR)
		{
			// indicate mode
			display_second = bit_is_set(cur_second, 0) ? 0 : -1;
			
			// indicate AM or PM
			display_minute = (((cur_hour >= 12) ? 58 : 28) + (cur_second % 5)) % 60;
			
			// indicate current settings
			display_hour = cur_hour;
			
			if (button1_is_down())
			{
				// change
				cur_hour = (cur_hour + 1) % 24;
				display_hour = cur_hour;
				
				// indicate AM or PM
				display_minute = (((cur_hour >= 12) ? 58 : 28) + (cur_second % 5)) % 60;
				
				debounce();
				
				for (uint8_t i = 0; i < (button_was_down == 0 ? 10 : 5) && button1_is_down(); i++)
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 0 : -1;
					
					// indicate AM or PM
					display_minute = (((cur_hour >= 12) ? 58 : 28) + (cur_second % 5)) % 60;
					
					// indicate current settings
					display_hour = cur_hour;
					
					debounce();
				}
				
				button_was_down = 1;
				
				if (button1_is_up())
				{
					button_was_down = 0;
					debounce();
				}
			}
			else if (button1_is_up())
			{
				if (button_was_down != 0)
				{
					debounce();
				}
				
				button_was_down = 0;
			}
			
			if (button2_is_down())
			{
				// indicate mode
				display_second = bit_is_set(cur_second, 0) ? 5 : -1;
				
				// indicate current settings
				display_minute = cur_minute;
				display_hour = -1;
				
				// goto next mode
				mode = MODE_SETTIME_MINUTE;
				to_sleep = 0;
				
				debounce();
				
				while (button2_is_down())
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 5 : -1;
					
					// indicate current settings
					display_minute = cur_minute;
				}
				
				debounce();
			}
		}
		else if (mode == MODE_SETTIME_MINUTE)
		{
			// indicate mode
			display_second = bit_is_set(cur_second, 0) ? 5 : -1;
			
			// turn off
			display_hour = -1;
			
			// show current settings
			display_minute = cur_minute;
			
			if (button1_is_down())
			{
				// change
				cur_minute = (cur_minute + 1) % 60;
				display_minute = cur_minute;
				
				debounce();
				
				for (uint8_t i = 0; i < (button_was_down == 0 ? 10 : 5) && button1_is_down(); i++)
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 5 : -1;
					
					// show current settings
					display_minute = cur_minute;
					debounce();
				}
				
				button_was_down = 1;
				
				if (button1_is_up())
				{
					button_was_down = 0;
					debounce();
				}
			}
			else if (button1_is_up())
			{
				if (button_was_down != 0)
				{
					debounce();
				}
				
				button_was_down = 0;
			}
			
			if (button2_is_down())
			{
				// clear display
				display_second = -1;
				display_minute = -1;
				display_hour = -1;
				
				if (alarm_enabled == 0)
				{
					// skip alarm time set modes, instead go to enable/disable alarm mode
					mode = MODE_SETALARM_ONOFF;
					to_sleep = 0;
					debounce();
					while (button2_is_down())
					{
						// indicate mode
						display_second = bit_is_set(cur_second, 0) ? 20 : -1;
						
						// indicate alarm enabled status
						display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
					}
					debounce();
				}
				else
				{
					// goto alarm time set mode
					mode = MODE_SETALARM_HOUR;
					to_sleep = 0;
					debounce();
					while (button2_is_down())
					{
						// indicate mode
						display_second = bit_is_set(cur_second, 0) ? 10 : -1;
						
						// indicate alarm enabled status
						display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
					}
					debounce();
				}
			}
		}
		else if (mode == MODE_SETALARM_HOUR)
		{
			// indicate mode
			display_second = bit_is_set(cur_second, 0) ? 10 : -1;
			
			// indicate AM or PM
			display_minute = (((alarm_hour >= 12) ? 58 : 28) + (cur_second % 5)) % 60;
			
			// indicate current settings
			display_hour = alarm_hour;
			
			if (button1_is_down())
			{
				// change
				alarm_hour = (alarm_hour + 1) % 24;
				display_hour = alarm_hour;
				
				// indicate AM or PM
				display_minute = (((alarm_hour >= 12) ? 58 : 28) + (cur_second % 5)) % 60;
				
				debounce();
				
				for (uint8_t i = 0; i < (button_was_down == 0 ? 10 : 5) && button1_is_down(); i++)
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 10 : -1;
					
					// indicate AM or PM
					display_minute = (((alarm_hour >= 12) ? 58 : 28) + (cur_second % 5)) % 60;
					
					// indicate current settings
					display_hour = alarm_hour;
					
					debounce();
				}
				
				button_was_down = 1;
				
				if (button1_is_up())
				{
					button_was_down = 0;
					debounce();
				}
			}
			else if (button1_is_up())
			{
				if (button_was_down != 0)
				{
					debounce();
				}
				
				button_was_down = 0;
			}
			
			if (button2_is_down())
			{
				// indicate mode
				display_second = bit_is_set(cur_second, 0) ? 15 : -1;
				
				// indicate current settings
				display_minute = alarm_minute;
				display_hour = -1;
				
				// goto next mode
				mode = MODE_SETALARM_MINUTE;
				to_sleep = 0;
				
				debounce();
				
				while (button2_is_down())
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 15 : -1;
					
					// indicate current settings
					display_minute = cur_minute;
				}
				
				debounce();
			}
		}
		else if (mode == MODE_SETALARM_MINUTE)
		{
			// indicate mode
			display_second = bit_is_set(cur_second, 0) ? 15 : -1;
			
			// turn off
			display_hour = -1;
			
			// show current settings
			display_minute = alarm_minute;
			
			if (button1_is_down())
			{
				// change
				alarm_minute = (alarm_minute + 1) % 60;
				display_minute = alarm_minute;
				
				debounce();
				
				for (uint8_t i = 0; i < (button_was_down == 0 ? 10 : 5) && button1_is_down(); i++)
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 15 : -1;
					
					// show current settings
					display_minute = alarm_minute;
					debounce();
				}
				
				button_was_down = 1;
				
				if (button1_is_up())
				{
					button_was_down = 0;
					debounce();
				}
			}
			else if (button1_is_up())
			{
				if (button_was_down != 0)
				{
					debounce();
				}
				
				button_was_down = 0;
			}
			
			if (button2_is_down())
			{
				// clear display
				display_second = -1;
				display_minute = -1;
				display_hour = -1;
				
				// goto next mode
				mode = MODE_SETALARM_ONOFF;
				to_sleep = 0;
				debounce();
				while (button2_is_down())
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 20 : -1;
					
					// indicate alarm enabled status
					display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
				}
				debounce();
			}
		}
		else if (mode == MODE_SETALARM_ONOFF)
		{
			// indicate mode
			display_second = bit_is_set(cur_second, 0) ? 20 : -1;
			
			display_minute = -1; // turn off
			
			// indicate alarm enabled status
			display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
			
			if (button1_is_down())
			{
				alarm_enabled = alarm_enabled == 0 ? 1 : 0; // change
				display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
				debounce();
				while (button1_is_down())
				{
					// indicate mode
					display_second = bit_is_set(cur_second, 0) ? 20 : -1;
					
					// indicate alarm enabled status
					display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
				}
				debounce();
			}
			
			if (button2_is_down())
			{
				// go to sleep
				display_second = -1;
				display_minute = -1;
				display_hour = -1;
				mode = MODE_WAS_SLEEPING;
				to_sleep = 1;
				debounce();
			}
		}
		else if (mode == MODE_TESTING)
		{
			// flash all LED, the buzzer will buzz, the motor will run for half second intervals
			
			display_second = (display_second + 1) % 60;
			display_minute = (display_minute + 1) % 60;
			display_hour   = (display_hour + 1) % 24;
			
			if (display_second < 5)
			{
				motor_on();
			}
			else
			{
				motor_off();
			}
			
			_delay_ms(100);
			
			// exit test mode if able
			if (button1_is_down() && TCNT2 != 0)
			{
				mode = MODE_CURTIME_ENTERING;
				animation_cnt = 0;
			}
		}
		
		if (to_sleep)
		{
			// nothing to do, disable LEDs and go to sleep
			display_second = -1;
			display_minute = -1;
			display_hour = -1;
			mode = MODE_WAS_SLEEPING;

#ifdef USE_EEPROM
			// save data if required
			eeprom_update_byte(0, cur_second);
			eeprom_update_byte(1, cur_minute);
			eeprom_update_byte(2, cur_hour);
			eeprom_update_byte(3, alarm_minute);
			eeprom_update_byte(4, alarm_hour);
			eeprom_update_byte(5, alarm_enabled);
#endif
			
			if (alarm_active == 0)
			{
				// disable unnecessary interrupts
				TIMSK2 &= ~_BV(OCR2A);				
				timer0_disable();
				
				// make sure things are off
				buzzer_off();
				motor_off();
				
				sleep_enable(); // SMCR |= _BV(SE);
				sleep_cpu();
				
				// whatever interrupt happens, will happen here
				
				sleep_disable(); // SMCR &= ~_BV(SE); // prevent accidental sleep
			}
		}
	}
	
	return 0;
}