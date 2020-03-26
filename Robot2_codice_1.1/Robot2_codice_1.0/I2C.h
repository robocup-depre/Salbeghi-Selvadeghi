/*
 * I2C.h
 *
 * Created: 16/09/2017 23:30:53
 *  Author: Matteo
 */ 


#ifndef I2C_H_
#define I2C_H_
#ifndef F_CPU
	#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <stdbool.h>

	void error ();
	
	short startI2C();
	void waitACK();
	uint8_t checkSTART();
	short sendI2C(short dato);
	uint8_t checkMTSLAACK();
	uint8_t checkMRSLAACK();
	uint8_t checkDATAACK();
	void stop();
	uint8_t check_readNACK();
	uint16_t readNACK();
	uint8_t check_readACK();
	uint16_t readACK ();
	
	void init_I2C();

	short i2cRead(short slave, short address);

#endif /* I2C_H_ */