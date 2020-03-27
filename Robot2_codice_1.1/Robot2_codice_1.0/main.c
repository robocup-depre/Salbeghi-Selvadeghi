/*
 * Marilù_codice_4.2.c
 *
 * Created: 06/02/2020 18:31:09
 * Author : Pogetta Matteo
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "Movimenti.h"
#include "Sensori.h"
#include "NeoPixel.h"
#include "PID-PWM.h"
#include "Kit.h"
#include "BNO.h"
#include "TWI.h"
#include "ADC.h"
#include "VL53L0.h"
#include "VL6180X.h"
#include "I2C.h"

#define ndestre 5

 ISR(INT2_vect){
	 fISR(nINT2);
 }
 ISR(INT3_vect){
 	fISR(nINT3);
 }
 ISR(INT4_vect){
 	fISR(nINT4);
 }
 ISR(INT5_vect){
 	fISR(nINT5);
 }

int main(void)
{	
	SpegniLED1();
	SpegniLED2();
	SpegniLED3();
	Serial_Init();
	init_Fotoresistenza();
	//Serial2_Init();
	Serial_Send_String("init \n");
	init_I2C(),
	initPulsanti();
	//setVL53L0();
	setVL6180X();
	BNO_init();
 	init_PID();
 	start_PID();
 	InitADC();
	initKit();

 	giriAvanti=1.2;
 	ultimoGira=180;
 	statokit=1;
	uint8_t presenzaMuroSinistra=0;//1 se è presente muro a sx
	uint8_t presenzaMuroDestra=0;//1 se è presente muro a dx
	uint8_t presenzaMuroAvanti=0;//1 se è presente muro av
 	countFinecorsa=0;
 	float diDxAv, diSxAv,diDxDi,  diSxDi, diAvDx, diAvSx;	
 	int contadestre=0;
	unsigned long millis_allinea;
	unsigned int tempo_allinea;
	uint8_t FD, FS;
	uint8_t no_avanti=0, allinea=0, no_destra=0;
	uint8_t mov=0;
	
	resetta_distanza();
	_delay_ms(500);
	init_asseY=readAxis(asseY);
	Serial_Send("fine init\n");
	
  	while (1){		  
		ColoreLED2(WHITE);
 		if(ColoreCasella()==Nero){
 			dritto(30,-1.2);
 			no_avanti=1;
 			contadestre=0;
			//if(ultimoGira==90){
				 no_destra=1;
			//}
 		}
    	diDxAv=DistanzaLaser(DestraAvanti);
    	diDxDi=DistanzaLaser(DestraDietro);
   		diSxAv=DistanzaLaser(SinistraAvanti);
  		diSxDi=DistanzaLaser(SinistraDietro);
		ColoreLED2(RED);
		mov=1;
		if((diDxAv>=20)&&(diDxDi<20)){ 
			set_giri(giriAvanti);
			while(diDxDi<20) diDxDi=DistanzaLaser(DestraDietro);
		}
		else if((diSxAv>=20)&&(diSxDi<20)){
			set_giri(giriAvanti);
			while(diSxDi<20) diSxDi=DistanzaLaser(SinistraDietro);
		}
		else if((diDxAv<20)&&(diDxDi>=20)){
			set_giri(-giriAvanti);
			while(diDxAv<20) diDxAv=DistanzaLaser(DestraAvanti);
		}
		else if((diSxAv<20)&&(diSxDi>=20)){
			set_giri(-giriAvanti);
			while(diSxAv<20) diSxAv=DistanzaLaser(SinistraAvanti);
		}else{
			mov=0;
		}
		if(mov==1){
			mov=0;
			resetta_distanza();
			while(ABS(distanzaPercorsa())<6){
				rilevaVittima();
				set_giri(1);
			}
		}
		StopMotori();
		ColoreLED2(WHITE);
		diDxAv=DistanzaLaser(DestraAvanti);
		diDxDi=DistanzaLaser(DestraDietro);
		diSxAv=DistanzaLaser(SinistraAvanti);
		diSxDi=DistanzaLaser(SinistraDietro);
		diAvDx=DistanzaLaser(AvantiDestra);
		diAvSx=DistanzaLaser(AvantiSinistra);
		if (((diSxAv+diSxDi)/2)<=20.0){
			presenzaMuroSinistra=1;
		}
		if (((diDxAv+diDxDi)/2) <=20.0){
			presenzaMuroDestra=1;
		}
		if (((diAvSx+diAvDx)/2) <=20.0){
			presenzaMuroAvanti=1;
		}
  		if ((no_destra==0)&&(contadestre<ndestre)&&((diDxAv>20)||(diDxDi>20))){
			ColoreLED2(BLUE);
  			Gira(90);
  			contadestre++;
  			if (presenzaMuroSinistra==1){
				 allinea=1;
				 presenzaMuroSinistra=0;
  			}
			  ColoreLED2(SPENTO);
  		}else if(diAvDx<=10 || diAvSx<=10 || no_avanti==1 || no_destra==1){
			ColoreLED2(YELLOW);
			no_destra=0;
			no_avanti=0;
  			contadestre=0;
  			if((diSxAv>18.0) && (diSxDi>18.0)){
  				 Gira(-90);
				 if (presenzaMuroDestra==1){
					allinea=1;
					presenzaMuroDestra=0;
				}
  			}else{
				if(diDxAv>=diSxAv){	//verifica da che parte è meglio girare
  					Gira(90);
  					rilevaVittima();
  					Gira(90);
  					parallelototale();
  				}else{
  					Gira(-90);
  					rilevaVittima();
  					Gira(-90);
  					parallelototale();
  				}
				if(presenzaMuroAvanti==1){
					presenzaMuroAvanti=0;
					allinea=1;
				}
			}
			 ColoreLED2(SPENTO); 	
  		}
		presenzaMuroDestra=0;
		presenzaMuroSinistra=0;
		presenzaMuroAvanti=0;
		if(allinea==1){  //DECOMMENTARE SE CI SONO FINECORSA DIETRO
			ColoreLED2(GREEN);
			allinea=0;
			parallelototale();
			set_giri(-0.8);
// 			FD=0;
// 			FS=0;
 			millis_allinea=millis();
			tempo_allinea=1000; //impostare a 5000 con i finecorsa dietro
 			while(millis()-millis_allinea<tempo_allinea){
// 				if(finecorsaDietroDx() && FD==0){
// 					FD=1;
// 					tempo_allinea=1000;
// 					millis_allinea=millis();
// 				}
// 				if(finecorsaDietroSx() && FS==0){
// 					FS=1;
// 					tempo_allinea=1000;
// 					millis_allinea=millis();
// 				}
// 				if(FD==1 && FS==1){
// 					g_temp=readAxis(asseZ);
// 					if(g_temp>315 || g_temp <=45) g_temp1=0;
// 					else if(g_temp>45 && g_temp<=135) g_temp1=90;
// 					else if(g_temp>135 && g_temp<=225) g_temp1=180;
// 					else if(g_temp>225 && g_temp<=315) g_temp1=270;
// 					correzzione_asseZ=g_temp-g_temp1;
// 					break;
// 				}
			}
			dritto(0.5,-1.5);
			resetta_distanza();
			set_giri(1);
			while(distanzaPercorsa()<2){
				rilevaVittima();
				set_giri(1);
			}
			StopMotori();
		}
   		if (contadestre>=ndestre){
   			Avanti30cm(giriAvanti);
			contadestre=0;
   		}
		parallelototale();
		ColoreLED2(SPENTO);
  		rilevaVittima();
  		Avanti30cm(giriAvanti);
 		rilevaVittima();
  	}
 }