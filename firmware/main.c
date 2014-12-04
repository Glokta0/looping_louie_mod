/**
 * Looping Louie Modification
 */

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#define DELAY_LOOP	1500
#define DELAY_BREAK	500
#define DELAY_BACKWARD	1000
#define DELAY_STEP	200

#define SEED	PB3
#define POTI	PB4
#define PWM	PB0
#define DIR1	PB1
#define DIR2	PB2

#define BREAK()		do {						\
	PORTB &= ~((1<<DIR1) | (1<<DIR2));				\
} while (0)

#define FORWARD()	do {						\
	PORTB &= ~(1<<DIR2);						\
	PORTB |= (1<<DIR1);						\
} while (0)

#define BACKWARD()	do {						\
	PORTB &= ~(1<<DIR1);						\
	PORTB |= (1<<DIR2);						\
} while (0)

static void set_speed(void);
static void step_to_level(uint8_t target);

static uint8_t level = 0;			/* speed level [0-2] */

int
main(void)
{
	DDRB |= (1<<PWM) | (1<<DIR1) | (1<<DIR2);
	PORTB |= (1<<DIR1);

	OCR0A = 0xff;				/* initialize compare value */
	TCCR0A |= (1<<WGM01) | (1<<WGM00) |	/* fast PWM */
	    (1<<COM0A1);			/* non-inverting */	
	TCCR0B |= (1<<CS00);			/* no prescaling */

	ADMUX |= (1<<ADLAR) |			/* left adjust ADC result */
	    (1<<MUX1) | (1<<MUX0);		/* select PB3 / ADC3 as input */
	ADCSRA |= (1<<ADEN) |			/* enable ADC */
	    (1<<ADPS2) | (1<<ADPS1) |		/* prescaler div 64 */
	    (1<<ADSC);				/* start conversion */

	while (ADCSRA & (1<<ADSC));
	srand(ADCH);

	ADMUX ^= (1<<MUX0);			/* select PB4 / ADC2 as input */
	ADCSRA |= (1<<ADATE) |			/* enable auto trigger,
						   implies free running mode */
	    (1<<ADSC);				/* start conversion */

	for (;;) {
		uint8_t next = rand() % 3;
		step_to_level(next);

		if((rand() % 10) == 0) {
			uint8_t current_level = level;

			step_to_level(0);
			BREAK();
			_delay_ms(DELAY_BREAK);

			BACKWARD();
			step_to_level(current_level);
			_delay_ms(DELAY_BACKWARD);

			step_to_level(0);
			BREAK();
			_delay_ms(DELAY_BREAK);

			FORWARD();
			step_to_level(current_level);
		}

		_delay_ms(DELAY_LOOP);
	}

	/* NOTREACHED */
	return (0);
}

static void
step_to_level(uint8_t target)
{
	if (target == level)
		return;

	int8_t incr = 1;
	if (target < level)
		incr = -1;

	for (;target != level; level+=incr) {
		set_speed();
		_delay_ms(DELAY_STEP);
	}
}

static void
set_speed()
{
	uint8_t poti = ADCH / 3;

	switch (level) {
	case 0:
		OCR0A = 0xaa;
		break;
	case 1:
		OCR0A = 0xaa + (poti / 2);
		break;
	case 2:
		OCR0A = 0xaa + poti;
		break;
	}
}
