#include "pins.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <util/delay.h>

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
#define battery_is_low() bit_is_clear(PORTA, 3)
#define debounce() _delay_us(100)
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
volatile uint8_t mode = 0;

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
	
	// check if alarm should go off
	if (alarm_enabled != 0 && alarm_hour == cur_hour && alarm_minute == cur_minute)
	{
		if (alarm_already_off == 0)
		{
			alarm_active = 1;
			alarm_already_off = 1;
			DDRx_BUZZER |= _BV(PIN_BUZZER);
			if (mode == 0)
			{
				mode = 7;
				to_sleep = 0;
			}
		}
	}
	else
	{
		alarm_already_off = 0;
	}
	
	if (mode == 0)
	{
		// should go back to sleep, nothing to do
		to_sleep = 1;
	}
}

volatile uint8_t ovf_cnt = 0;

ISR(TIMER0_OVF_vect)
{
	// 2.05 KHz event
	
	if (alarm_active)
	{
		// buzz the alarm buzzer, set pin high, use the compare match to set pin low for 4.1 KHz
		PORTx_BUZZER |= _BV(PIN_BUZZER);
		if (battery_is_low() == 0) PORTx_MOTOR |= _BV(PIN_MOTOR);
	}
	else
	{
		PORTx_MOTOR &= ~_BV(PIN_MOTOR);
	}
	
	if (mode != 0 && mode != 7)
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
		
		if (ovf_cnt >= 125)
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
	if (alarm_active)
	{
		// toggle the buzzer pin again, this makes the buzzer frequency about 4.1 KHz
		PORTx_BUZZER &= ~_BV(PIN_BUZZER);
	}
	else if (mode == 0)
	{
		// should go back to sleep, nothing to do
		clear_leds();
		to_sleep = 1;
	}
}

ISR(PCINT1_vect)
{
	// turn off alarm
	alarm_active = 0;
	
	// disable the output on buzzer but leave pull-up resistor on to save power
	PORTx_BUZZER |= _BV(PIN_BUZZER);
	DDRx_BUZZER &= ~_BV(PIN_BUZZER);
	
	// at this point, the processor has woken up
}

// remember the last LED being lit so it can be turned off efficiently
volatile uint8_t* last_led_port;
volatile uint8_t* last_led_ddr;
volatile uint8_t last_led_pin;

void set_hour_led(int8_t number)
{
	volatile uint8_t idx;
	
	// turn off last LED
	*last_led_port &= ~last_led_pin;
	*last_led_ddr &= ~last_led_pin;
	
	if (number >= 0)
	{
		// put the ring low
		PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_HOUR)) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_SECOND_ODD);

		// calculate appropriate index
		idx = (number % 12) * 5;
		idx /= 2;

		// set the LED on and remember which one
		last_led_port = portx[idx];
		last_led_ddr = ddrx[idx];
		last_led_pin = pins[idx];
		*last_led_port |= last_led_pin;
		*last_led_ddr |= last_led_pin;
	}
}

void set_minute_led(int8_t number)
{
	volatile uint8_t idx;
	
	// turn off last LED
	*last_led_port &= ~last_led_pin;
	*last_led_ddr &= ~last_led_pin;
	
	if (number >= 0)
	{
		// put the ring low, depending on which one is needed
		if (bit_is_clear(number, 0))
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_MINUTE_EVEN)) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_MINUTE_ODD) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_SECOND_ODD);
		}
		else
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_MINUTE_ODD)) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_SECOND_ODD);
		}
		
		// calculate appropriate index
		idx = number % 60;
		idx /= 2;
		
		// set the LED on and remember which one
		last_led_port = portx[idx];
		last_led_ddr = ddrx[idx];
		last_led_pin = pins[idx];
		*last_led_port |= last_led_pin;
		*last_led_ddr |= last_led_pin;
	}
}

