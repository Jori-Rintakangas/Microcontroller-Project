/*
 * 
 */ 

#define F_CPU 16e6

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdlib.h>
#include <math.h>

#include "init.h"

#define POTENTIOMETER 8

#define BAUDRATE 9600
#define UBRR F_CPU/16/BAUDRATE-1

#define CONTROL 2
#define HEATER 0

#define TEMP_MAX 255.0
#define VOLTAGE_DIV 5000.0/105000

#define ENTER_KEY 13
#define BUFFER_SIZE 4

#define GREEN_LED_ON() PORTB |= 0x02;\
					   PORTB &= ~0x05;
#define BLUE_LED_ON() PORTB |= 0x01;\
					  PORTB &= ~0x06;
#define RED_LED_ON() PORTB |= 0x04;\
					 PORTB &= ~0x03;

#define SYSTEM_SLEEP (PIND & (1 << PIND2)) == 0

uint8_t setting = 0;
uint8_t reading = 0;
uint8_t buff_index = 0;
uint8_t temp_source = 0;
uint8_t system_stable = 0;
uint8_t previous_stable_setting = 0;

char buffer[BUFFER_SIZE];


/* System power down mode */
void power_down()
{
	if (SYSTEM_SLEEP)
	{
		previous_stable_setting = 0;

		// Turning off LEDs
		PORTB &= ~(1 << PORTB1) & ~(1 << PORTB0) & ~(1 << PORTB2);
		// Disabling AD-converter
		ADCSRA &= ~(1 << ADEN);
		// Disabling functional blocks during the sleep
		PRR |= (1 << PRTIM0) | (1 << PRTIM2) | (1 << PRADC);
		
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		cli();
		sleep_enable(); // set SE-bit
		sei();
		sleep_cpu();
		// Wake up point
		sleep_disable(); // reset SE-bit
		sei();
		
		PORTB |= (1 << PORTB1);
		// Enabling AD-converter
		ADCSRA |= (1 << ADEN);
		// Enabling functional blocks after wake up
		PRR &= ~(1 << PRTIM0) & ~(1 << PRTIM2) & ~(1 << PRADC);
	}
}

/* Entering to sleep mode by ON/OFF switch toggle */
ISR(PCINT2_vect) 
{	
	power_down();
}

/* Reading the source of temperature setting */
ISR(INT1_vect)
{
	temp_source = PIND & (1 << PIND3);
}

/* Reading temperature setting from virtual terminal */
ISR(USART_RX_vect)
{
	// Reading a single character from USART to buffer
	char c = UDR0;
	buffer[buff_index] = c;
	buff_index++;
	
	// Reading the temperature setting from buffer when enter pressed
	if (c == ENTER_KEY)
	{
		setting = atoi(buffer);
		buff_index = 0;
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

/* Checking the initial positions of ON/OFF and LOCAL/EXTERNAL switches */
void check_system_state()
{
	// Software generated interrupt to find out ON/OFF switch position
	DDRD |= (1 << DDD2);
	DDRD &= ~(1 << DDD2);
	
	// Reading the LOCAL/EXTERNAL switch position
	temp_source = PIND & (1 << PIND3);
}

/* Adjusting temperature and turning on LEDs accordingly */
void adjust_temperature()
{
	// Setting value for PWM to control heating element
	OCR0A = setting;
	
	// Reading temperature value from heating element			
	reading = read_ADC(HEATER);
	
	// Calculating percentages of voltages	
	int setting_pct = round((setting / TEMP_MAX) * 100.0);
	int reading_pct = round((reading / TEMP_MAX) * 100.0);
	int drop_pct = round(VOLTAGE_DIV * (setting / TEMP_MAX) * 100.0);

	// If setting is within 1% of reading + drop over 5k resistor
	if (setting_pct <= reading_pct + drop_pct + 1
	 && setting_pct >= reading_pct + drop_pct - 1)
	{
		GREEN_LED_ON();
		previous_stable_setting = setting;
	}
	else if (setting_pct > reading_pct + drop_pct)
	{
		RED_LED_ON();
	}
	else
	{
		BLUE_LED_ON();
	}
}

/* Going to idle mode, waking up after 16 ms */
void idle_mode()
{
	// Enabling timer2 interrupts
	TIMSK2 |= (1 << TOIE2);
	TCNT2 = 0;
	set_sleep_mode(SLEEP_MODE_IDLE);
	cli();
	sleep_enable(); // set SE-bit
	sei();
	sleep_cpu();
	// Wake up point
	sleep_disable(); // reset SE-bit
}

/* Waking up from idle mode */
ISR(TIMER2_OVF_vect)
{
	// Disabling timer2 interrupts
	TIMSK2 &= ~(1 << TOIE2);
}

int main()
{	
	// Setting LED pins as output
	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
	sei();
	
	init_switches();
	init_USART(UBRR);
	init_ADC();
	init_PWM();
	init_idle_timer();
	check_system_state();
	
    while (1) 
    {		
		if (temp_source == POTENTIOMETER)
		{
			setting = read_ADC(CONTROL);
		}
				
		if (previous_stable_setting != setting)
		{
			adjust_temperature();
		}
		else
		{
			idle_mode();
		}		
    }
}

