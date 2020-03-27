
#include <avr/io.h>
		//PARALLELO
#define pwmParallelo 1.5//32
#define tolleranzaParallelo 0.2//.7
#define tolleranzaMassimaParallelo 15
#define distanzaMinimaParalleo 20
void parallelototale();

		//GIRA
#define giriGiraInizio  1.4
#define giriGiraFine 0.4
#define gradiretta 30
#define angPwmGira	((float)giriGiraInizio-giriGiraFine)/(gradiretta)
void Gira(float);
int ultimoGira;

		//VITTIME
#define rileva_vittime 1
uint8_t statokit;
#define temperaturaMinVittime 30//36 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
uint8_t rilevaVittima();
void rilevaVittimaRampa();

		//AVANTI 30cm
float giriAvanti;
#define	kp_salita 0.08
void Avanti30cm(int vel);
	
		//FINECORSA

#define giriFinecorsa 1.1
char ControllaFinecorsa(int vel);
unsigned int countFinecorsa;

		//CASELLANERA
uint8_t ControllaCasellaNera(int statoazione);