
#define SERIAL
#include <avr/io.h>
#include <stdarg.h>
 
#define USART0_BAUDRATE 1000000
#define USART1_BAUDRATE 115200
#define USART2_BAUDRATE 115200
 
 
float ABS(float x);
float limita(float x, float I, float F);
 
void Serial_Init();//Init
void Serial_Tx( unsigned char data );//TRASMISSIONE
unsigned char Serial_Rx( void );//RICEVI
void Serial_Send_Int(int32_t num);//NUMERO
void Serial_Send_String(char *string1);//STRINGA
void Serial_Send_Float(float val); 
void SerialN();//NUOVA RIGA

void Serial1_Init();
void Serial1_Tx(unsigned char data);
unsigned char Serial1_Rx( void );//RICEVI
void Serial1_Send_Int(int32_t num);
void Serial1_Send_String(char *string1);
void Serial1_Send_Float(float data);
void Serial1N();


void Serial2_Init();//Init
void Serial2_Tx( unsigned char data );//TRASMISSIONE
unsigned char Serial2_Rx( void );//RICEVI
void Serial2N();

#define Serial_Send(_1) _Generic(_1,int: Serial_Send_Int,char*: Serial_Send_String,float: Serial_Send_Float,char: Serial_Tx,double: Serial_Send_Float,unsigned char: Serial_Send_Int,unsigned long: Serial_Send_Int,unsigned int: Serial_Send_Int)(_1)
#define Serial1_Send(_1) _Generic(_1,int: Serial1_Send_Int,char*: Serial1_Send_String,float: Serial1_Send_Float,char: Serial1_Tx,double: Serial1_Send_Float,unsigned char: Serial1_Send_Int,unsigned long: Serial1_Send_Int,unsigned int: Serial1_Send_Int)(_1)
