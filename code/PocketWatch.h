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

#define debounce() _delay_us(100)
#define HOLD_CNT_THRESH (10 * 1000 / 50)
#define TIMEOUT_THRESH (10 * 1000 / 50)
#define ANIMATION_SPEED (10)
#define MOTOR_PULSE_LENGTH (0x100 / 8)

typedef enum
{
	MODE_WAS_SLEEPING = 0,
	MODE_CURTIME_ENTERING,
	MODE_CURTIME_SHOWING,
	MODE_CURTIME_EXITING,
	MODE_SETTIME_HOUR,
	MODE_SETTIME_MINUTE,
	MODE_SETALARM_HOUR,
	MODE_SETALARM_MINUTE,
	MODE_SETALARM_ONOFF,
	MODE_TO_SLEEP,
	MODE_ALARMING,
} mode_t;


#endif