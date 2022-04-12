/*
 *
 */ 

#include <avr/io.h>

#ifndef INIT_H_
#define INIT_H_

void init_PWM();

void init_USART(unsigned int ubrr);

void init_ADC();

void init_switches();

void init_idle_timer();



#endif /* INIT_H_ */