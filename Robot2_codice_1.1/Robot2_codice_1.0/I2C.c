/*
 * I2C.c
 *
 * Created: 16/09/2017 23:30:38
 *  Author: Matteo
 */ 
#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#define TWI_FREQ 100000
#include "I2C.h"
#include "serial.h"
 #include "NeoPixel.h"

void error ()
{
	ColoreLED2(RED);
	ColoreLED1(RED);
	ColoreLED3(RED);
	Serial_Send_String("e");
	PORTC=0;
}

short startI2C()//inizia la comunicazione I2C 
{
	TWCR|=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	waitACK();
	if (checkSTART()) {return 1;}			//restituisce 1 quando lo start non è avvenuto correttamente
	else {return 0;}
}

void waitACK()
{
	while(!(TWCR & (1<<TWINT)));
}

uint8_t checkSTART()
{
	if ( ( ( TW_STATUS) != TW_START) && ( TW_STATUS != TW_REP_START) )
	return 1; //TW_STATUS...... guarda util.twi.h
	else return 0;
}

short sendI2C(short dato)
{
	TWDR=dato;
	TWCR= (1<<TWINT) | (1<<TWEN);
	waitACK();
	if (checkDATAACK()&&checkMTSLAACK()&&checkMRSLAACK()) {return 1;}		//controlla se il dato restituito è uno di quelli possibili
		else {return 0;}
}

uint8_t checkMTSLAACK()
{
	if ( ( TW_STATUS) != TW_MT_SLA_ACK) return 1;
	else return 0;
}
uint8_t checkMRSLAACK()
{
	if ( ( TW_STATUS) != TW_MR_SLA_ACK) return 1;
	else return 0;
}

uint8_t checkDATAACK()
{
	if ( ( TW_STATUS) != TW_MT_DATA_ACK) return 1;
	else return 0;
}
void stop()
{
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}
uint8_t check_readNACK()//controllo lettura non sequenziale
{
	if(TW_STATUS!= TW_MR_DATA_NACK)return 1;
	else return 0;
}
uint16_t readNACK ()//lettura non sequenziale
{
	TWCR=(1<<TWINT)|(1<<TWEN);
	waitACK();
	uint16_t ret = (uint16_t)check_readNACK() << 8;//da questa riga di programma posso avere o un 1 o uno 0 shiftati di 8 verso sinistra
	return ret | TWDR;//qui prendo la variabile ret e la metto in or con il dato all'interno di TWDR
}
uint8_t check_readACK()//controllo lettura sequenziale
{
	if(TW_STATUS!= TW_MR_DATA_ACK)return 1;
	else return 0;
}
uint16_t readACK ()//lettura sequenziale da verificare appena avremmo un attimo
{
	TWCR=(1<<TWINT)|(1<<TWEA)|(1<<TWEN);
	waitACK();
	uint16_t ret = (uint16_t)check_readACK() << 8;//da questa riga di programma posso avere o un 1 o uno 0 shiftati di 8 verso sinistra
	return ret | TWDR;//qui prendo la variabile ret e la metto in or con il dato all'interno di TWDR
}

void init_I2C()
{
	TWSR=0;
	TWBR = ((F_CPU / TWI_FREQ) - 16) / (2*4);  //3=400KHz 18=100KHz
}


short i2cRead(short slave, short address)  //Legge 1 BYTE dal SLAVE e dall'ADDRESS indicati
{
	if(startI2C()) {stop(); return(0);}  //START
	if(sendI2C(slave|TW_WRITE)) {error(); stop();}  //WRITE MODE
	if (sendI2C(address & 0xff)) {error(); stop();}
	
	startI2C();  //REPEATED START
	if (sendI2C(slave|TW_READ)) {error(); stop();}  //READ MODE
	
	TWCR = (1<<TWINT) | (1<<TWEN);  //Attivo il FLAG e aspetto di ricevere il BYTE
	while (!(TWCR & (1<<TWINT)));
	
	stop();
	return(TWDR);
}