void set_second_led(int8_t number)
{
	volatile uint8_t idx;
	
	// turn off last LED
	*last_led_port &= ~last_led_pin;
	*last_led_ddr &= ~last_led_pin;
	
	if (number >= 0)
	{
		// put the ring low, depending on which one is needed
		if (bit_is_clear(number, 0))
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_SECOND_EVEN)) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_ODD) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
		}
		else
		{
			PORTx_RINGS = PORTx_RINGS & (~_BV(PIN_RING_SECOND_ODD)) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
		}
		
		// calculate appropriate index
		idx = number % 60;
		idx /= 2;
		
		// set the LED on and remember which one
		last_led_port = portx[idx];
		last_led_ddr = ddrx[idx];
		last_led_pin = pins[idx];
		*last_led_port |= last_led_pin;
		*last_led_ddr |= last_led_pin;
	}
}

void clear_leds()
{
	// turn off all the LEDs
	*last_led_port &= ~last_led_pin;
	*last_led_ddr &= ~last_led_pin;
	PORTx_RINGS = PORTx_RINGS | _BV(PIN_RING_SECOND_ODD) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
}

int main()
{
	// read in time from eeprom, to keep things interesting during development
	cur_second = eeprom_read_byte(0) % 60;
	cur_minute = eeprom_read_byte(1) % 60;
	cur_hour = eeprom_read_byte(2) % 24;
	
	// read in alarm settings from eeprom
	alarm_minute = eeprom_read_byte(3) % 60;
	alarm_hour = eeprom_read_byte(4) % 24;
	alarm_enabled = eeprom_read_byte(5);
	
	// ground all grounded pins
	DDRA  = 0x00;
	DDRB  = 0x00;
	DDRC  = 0x00;
	DDRD  = 0x00;
	DDRE  = 0x00;
	DDRG  = 0x00;
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	PORTE = 0x00;
	PORTG = 0x00;
	
	// setup pins
	DDRx_RINGS  = _BV(PIN_RING_SECOND_ODD) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
	PORTx_RINGS = _BV(PIN_RING_SECOND_ODD) | _BV(PIN_RING_HOUR) | _BV(PIN_RING_SECOND_EVEN) | _BV(PIN_RING_MINUTE_EVEN) | _BV(PIN_RING_MINUTE_ODD);
	
	// setup inputs and outputs
	DDRx_BUTTON1 &= ~_BV(PIN_BUTTON1);
	DDRx_BUTTON2 &= ~_BV(PIN_BUTTON2);
	PORTx_BUTTON1 |= _BV(PIN_BUTTON1);
	PORTx_BUTTON2 |= _BV(PIN_BUTTON2);
	PORTx_BUZZER |= _BV(PIN_BUZZER);
	DDRx_BUZZER &= ~_BV(PIN_BUZZER);
	DDRx_MOTOR |= _BV(PIN_MOTOR);
	PORTx_LOWBATTDET |= _BV(PIN_LOWBATTDET);
	DDRx_LOWBATTDET &= ~_BV(PIN_LOWBATTDET);
	for (uint8_t i = 0; i < 30; i++)
	{
		uint8_t* p = portx[i];
		*p &= ~pins[i];
		uint8_t* d = ddrx[i];
		*d &= ~pins[i];
	}
	last_led_port = portx[0];
	last_led_ddr = ddrx[0];
	last_led_pin = pins[0];
	
	// setup async timer 2
	ASSR = _BV(AS2); // enable async xtal input
	TCCR2A = _BV(CS22) | 0 | _BV(CS20); // start timer with clock div 128
	TIMSK2 = _BV(TOIE2); // enable overflow interrupt
	
	// setup timer 0
	TCCR0A = _BV(CS00); // start timer with clk div 1
	OCR0A = 0x80; // setup compare match at 50% duty cycle
	TIMSK0 = _BV(TOIE0) | _BV(OCIE0A); // enable overflow and compare match interrupts
	
	// setup button interrupts
	PCMSK1 |= _BV(PCINT11) | _BV(PCINT9);
	EIMSK |= _BV(PCIE1);
	
	// setup power saving features
	power_all_disable(); // PRR = 0x0F; // disable all (ADC, USART, SPI, TMR1)
	SMCR = _BV(SM1) | _BV(SM0); // set power-save mode
	
	sei(); // enable global interrupts
	
	uint32_t held_cnt = 0;
	uint32_t timeout_cnt = 0;
	uint8_t button_was_down = 0;
	uint8_t eeprom_is_dirty = 0;
	
	mode = 0;
	to_sleep = 1;
	
	for (;;)
	{
		if (mode == 0 || mode == 7)
		{
			if (button2_is_down())
			{
				// go directly into settings mode
				mode = 2;
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
				mode = 1;
				to_sleep = 0;
				
				debounce();
				held_cnt = 0;
			}
			else if (mode != 7)
			{
				to_sleep = 1;
			}
		}
		else if (mode == 1)
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
				held_cnt = (held_cnt > 10 * 1000 / 50) ? held_cnt : (held_cnt + 1);
				timeout_cnt = 0;
				to_sleep = 0;
			}
			else
			{
				if (battery_is_low() || held_cnt >= 10 * 1000 / 50)
				{
					// save power if battery is low
					// or if user holds down the button for longer than 5 seconds
					mode = 0;
					to_sleep = 1;
				}
				else
				{
					timeout_cnt = (timeout_cnt > 10 * 1000 / 50) ? timeout_cnt : (timeout_cnt + 1);
					if (timeout_cnt >= 10 * 1000 / 50)
					{
						// button has been release for a sufficiently long enough time to sleep
						mode = 0;
						to_sleep = 1;
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
		else if (mode == 2)
		{
			// set current hour
			
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
				
				eeprom_is_dirty = 1;
				
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
				
				eeprom_is_dirty = 1;
				
				// goto next mode
				mode = 3;
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
		else if (mode == 3)
		{
			// set current minute
			
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
				
				eeprom_is_dirty = 1;
				
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
					mode = 6;
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
					mode = 4;
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
		else if (mode == 4)
		{
			// set alarm hour
			
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
				
				eeprom_is_dirty = 1;
				
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
				mode = 5;
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
		else if (mode == 5)
		{
			// set alarm minute
			
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
				
				eeprom_is_dirty = 1;
				
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
				mode = 6;
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
		else if (mode == 6)
		{
			// enable/disable alarm
			
			// indicate mode
			display_second = bit_is_set(cur_second, 0) ? 20 : -1;
			
			display_minute = -1; // turn off
			
			// indicate alarm enabled status
			display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
			
			if (button1_is_down())
			{
				alarm_enabled = alarm_enabled == 0 ? 1 : 0; // change
				display_hour = (alarm_enabled != 0) ? (5 + (cur_second % 3)) : ((11 + (cur_second % 3)) % 12);
				eeprom_is_dirty = 1;
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
				mode = 0;
				to_sleep = 1;
				debounce();
			}
		}
		else if (mode == 7)
		{
			if (bit_is_set(cur_second, 0))
			{
				display_second = -1;
				display_minute = -1;
				display_hour = -1;
			}
			else
			{
				display_second = cur_second;
				display_minute = cur_minute;
				display_hour = cur_hour;
			}
		}
		
		if (to_sleep)
		{
			// nothing to do, disable LEDs and go to sleep
			display_second = -1;
			display_minute = -1;
			display_hour = -1;
			mode = 0;
			
			// save data if required
			if (eeprom_is_dirty)
			{
				eeprom_update_byte(0, cur_second);
				eeprom_update_byte(1, cur_minute);
				eeprom_update_byte(2, cur_hour);
				eeprom_update_byte(3, alarm_minute);
				eeprom_update_byte(4, alarm_hour);
				eeprom_update_byte(5, alarm_enabled);
				
				eeprom_is_dirty = 0;
			}
			
			if (alarm_active == 0)
			{
				sleep_enable(); // SMCR |= _BV(SE); // enable sleep
				sleep_cpu();
				sleep_disable(); // SMCR &= ~_BV(SE); // disable sleep, prevent accidental sleep
			}
		}
	}
	
	return 0;
}