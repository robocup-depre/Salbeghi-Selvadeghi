#define F_CPU 16000000ul
#include <avr/io.h>
#include "ADC.h"

/*-------------ADC-----------------*/
void InitADC(void){
	
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));    //16Mhz/128 = 125Khz the ADC reference clock
	ADMUX |= (1<<REFS0);                //Voltage reference from Avcc (5v)
	ADCSRA |= (1<<ADEN);                //Turn on ADC
	ADCSRA |= (1<<ADSC);                //Do an initial conversion because this one is the slowest and to ensure that everything is up and running
	
}
int StartADC(uint8_t channel){
	for (int i=0;i<2;i++)
	{
		ADMUX &= 0xF0;
		ADCSRB &=~(1<<MUX5);                    //Clear the older channel that was read
		if(channel<=7){
			ADMUX |= channel;                //Defines the new ADC channel to be read
		}else{
			ADMUX |= (channel & 0b00000111);
			ADCSRB |= 1<<MUX5;
		}
		
		ADCSRA |= (1<<ADSC);                //Starts a new conversion
		while(ADCSRA & (1<<ADSC));            //Wait until the conversion is done
	}
	return ADCW;                    //Returns the ADC value of the chosen channel
}


/*---------------PWM----------------------*/

