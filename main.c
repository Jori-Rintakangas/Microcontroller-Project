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

#define ENTER_KEY 13
#define BUFFER_SIZE 4

#define SYSTEM_SLEEP (PIND & (1 << PIND2)) == 0
#define NEW_SETTING_RECEIVED (UCSR0A & (1 << RXC0)) != 0

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

/* Reading temperature setting send by virtual terminal */
void receive_USART()
{
	if (NEW_SETTING_RECEIVED)
	{
		int i = 0;
		char buffer[BUFFER_SIZE];
		char c = '\0';
		while (c != ENTER_KEY) // Receive characters until enter pressed
		{
			// Wait until data receiving is ready
			while ( !(UCSR0A & (1 << RXC0)));
			c = UDR0;
			buffer[i] = c;
			i++;
		}
		setting = atoi(buffer);
	}
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
		if (temp_source == TRIMMER)
		{
			setting = read_ADC(CONTROL);
		}
		else
		{	
			receive_USART();
		}
		
		
		reading = read_ADC(HEATER);
				
		// Setting value for PWM to control heating element
		OCR0A = setting;
				
		int setting_pct = round((setting / TEMP_MAX) * 100.0);
		int reading_pct = round((reading / TEMP_MAX) * 100.0);
		int diff_pct = round(VOLTAGE_DIV * (setting / TEMP_MAX) * 100.0);
						
		if (setting_pct <= reading_pct + diff_pct + 1
			&& setting_pct >= reading_pct + diff_pct - 1)
		{
			PORTB |= (1 << PORTB1);
			PORTB &= ~(1 << PORTB0) & ~(1 << PORTB2);
		}
		else if (setting_pct > reading_pct + diff_pct)
		{
			PORTB |= (1 << PORTB2);
			PORTB &= ~(1 << PORTB0) & ~(1 << PORTB1);
		}
		else
		{
			PORTB |= (1 << PORTB0);
			PORTB &= ~(1 << PORTB1) & ~(1 << PORTB2);
		}		
    }
}

