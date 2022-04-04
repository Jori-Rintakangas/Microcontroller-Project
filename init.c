/*
 * 
 */


#include "init.h"


/* Initialization of ON/OFF and LOCAL/EXTERNAL switches */
void init_switches()
{
	// Setting switch pins as input
	DDRD &= ~(1 << DDD2) & ~(1 << DDD3);
		
	// Enabling pull-ups
	PORTD |= (1 << PORTD2) | (1 << PORTD3);
	
	// Enabling interrupts in PORTD
	PCICR |= (1 << PCIE2);
	
	// Enabling interrupts in PD2 and PD3 pins
	PCMSK2 |= ((1 << PCINT18) | (1 << PCINT19));
}
