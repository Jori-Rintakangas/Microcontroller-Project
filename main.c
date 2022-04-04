/*
 * 
 */ 

//#define F_CPU 16e6

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "init.h"

#define TRIMMER 8
#define TERMINAL 0

#define SYSTEM_OFF (PIND & (1 << PIND2)) == 0

uint8_t temperature;
uint8_t temp_source;

/* Wake up from sleep mode */
ISR(PCINT2_vect) { }

/* Reading the source of temperature setting */
ISR(INT1_vect)
{
	temp_source = PIND & (1 << PIND3);
}

int main()
{
	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
	// Turning on all LEDs for 2 seconds in the beginning of program
	PORTB |= (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2);
	_delay_ms(2000);
	PORTB &= ~(1 << PORTB0) & ~(1 << PORTB1) & ~(1 << PORTB2);
	
	temp_source = PIND & (1 << PIND3);
	
	init_switches();
	
    while (1) 
    {
		set_sleep_mode(SLEEP_MODE_PWR_DOWN); // set the power-down sleep-mode
		cli(); // disable interrupts
		if (SYSTEM_OFF)
		{
			sleep_enable(); // set SE-bit
			sei(); // enable interrupts
			sleep_cpu(); // SLEEP-instruction
			// entry-point after wake-up
			sleep_disable(); // reset SE-bit
		}
		sei(); // enable interrupts
    }
}

