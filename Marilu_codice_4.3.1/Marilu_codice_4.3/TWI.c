


 #ifndef F_CPU
	#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include "i2cmaster.h"
#include <util/twi.h>
#include <compat/twi.h>
#include "TWI.h"
#define TWI_FREQ 100000
volatile int TWI_stat;

int TWI_status(int stato)
{
if(stato<=1)
{
TWI_stat=stato;
}
if(stato>1){return TWI_stat;}
return 0;
}

void TWI_init()
{
	TWSR=0;
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;  //12=400KHz 72=100KHz
}

void TWI_stop()
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);  //Invio il bit di STOP
}

short TWI_start()
{
	TWCR |= (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);  //Invio il bit di START 
	while (!(TWCR & (1<<TWINT)));  //Attendo che il comando sia inviato
	
	if((TWSR!=0x08)&&(TWSR!=0x10))  //Controllo se il comando inviato è lo START oppure R-START
	{
		return(1);
	}
	return(0);
}

short TWI_write(short data)
{
	TWDR = data;	//Carico il Byte da inviare nel registro TWDR
	TWCR = (1<<TWINT) | (1<<TWEN);  //Attivo il flag e attendo l'invio
	while (!(TWCR & (1<<TWINT)));
	
	if((TWSR!=0x18)&&(TWSR!=0x28)&&(TWSR!=0x40))  //controllo la riuscita dell'invio del dato
	{
		return(1);
	}
	return(0);
}

short TWI_read(short slave, short address)  //Legge 1 BYTE dal SLAVE e dall'ADDRESS indicati
{
	if(TWI_start()) {TWI_stop(); return(0);}  //START
	if(TWI_write(slave)) {TWI_stop(); return(0);}  //WRITE MODE
	if(TWI_write(address)) {TWI_stop(); return(0);}  //ADDRESS
	
	TWI_start();  //REPEATED START
	if(TWI_write(slave|1)) {TWI_stop(); return(0);}  //READ MODE
	
	TWCR = (1<<TWINT) | (1<<TWEN);  //Attivo il FLAG e aspetto di ricevere il BYTE
	while (!(TWCR & (1<<TWINT)));
	
	TWI_stop();
	return(TWDR);
}

short TWI_read2(short slave, short address)  //Legge 2 BYTE dal SLAVE partendo dall'ADDRESS indicato
{
	if(TWI_start()) {TWI_stop(); return(0);}  //START
	if(TWI_write(slave)) {TWI_stop(); return(0);}  //WRITE MODE
	if(TWI_write(address)) {TWI_stop(); return(0);}  //ADDRESS
	
	TWI_start();  //REPEATED START
	if(TWI_write(slave|1)) {TWI_stop(); return(0);}  //READ MODE attivato con |1
	
	TWCR = (1<<TWINT) | (1<<TWEN)|(1<<TWEA);  //Attivo il FLAG e aspetto di ricevere il BYTE senza mandare lo stop alla fine (1<<TWEA)
	while (!(TWCR & (1<<TWINT)));
	uint8_t val1=TWDR;
	TWCR = (1<<TWINT) | (1<<TWEN);  //Attivo il FLAG e aspetto di ricevere il BYTE
	while (!(TWCR & (1<<TWINT)));
	uint8_t val2=TWDR;
	TWI_stop();
	return ((val2 << 8) | val1); //Unisce i due valori complementari
}

int TWI_reads(short slave, short address,int num,short *var)  //Legge 2 BYTE dal SLAVE partendo dall'ADDRESS indicato
{
	if(TWI_start()) {TWI_stop(); return(0);}  //START
	if(TWI_write(slave)) {TWI_stop(); return(0);}  //WRITE MODE
	if(TWI_write(address)) {TWI_stop(); return(0);}  //ADDRESS
	
	TWI_start();  //REPEATED START
	if(TWI_write(slave|1)) {TWI_stop(); return(0);}  //READ MODE attivato con |1
	for(int i=0;i<num;i++)
	{
	if(i<num-1)
	{
	TWCR = (1<<TWINT) | (1<<TWEN)|(1<<TWEA);  //Attivo il FLAG e aspetto di ricevere il BYTE senza mandare lo stop alla fine (1<<TWEA)
	while (!(TWCR & (1<<TWINT)));
	*(var+i)=TWDR;
	}
	else
	{
	TWCR = (1<<TWINT) | (1<<TWEN);  //Attivo il FLAG e aspetto di ricevere il BYTE
	while (!(TWCR & (1<<TWINT)));
	*(var+i)=TWDR;
	}
	}
	TWI_stop();
}

short TWI_send(short slave, short address, short data)  //Scrivo 1 BYTE sullo SLAVE all'ADDRESS indicato
{
	if(TWI_start()) {TWI_stop(); return(0);}  //START
	if(TWI_write(slave)) {TWI_stop(); _delay_ms(5); return(0);}  //WRITE MODE
	if(TWI_write(address)) {TWI_stop(); _delay_ms(5); return(0);}  //ADDRESS
	if(TWI_write(data)) {TWI_stop(); _delay_ms(5); return(0);}  //Invio il BYTE da scrivere
	TWI_stop();
	_delay_ms(5);  //delay tra 1 operazione e l'altra
	return(1);
}

short TWI_text(short slave, short address, char *data)
{
	int k=0;
	
	while(data[k])
	{
		if(TWI_start()) {TWI_stop(); return(0);}  //START
		if(TWI_write(slave)) {TWI_stop(); _delay_ms(5); return(0);}
		if(TWI_write(address+k)) {TWI_stop(); _delay_ms(5); return(0);}
		if(TWI_write((short)data[k])) {TWI_stop(); _delay_ms(5); return(0);}
		TWI_stop();
		_delay_ms(5);
		k++;
	}
	return(1);
}

short TWI_test()
{
	return(0);
}


/*-------------------------------------*/

//da i2c master
void i2c_init(void)
{
	/* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
	
	TWSR = 0;                         /* no prescaler */
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */

	}/* i2c_init */
	

unsigned char i2c_start(unsigned char address)
{
	uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

	}/* i2c_start */
	
void i2c_start_wait(unsigned char address)
{
	uint8_t   twst;


	while ( 1 )
	{
		// send START condition
		TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
		
		// wait until transmission completed
		while(!(TWCR & (1<<TWINT)));
		
		// check value of TWI Status Register. Mask prescaler bits.
		twst = TW_STATUS & 0xF8;
		if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
		
		// send device address
		TWDR = address;
		TWCR = (1<<TWINT) | (1<<TWEN);
		
		// wail until transmission completed
		while(!(TWCR & (1<<TWINT)));
		
		// check value of TWI Status Register. Mask prescaler bits.
		twst = TW_STATUS & 0xF8;
		if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) )
		{
			/* device busy, send stop condition to terminate write operation */
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
			
			// wait until stop condition is executed and bus released
			while(TWCR & (1<<TWSTO));
			
			continue;
		}
		//if( twst != TW_MT_SLA_ACK) return 1;
		break;
	}

	}/* i2c_start_wait */

unsigned char i2c_rep_start(unsigned char address)
{
	return i2c_start( address );

	}/* i2c_rep_start */


void i2c_stop(void)
{
	/* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

	}/* i2c_stop */
	
unsigned char i2c_write( unsigned char data )
{
	uint8_t   twst;
	
	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

	}/* i2c_write */

unsigned char i2c_readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));

	return TWDR;

	}/* i2c_readAck */


unsigned char i2c_readNak(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
	return TWDR;

	}/* i2c_readNak */
