#define F_CPU 16000000ul
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "Kit.h"
#include "NeoPixel.h"
#include "serial.h"
//#include "millis.h"
#include "PID-PWM.h"
int nkit;
void initKit(){
	nkit=8;
	DDRB|=1<<PB4;
	TCCR2A=(1<<WGM21)|(1<<WGM20)|(1<<COM2A1)|(0<<COM2A0);
	TCCR2B=(0<<WGM22)|(1<<CS22)|(1<<CS21)|(1<<CS20);
	setServoKit(20);
}

void setServoKit(int num){
 OCR2A=num;
}

unsigned long oldM_tot, oldM_oscillazione, oldM_ritorno, oldM_led;
uint8_t f_oscillazione, f_s_ritorno, f_ritorno, f_led;
void rilasciakit(uint8_t Dir, uint8_t num){
	oldM_tot=millis();
	oldM_led=oldM_tot;
	(Dir==Destra)? ColoreLED3(BLUE):ColoreLED1(BLUE);
	//setServoKit((Dir==Destra)?28:12);
	setServoKit(28);
	f_oscillazione=0;
	f_s_ritorno=0;
	f_ritorno=0;
	f_led=0;
	while(1){
		if(f_oscillazione==0 && ((millis()-oldM_tot)>=200 ) && num!=0 && nkit!=0 && f_ritorno==0){
			 f_oscillazione=1;
		}
		if(nkit==0)num=0;
		if(nkit!=0 && (f_oscillazione==1 || (f_oscillazione>1 && ((millis()-oldM_oscillazione)>=50)))){
			if(f_oscillazione%2==0){
				//setServoKit((Dir==Destra)?28:12);
				setServoKit(28);
			}
			else{
			//	setServoKit((Dir==Destra)?26:14);
				setServoKit(26);
			}
			f_oscillazione++;
			oldM_oscillazione=millis();
			if(f_oscillazione>12){ 
				f_oscillazione=0;
				f_ritorno=1;
			}
		}
		
		if(f_ritorno==1){
			if(f_s_ritorno==0) {
				setServoKit((Dir==Destra)?20:21);
				f_s_ritorno=1;
				oldM_ritorno=millis();
			}
			if(f_s_ritorno==1 && ((millis()-oldM_ritorno)>=200)){
				setServoKit((Dir==Destra)?21:20),
				f_s_ritorno=2;
				oldM_ritorno=millis();
			}
			if(f_s_ritorno==2 && ((millis()-oldM_ritorno)>=200)){
				setServoKit(20);
				f_ritorno=0;
				num--;
				nkit--;
				f_s_ritorno=0;
			}
		}
		
		if(millis()-oldM_led>=500){
			if(f_led==0){
				(Dir==Destra)? ColoreLED3(WHITE):ColoreLED1(WHITE);
				f_led=1;
			}else{
				(Dir==Destra)? ColoreLED3(BLUE):ColoreLED1(BLUE);
				//ColoreFotoresistenza(0,0,255);
				f_led=0;
			}
			oldM_led=millis();
		}
		if((millis()-oldM_tot>=5000) && num==0) break;
		
	}
	ColoreLED1(SPENTO);	
	ColoreLED3(SPENTO);
}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
// 	
// 	
// 	for(int i=0;i<num;i++){
// 		setServoKit(28);
// 		_delay_ms(200);
// 		for(int i=0;i<6;i++){
// 			setServoKit(28);
// 			_delay_ms(50);
// 			setServoKit(26);
// 			_delay_ms(50);
// 		}
// 		setServoKit(20);
// 		_delay_ms(100);
// 		setServoKit(21);
// 		_delay_ms(100);
// 		setServoKit(20);
// 	}
// 	while()
//}

void rilasciakitSX(int num){

	OCR2A=12;
	_delay_ms(200);
	for(int i=0;i<6;i++){
		OCR2A=12;
		_delay_ms(50);
		OCR2A=14;
		_delay_ms(50);
	}
	OCR2A=21;
	_delay_ms(500);
	OCR2A=20;
}
