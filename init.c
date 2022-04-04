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
	
	// Enabling pin change interrupts in PD2 pin
	PCMSK2 |= (1 << PCINT18);
	
	// Enabling external interrupts in PD3 pin by any logical change
	EICRA |= (1 << ISC10);
	
	// Enabling INT1 interrupt
	EIMSK |= (1 << INT1);
}
