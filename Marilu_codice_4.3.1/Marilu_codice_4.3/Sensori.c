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


/*--------------SENSORI------------------*/

float UltimaDistanzaRilevata=1000;
float corto, lungo, differenza, dist;
float SFoto;



float Fotoresistenza(){
	SFoto=0;
	for(int i=0;i<5;i++){
		SFoto+=StartADC(7);
	}
	return (SFoto/5);
}

float DistanzaIR(uint8_t POS){//funzione per sensori sharp corti e lunghi
	float Valore=0;
	for(int i=0;i<3;i++)
	Valore += (float)StartADC(POS);
	Valore=Valore/3;
	 
	Valore=((13*1023)/(5.0*Valore));
	if (Valore>35)
	{
		Valore=1000;
	}

	return Valore;
}


float TemperaturaDX(){   //temperaturaTpa81DX
	int devDX=0xDE;//6F;//sens dx
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
	float valore1=Fotoresistenza();
	Serial_Send_String("   ");Serial_Send_Float(valore1);Serial_Send_String("     \n");
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
		if((PING & (1<<PG1))==0){
			valore=1;
			break;
		}
	}
	return valore;
}

uint8_t finecorsaDX(){
	 valore=0;
	for(i=0;i<6;i++){
		if((PINB & (1<<PB0))==0){
			valore=1;
			break;
		}
	}
	return valore;
}		
uint8_t finecorsaSX(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINB & (1<<PB2))==0){
			valore=1;
			break;
		}
	}
	return valore;
}
uint8_t finecorsaCentrale(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINB & (1<<PB1))==0){
			valore=1;
			break;
		}
	}
	return valore;
}

uint8_t finecorsaDietroDx(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINK & (1<<PK0))==0){
			valore=1;
			break;
		}
	}
	return valore;
}
uint8_t finecorsaDietroSx(){
	valore=0;
	for(i=0;i<6;i++){
		if((PINK & (1<<PK1))==0){
			valore=1;
			break;
		}
	}
	return valore;
}

void initPulsanti(){
	DDRB&=~((1<<PB1)|(1<<PB2)|(1<<PB0));
	PORTB|=(1<<PB1)|(1<<PB2)|(1<<PB0);
	DDRG&=~((1<<PG1));
	PORTG|=(1<<PG1);
	DDRK&=~((1<<PK0)|(1<<PK1));
	PORTK|=(1<<PK0)|(1<<PK1);
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