#define F_CPU 16000000ul
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "Kit.h"
#include "NeoPixel.h"
#include "serial.h"
int Nstep,  step, var1S, var2S, varF1, varF2;
unsigned int contatorekit;
//long millsInizio;


void initTimerkit(){
	TCCR5B=(1<<WGM52)|(1<<CS52)|(1<<CS50);
	OCR5A=(int)((15625.0*tempoStep)); //1300us
	TIMSK5=(1<<OCIE5A);
	step=0;
	Nstep=0;
	contatorekit=0;
	statoRilascio=0;
	var1S=1.0/tempoStep;
	var2S=2.0/tempoStep;
	varF1=tempoLampeggio/tempoStep;//5000
	varF2=stepAvanti+(Attesacentrale/tempoStep);//3950
	if(varF1<varF2)varF1=varF2;
	ColoreLED1(SPENTO);
	//millsInizio=millis();
}

void stopTimerKit(){
	TCCR5B=0;
}



ISR(TIMER5_COMPA_vect){
	if(statoRilascio==0){
		// Serial_Send_String("0..........\n");
	 	
	 	if(contatorekit%var2S==0){	 
		 	ColoreLED1(BLUE);
		 }else if(contatorekit%var1S==0){
		 	ColoreLED1(WHITE);
	 	}
	 	if(Nstep<stepAvanti){
		 	step=one_step(step,statoRilascio);
		 	Nstep++;
	 	}
	 	if(contatorekit>varF1){
		 	statoRilascio=1;
		 	ColoreLED1(SPENTO);
	 	}
		 
		 contatorekit++;
	 }else if(Nstep<(stepIndietro+stepAvanti)){
			// Serial_Send_String("1..........\n");
	 	step=one_step(step,statoRilascio);
		Nstep++;
 	}else{
		 PORTC=0;
		 statoRilascio=-1;
	 	stopTimerKit();
 	}
		
	
	
	
 	//if(STimer==0){
 		//if(statoled==WHITE){
 			//ColoreLED1(BLUE);
 			//statoled=BLUE;
 		//}else if(statoled==BLUE){
 			//ColoreLED1(WHITE);
 			//statoled=WHITE;
 		//}
 	//}else if(STimer==1){
 		//ColoreLED1(RED);
 		//if(Nstep<stepIndietro){
 			//step=one_step(step,1);
 			//Nstep++;
 			//Serial_Send_String("Nuovo step \n");
 		//}else{
  			//stopTimerKit();
  			//Serial_Send_String("Fine TIMR5\n");
 			//STimer=-1;
 			//ColoreLED1(SPENTO);
 		//}
 		//
 	//}
	
}

int one_step(int step, int dir){//funione motore passo passo
	switch (step){
		case 0: PORTC=1; break;
		case 1: PORTC=3; break;
		case 2: PORTC=2; break;
		case 3: PORTC=6; break;
		case 4: PORTC=4; break;
		case 5: PORTC=12; break;
		case 6: PORTC=8; break;
		case 7: PORTC=9; break;
	}
	if(dir==1)step++;
	else step--;
	if(step>7)step=0;
	if(step<0)step=7;
	return step;
}
void initRilKit(){
	DDRC|=0b00111111;
}
void rilasciokit(int num){
	ColoreLED1(SPENTO);
		int y=0;
		statoGenRil=1;
		while(y<num)
		{
			if(statoRilascio!=-1){
				initTimerkit();
				y++;
			}
		}
		statoGenRil=0;
	ColoreLED1(SPENTO);
}


/*------------------------------------------*/



