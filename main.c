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


#define SYSTEM_OFF (PIND & (1 << PIND2)) == 0

uint8_t temperature;
uint8_t temp_source;
uint8_t temp_setting;

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
		if (SYSTEM_OFF)
		{
			sleep_enable(); // set SE-bit
			sei(); // enable interrupts
			sleep_cpu(); // SLEEP-instruction
			// entry-point after wake-up
			sleep_disable(); // reset SE-bit
		}
		sei(); // enable interrupts	
		
		
		// Reading selected temperature and true temperature
		temp_setting = read_ADC(CONTROL);
		temperature = read_ADC(HEATER);
		
		PORTC &= ~(1 << PORTC0);	
		OCR0A = temp_setting;
		
		double set_pct = (temp_setting/255.0) * 100.0;
		double temp_pct = (temperature/243.0) * 100.0;
				
		int p1 = round(set_pct);
		int p2 = round(temp_pct);
		
		PORTB &= ~(1 << PORTB0) & ~(1 << PORTB1) & ~(1 << PORTB2);
		if (p1 <= p2 + 1 && p1 >= p2 - 1)
		{
			PORTB |= (1 << PORTB1);
		}
		else if (p1 > p2)
		{
			PORTB |= (1 << PORTB2);
		}
		else
		{
			PORTB |= (1 << PORTB0);
		}		
    }
}

