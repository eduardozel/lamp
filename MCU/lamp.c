/*
 * lebre.c
 *
 * Created: 21.1.2016 19:38:59
 *  Author: Eduardo
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <math.h>
#include <util/delay.h>


#define YEL_ON  PORTB  |=  0b00000001
#define YEL_OFF PORTB  &=  0b11111110

#define yel_OFF PORTB  |=  0b00001000
#define yel_ON  PORTB  &=  0b11110111

#define BLU_OFF PORTB  |=  0b00000100
#define BLU_ON  PORTB  &=  0b11111011

#define bntOn  bit_is_clear(PINB, PB1)
#define bntOff bit_is_set(PINB, PB1)

#define set_Timer_Cnt TCNT0 = (256 - 234)

const unsigned char OneSec =   5;
const int maxDelay = 2100;// =7*60*OneSec;

int YELDelay = 1000;
int BLUDelay = 100;

int delay;

void startADC( unsigned char chan
) {
	ADMUX  = 0b01100010; // ADC2
	_delay_ms(10);
	ADCSRA =( 1 << ADEN )|( 1 << ADIE ) | ( 1 << ADPS2 ) | ( 1 << ADPS1 ) | ( 1 << ADPS0 ) | ( 1 << ADSC ) ;
} // startADC


ISR(INT0_vect)
{
  delay = maxDelay;
  set_Timer_Cnt;
  TIMSK0 |= (unsigned char)_BV(TOIE0);
  set_sleep_mode( SLEEP_MODE_IDLE );
  yel_ON;
} // SIGNAL(SIG_PIN_CHANGE0)

ISR(TIM0_OVF_vect)
{
	if ( --delay == 0 ) {
		BLU_OFF;
	    YEL_OFF;
		TIMSK0 &= !(_BV(TOIE0));
		set_sleep_mode( SLEEP_MODE_PWR_DOWN );
    } else {
      if        ( ( delay < BLUDelay) ) {
	    yel_OFF;
	  } else if ( ( delay < YELDelay ) ) {
	    YEL_OFF;
        BLU_ON;
      } else if ( ( delay < (maxDelay - 5*OneSec )) ) {
	    YEL_ON;
        startADC(2);
      };
      set_Timer_Cnt;
    }; // if delay == 0

} // ISR(TIM0_OVF_vect)

ISR(ADC_vect)
{
  YELDelay = maxDelay - ADCH*6;
  BLUDelay = YELDelay - 15*OneSec;
  ADCSRA &= ~( 1 << ADEN );
} // ISR(ADC_vect




int main(void)
{
  DDRB  = ( 1 << DD0 ) | ( 1 << DD2 ) | ( 1 << DD3 );
  PORTB = 0b11111110;

  GIMSK = ( 1 << INT0 );
  //  MCUCR = 0; // 0 on INT0

  ACSR |= (unsigned char)( 1 << ACD );
  wdt_disable();

  TCCR0B = ( 1 << CS02 ) | ( 1 << CS00 ); // /1024

  sei();
  set_sleep_mode( SLEEP_MODE_PWR_DOWN );
  while (1) {
	  sleep_enable();
	  sleep_cpu();
  } // while
}