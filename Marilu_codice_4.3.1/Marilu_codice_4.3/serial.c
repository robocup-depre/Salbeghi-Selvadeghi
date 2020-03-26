//
#define F_CPU 16000000ul
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include "serial.h"

long BAUD0 = (F_CPU/16/USART0_BAUDRATE)-1;
long BAUD1 = (F_CPU/16/USART1_BAUDRATE)-1;
long BAUD2 = (F_CPU/16/USART2_BAUDRATE)-1;


float ABS(float x){
	if(x<0) x*=-1;
	return x;
}

float limita(float x, float I, float F){
	if(F>I){
		if(x<I)x=I;
		if(x>F)x=F;
		}else{
		if(x>I)x=I;
		if(x<F)x=F;
	}
	return x;
}



void Serial_Init(){
 	UBRR0H = (unsigned char)(BAUD0>>8); 
 	UBRR0L = (unsigned char)BAUD0;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = 0b00000110;
} 
void Serial_Tx(unsigned char data)
{
	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0=data;
}
unsigned char Serial_Rx( void )
{
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}

void Serial_Send_Int(int32_t num)
{
	if(num<0)
	{
		Serial_Send_String("-");
		num=-num;
	}
	if(num==0){Serial_Tx('0');}
	else
	{
		char str[32];				// definisce una stringa sulla quale convertire il numero da trasmettere (max 10 cifre)
		char i;						// contatore ciclo
		for(i=0;i<32;i++) str[i]=0; // cancella la stringa
		i=31;
		while (num)
		{
			str[i]=num%10+'0';		// converte il numero da trasmettere in una stringa (dalla cifra meno significativa)
			num/=10;
			i--;
		}
		for (i=0;i<32;i++)			// invia la stringa un carattere alla volta
		if (str[i]) Serial_Tx(str[i]);
	}
}


void Serial_Send_String(char *string1)
{
	int i=0;
	for(i=0;i<strlen(string1);i++)
	{
		Serial_Tx(string1[i]);
	}
}
void SerialN()
{
	Serial_Tx(13);
	Serial_Tx(10);
}
void Serial_Send_Float(float data)
{
	if(data<0)
	{
		Serial_Send_String("-");
		data=-data;
	}
	int size=3;
	int var=data;
	float data1=data*pow(10,size);
	Serial_Send_Int(var);
	Serial_Send_String(".");
	var=var*pow(10,size);
	data1=data1-var;
	var=data1;
	Serial_Send_Int(var);
}





void Serial1_Init()
{
UBRR1H = (unsigned char)(BAUD1>>8);  
UBRR1L = (unsigned char)BAUD1;
UCSR1B = (1<<RXEN1)|(1<<TXEN1);
UCSR1C = 0b00000110;
}
void Serial1_Tx(unsigned char data)
{
	while ( !( UCSR1A & (1<<UDRE1)) );
	UDR1=data;
}
unsigned char Serial1_Rx( void )
{
	while ( !(UCSR1A & (1<<RXC1)) );
	return UDR1;
}

void Serial1_Send_Int(int32_t num)
{
	if(num<0)
	{
		Serial1_Send_String("-");
		num=-num;
	}
	if(num==0){Serial1_Tx('0');}
	else
	{
		char str[32];				// definisce una stringa sulla quale convertire il numero da trasmettere (max 10 cifre)
		char i;						// contatore ciclo
		for(i=0;i<32;i++) str[i]=0; // cancella la stringa
		i=31;
		while (num)
		{
			str[i]=num%10+'0';		// converte il numero da trasmettere in una stringa (dalla cifra meno significativa)
			num/=10;
			i--;
		}
		for (i=0;i<32;i++)			// invia la stringa un carattere alla volta
		if (str[i]) Serial1_Tx(str[i]);
	}
}

void Serial1_Send_String(char *string1)
{
	int i=0;
	for(i=0;i<strlen(string1);i++)
	{
		Serial1_Tx(string1[i]);
	}
}

void Serial1_Send_Float(float data)
{
	if(data<0)
	{
		Serial1_Send_String("-");
		data=-data;
	}
	int size=3;
	int var=data;
	float data1=data*pow(10,size);
	Serial1_Send_Int(var);
	Serial1_Send_String(",");
	var=var*pow(10,size);
	data1=data1-var;
	var=data1;
	Serial1_Send_Int(var);
}
void Serial1N()
{
	Serial1_Tx(13);
	Serial1_Tx(10);
}



void Serial2_Init(){
	UBRR2H = (unsigned char)(BAUD2>>8);  
	UBRR2L = (unsigned char)BAUD2;
	UCSR2B = (1<<RXEN2)|(1<<TXEN2);
	UCSR2C = 0b00000110;
} 

void Serial2_Tx(unsigned char data)
{
	while ( !( UCSR2A & (1<<UDRE2)) );
	UDR2=data;
}

unsigned char Serial2_Rx( void )
{
	while ( !(UCSR2A & (1<<RXC2)) );
	return UDR2;
}
void Serial2N()
{
	Serial2_Tx(13);
	Serial2_Tx(10);
}


