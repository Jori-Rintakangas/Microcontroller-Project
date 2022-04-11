/*
 * 
 */


#include "init.h"

/* Initialization of PWM for heating element control */
void init_PWM()
{
	// Setting PWM output pin
	DDRD |= (1 << DDD6);
	
	// Setting Fast PWM mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	
	// Setting non-inverting mode
	TCCR0A |= (1 << COM0A1);
	
	// Prescaling by 256 -> 62.5kHz
	TCCR0B |= (1 << CS02); //(1 << CS01) | (1 << CS00);
	
	// initialize counter
	TCNT0 = 0;
	// initialize compare value
	OCR0A = 0;
}


/* Initialization of USART data transfer */
void init_USART(unsigned int ubrr)
{
	// Setting USART baudrate
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	
	// Enabling receiver and receiver interrupts
	UCSR0B |= (1 << RXEN0) | (1 << RXCIE0);	
}


/* Initialization of analog-to-digital converter */
void init_ADC()
{
	// Setting PC0 and PC2 as input
	DDRC &= ~(1 << DDC0) & ~(1 << DDC2);
	
	// Enable analog input in pins PC0 (Temperature measurement) 
	// and PC2 (Temperature setting measurement)
	DIDR0 |= ((1 << ADC0D) | (1 << ADC2D));
		
	// Using Vcc 5v as reference
	ADMUX |= ((1 << REFS0));
	
	// Using only high byte of the result
	ADMUX |= (1 << ADLAR);
	
	// 128 as a prescaler -> 125 kHz AD-clock
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
	
	// Enabling AD-converter in PRR
	PRR &= ~(1 << PRADC);
	
	// Enabling AD-converter in ADCSRA
	ADCSRA |= (1 << ADEN);
	
	// Disabling analog comparator for power reduction
	ACSR |= (1 << ACD);
	
}


/* Initialization of ON/OFF and LOCAL/EXTERNAL switches */
void init_switches()
{
	// Setting switch pins as input
	DDRD &= ~(1 << DDD2) & ~(1 << DDD3);
		
	// Enabling pull-ups
	PORTD |= (1 << PORTD2) | (1 << PORTD3);
	
	// Enabling interrupts in PORTD
	PCICR |= (1 << PCIE2);
	
	// Enabling pin change interrupts in PD2 pin
	PCMSK2 |= (1 << PCINT18);
	
	// Enabling external interrupts in PD3 pin by any logical change
	EICRA |= (1 << ISC10);
	
	// Enabling INT1 interrupt
	EIMSK |= (1 << INT1);
}
