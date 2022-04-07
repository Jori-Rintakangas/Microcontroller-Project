/*
 * 
 */ 

#define F_CPU 16e6

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdlib.h>
#include <math.h>

#include "init.h"

#define TRIMMER 8
#define TERMINAL 0

#define BAUDRATE 9600
#define UBRR F_CPU/16/BAUDRATE-1

#define CONTROL 2
#define HEATER 0

#define TEMP_MAX 255.0
#define VOLTAGE_DIV 5000.0/105000


#define SYSTEM_SLEEP (PIND & (1 << PIND2)) == 0

uint8_t setting;
uint8_t reading;
uint8_t temp_source;

/* Wake up from sleep mode */
ISR(PCINT2_vect) { }

/* Reading the source of temperature setting */
ISR(INT1_vect)
{
	temp_source = PIND & (1 << PIND3);
}

/* Reading an analog-to-digital converter result from given channel */
uint8_t read_ADC(uint8_t channel)
{
	// Selecting a channel with safety masking
	ADMUX = (ADMUX & 0xf0) | (channel & 0x0f);
	
	// starting conversion
	ADCSRA |= (1 << ADSC);
	
	// Waiting until conversion ready			
	while (ADCSRA & (1 << ADSC));
			
	return ADCH;
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
	init_ADC();
	init_USART(UBRR);
	init_PWM();
		
    while (1) 
    {
		set_sleep_mode(SLEEP_MODE_PWR_DOWN); // set the power-down sleep-mode
		cli(); // disable interrupts
		if (SYSTEM_SLEEP)
		{
			sleep_enable(); // set SE-bit
			sei(); // enable interrupts
			sleep_cpu(); // SLEEP-instruction
			// entry-point after wake-up
			sleep_disable(); // reset SE-bit
		}
		sei(); // enable interrupts	
		
		
		// Reading selected temperature and true temperature
		setting = read_ADC(CONTROL);
		reading = read_ADC(HEATER);
				
		// Setting value for PWM to control	heating element
		OCR0A = setting;
				
		int setting_pct = round((setting / TEMP_MAX) * 100.0);
		int reading_pct = round((reading / TEMP_MAX) * 100.0);
		int diff_pct = round(VOLTAGE_DIV * (setting / TEMP_MAX) * 100.0);
						
		PORTB &= ~(1 << PORTB0) & ~(1 << PORTB1) & ~(1 << PORTB2);
		if (setting_pct <= reading_pct + diff_pct + 1
			&& setting_pct >= reading_pct + diff_pct - 1)
		{
			PORTB |= (1 << PORTB1);
		}
		else if (setting_pct > reading_pct + diff_pct)
		{
			PORTB |= (1 << PORTB2);
		}
		else
		{
			PORTB |= (1 << PORTB0);
		}		
    }
}

