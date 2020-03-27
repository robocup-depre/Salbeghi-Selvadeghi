#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include "serial.h"
#include "Sensori.h"
#include "NeoPixel.h"
#include "PID-PWM.h"
#include "BNO.h"
#include "ADC.h"
#include "TWI.h"
#include "VL53L0.h"
#include "VL6180X.h"

/*--------------SENSORI------------------*/

float dist_temp;
float DistanzaLaser(uint8_t ind){
// 	uint8_t tipo;
// 	switch (ind){
// // 		case DestraAvanti: 
// // 			tipo=tipoLaser_DxAv;
// // 			break;
// // 		case SinistraAvanti:
// // 			tipo=tipoLaser_SxAv;
// // 			break;
// // 		case DestraDietro:
// // 			tipo=tipoLaser_DxDi;
// // 			break;
// // 		case SinistraDietro:
// // 			tipo=tipoLaser_SxDi;
// // 			break;
// // // 		case AvantiDestra:
// // // 			tipo=tipoLaser_AvSx;
// // // 			break;
// // 		case AvantiSinistra:
// // 			tipo=tipoLaser_AvSx;
// // 			break;
// 		default:
// 			tipo=0;
// 			break;
// 	}
///	if(tipo==0) 
	dist_temp= (float)readVL6180X(ind)/10.0;
//	else dist_temp= (float)readVL6180X(ind)/10.0;
	if(dist_temp==0)dist_temp=25.5;
	return dist_temp;
}

float SFoto;
//luce

void init_Fotoresistenza(){
	DDRB|=(1<<PB7); //fotoresistenza
	PORTB&=~(1<<PB7);
}

float Fotoresistenza(){
	//ColoreLED(WHITE);
	PORTB|=1<<PB7;
	_delay_ms(10);
	SFoto=0;
	for(int i=0;i<5;i++){
		SFoto+=StartADC(0);
	}
	//ColoreLED(SPENTO);
	PORTB&=~(1<<PB7);
	return (SFoto/5);
}

float TemperaturaDX(){   //temperaturaTpa81DX
	int devDX=0xD6;//6F;//sens dx
	float valoreDX=0, v=0;
	for (int i=2;i<9;i++)
	{
		v=TWI_read(devDX,i);
		if(valoreDX<v)
			valoreDX=v;
	}
	//Serial_Send_String("tempDX:");Serial_Send_Float(valoreDX);SerialN();
	return valoreDX;
}

float TemperaturaSX(){   //temperaturaTpa81SX
	int devSX=0xD0;//68;
	float valoreSX=0, v=0;
	for (int i=2;i<9;i++)
	{
		v=TWI_read(devSX,i);
		if(valoreSX<v)
			valoreSX=v;
	}
	//Serial_Send_String("tempSX:");Serial_Send_Float(valoreSX);SerialN();
	return valoreSX;
}

uint8_t ColoreCasella(void){
	float valore1=1023-Fotoresistenza();
	Serial_Send_String(" \n  val:");Serial_Send_Float(valore1);Serial_Send_String(" ");
	uint8_t Colore=Bianco;
	if(valore1>ArgentoMIN)
		Colore=Argento;
	else if(valore1>BiancoMIN)
		Colore=Bianco;
	else if(valore1>NeroMIN)
		Colore=Nero;
		
	return Colore;
}

/*-------------------------------------*/


/*--------------Pulsanti/finecorsa------------*/
uint8_t i, valore;

uint8_t pulsante(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINB & (1<<PB5))==0){
			valore=1;
			break;
		}
	}
	return valore;
}

uint8_t finecorsaDX(){
	 valore=0;
	for(i=0;i<6;i++){
		if((PINK & (1<<PK7))==0){
			valore=1;
			break;
		}
	}
	return valore;
}		
uint8_t finecorsaSX(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINK & (1<<PK5))==0){
			valore=1;
			break;
		}
	}
	return valore;
}
uint8_t finecorsaCentrale(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINK & (1<<PK6))==0){
			valore=1;
			break;
		}
	}
	return valore;
}

uint8_t finecorsaDietroDx(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINK & (1<<PK3))==0){
			valore=1;
			break;
		}
	}
	return valore;
}
uint8_t finecorsaDietroSx(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINK & (1<<PK2))==0){
			valore=1;
			break;
		}
	}
	return valore;
}


void initPulsanti(){
	DDRB&=~(1<<PB5);
 	PORTB|=(1<<PB5);
	DDRK=0;
	PORTK=0XFF;
}


void stampaPulsFinecorsa(){
	if(pulsante())Serial_Send_String("Pulsante \n");
	if(finecorsaCentrale())Serial_Send_String("Pala Davanti \n");
	if(finecorsaDX())Serial_Send_String("Baffo Dx \n");
	if(finecorsaSX())Serial_Send_String("Baffo Sx \n");
	if(finecorsaDietroDx())Serial_Send_String("Baffo Di Dx \n");
	if(finecorsaDietroSx())Serial_Send_String("Baffo Di Sx \n");
}

uint8_t ultimainclinazione, casella_inc;
float gradi_inc;
uint8_t inclinazione(){
	casella_inc=piano;
	gradi_inc=0;
	gradi_inc=gradiGiroscopio(asseY);
	if((gradi_inc>gradiSalita_errore)||(gradi_inc<gradiDiscesa_errore)){
		casella_inc=errore_inc;
	}
	else if((gradi_inc>gradiSalita)||((gradi_inc>gradiMinSalita)&&(ultimainclinazione==salita))){
		casella_inc=salita;
	}
	else if((gradi_inc<gradiDiscea)||((gradi_inc<gradiMinDiscesa)&&(ultimainclinazione==discesa))){
		casella_inc=discesa;
	}
	else{
		casella_inc=piano;
	}
	ultimainclinazione=casella_inc;
	return casella_inc;
}



/*----------------------*/