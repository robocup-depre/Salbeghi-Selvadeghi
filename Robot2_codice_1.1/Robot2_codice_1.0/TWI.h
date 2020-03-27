


#ifndef F_CPU
    #define F_CPU 16000000UL
#endif
#include <avr/io.h>

#ifndef TWI_LIB_H_
#define TWI_LIB_H_
#define TWI_TIMEOUT 100
//da i2c_master
#define SCL_CLOCK 50000L
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

short TWI_start();
short TWI_write(short data);
void TWI_stop();
short TWI_read(short slave, short address);
short TWI_read2(short slave, short address);
int  TWI_reads(short slave,short address,int num,short *var);
short TWI_send(short slave, short address, short data);
short TWI_text(short slave, short address, char *data);
void TWI_init();
int TWI_status(int stato);

//da I2c master
void i2c_init();
unsigned char i2c_start(unsigned char address);
void i2c_start_wait(unsigned char address);
unsigned char i2c_rep_start(unsigned char address);
void i2c_stop();
unsigned char i2c_write( unsigned char data );
unsigned char i2c_readAck();
unsigned char i2c_readNak();

#endif /* TWI_LIB_H_ */