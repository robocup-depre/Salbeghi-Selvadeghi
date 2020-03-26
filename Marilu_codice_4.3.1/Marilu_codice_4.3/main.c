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
	ColoreLED(SPENTO);
	ColoreLED1(SPENTO);
	Serial_Init();
	//Serial2_Init();
	Serial_Send_String("init \n");
	TWI_init();
	BNO_init();
	initPulsanti();
	initRilKit();
 	init_PID();
 	start_PID();
 	InitADC();
	
 	LedGara=1;
 	giriAvanti=1.2;
 	ultimoGira=180;
 	statokit=1;
 	uint8_t presenzaMuroSinistra=0;//1 se è presente muro a sx
 	uint8_t presenzaMuroDestra=0;//1 se è presente muro a dx
 	uint8_t presenzaMuroAvanti=0;//1 se è presente muro av
 	countFinecorsa=0;
 	float diDxAv, diSxAv,diDxDi,  diSxDi, diAvDx, diAvSx;	
	 float g_temp=0, g_temp1=0;
 	int contadestre=0;
 	int ndestre=5;
	uint8_t no_avanti=0, allinea=0, no_destra=0;
	unsigned long millis_allinea;
	unsigned int tempo_allinea;
	uint8_t FD, FS;
	resetta_distanza();
	_delay_ms(500);
	reset_millis();
	init_asseY=readAxis(asseY);
	correzzione_asseZ=readAxis(asseZ);

  	while (1){		  
  		//Serial_Send_String("main\n");
		 ColoreLED(WHITE);
		if(ColoreCasella()==Nero){
			dritto(30,-1.2);
			no_avanti=1;
			contadestre=0;
			//if(ultimoGira==90){
				no_destra=1;
			//}
		}
    	diDxAv=DistanzaIR(DestraAvanti);
    	diDxDi=DistanzaIR(DestraDietro);
   		diSxAv=DistanzaIR(SinistraAvanti);
  		diSxDi=DistanzaIR(SinistraDietro);
		ColoreLED(RED);
		uint8_t mov=1;
		if((diDxAv>=20)&&(diDxDi<20)){
			set_giri(giriAvanti);
			while(diDxDi<20) diDxDi=DistanzaIR(DestraDietro);
		}
		else if((diSxAv>=20)&&(diSxDi<20)){
			set_giri(giriAvanti);
			while(diSxDi<20) diSxDi=DistanzaIR(SinistraDietro);
		}
		else if((diDxAv<20)&&(diDxDi>=20)){
			set_giri(-giriAvanti);
			while(diDxAv<20) diDxAv=DistanzaIR(DestraAvanti);
		}
		else if((diSxAv<20)&&(diSxDi>=20)){
			set_giri(-giriAvanti);
			while(diSxAv<20) diSxAv=DistanzaIR(SinistraAvanti);
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
		ColoreLED(WHITE);
		diDxAv=DistanzaIR(DestraAvanti);
		diDxDi=DistanzaIR(DestraDietro);
		diSxAv=DistanzaIR(SinistraAvanti);
		diSxDi=DistanzaIR(SinistraDietro);
		diAvDx=DistanzaIR(AvantiDestra);
		diAvSx=DistanzaIR(AvantiSinistra);
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
  			ColoreLED(BLUE);
  			
  			Serial_Send_String("mainiDx\n");
  			Gira(90);
  			contadestre++;
  			if (presenzaMuroSinistra==1){
				 allinea=1;
				 presenzaMuroSinistra=0;
  			}
  			ColoreLED(SPENTO);
  		}else if(diAvDx<=10 || diAvSx<=10 || no_avanti==1 || no_destra==1){
			ColoreLED(YELLOW);
			no_destra=0;
			no_avanti=0;
  			contadestre=0;
  			if((diSxAv>18.0) && (diSxDi>18.0)){
 				 Serial_Send_String("gira -90\n");		
  				 Gira(-90);
				 if (presenzaMuroDestra==1){
					allinea=1;
					presenzaMuroDestra=0;
				}
  			}else{
				if(diDxAv>=diSxAv){	//verifica da che parte è meglio girare
  					Serial_Send_String("180-Or\n");
  					Gira(90);
  					rilevaVittima();
  					Gira(90);
  					parallelototale();
  				}else{
  					Serial_Send_String("180-AntOr\n");
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
			ColoreLED(SPENTO);
  		}
		  presenzaMuroDestra=0;
		  presenzaMuroSinistra=0;
		  presenzaMuroAvanti=0;
		if(allinea==1){
			ColoreLED(GREEN);
			allinea=0;
			parallelototale();
			set_giri(-0.8);
			FD=0;
			FS=0;
			millis_allinea=millis();
			tempo_allinea=5000;
			while(millis()-millis_allinea<tempo_allinea){
				if(finecorsaDietroDx() && FD==0){
					FD=1;
					tempo_allinea=1000;
					millis_allinea=millis();
				}
				if(finecorsaDietroSx() && FS==0){
					FS=1;
					tempo_allinea=1000;
					millis_allinea=millis();
				}
				if(FD==1 && FS==1){
					g_temp=readAxis(asseZ);
					if(g_temp>315 || g_temp <=45) g_temp1=0;
					else if(g_temp>45 && g_temp<=135) g_temp1=90;
					else if(g_temp>135 && g_temp<=225) g_temp1=180;
					else if(g_temp>225 && g_temp<=315) g_temp1=270;
					correzzione_asseZ=g_temp-g_temp1;
					break;
				}
			}
			dritto(0.5,-1.5);
			resetta_distanza();
			set_giri(1);
			while(distanzaPercorsa()<3){
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
  		ColoreLED(SPENTO);
  		rilevaVittima();
  		Avanti30cm(giriAvanti);
 		rilevaVittima();
  	}
 }