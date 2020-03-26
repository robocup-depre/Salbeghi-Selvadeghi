
#include <avr/io.h>
		//PARALLELO
#define pwmParallelo 1.5//32
#define tolleranzaParallelo 0.2//.7
#define tolleranzaMassimaParallelo 15
#define distanzaMinimaParalleo 20
void parallelototale();

float giriAvanti;
uint8_t Rotazioni;
		//GIRA
#define giriGiraInizio  1.4
#define giriGiraFine 0.3
#define gradiretta 30
#define angPwmGira	((float)giriGiraInizio-giriGiraFine)/(gradiretta)
void Gira(float);
int ultimoGira;

		//VITTIME
#define rileva_vittime 1
#define tempMovRilKit 120 //tempo indieto nel movimento per rilasciare il kit
uint8_t statokit;
#define temperaturaMinVittime 35//36 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
uint8_t rilevaVittima();
void rilevaVittimaRampa();

		//AVANTI 30cm
#define kp_salita 0.1
#define kp_AvGiro 0.05
#define ki_AvGiro 0.0005
#define kd_AvGiro 0.002
#define limI_AvGiro 0.5
void Avanti30cm(int vel);
			//con sensori
	
		//FINECORSA
#define gradiFinecorsa 28
#define tempoFinecorsa 400
#define tempoFinecorsaSoloIndietro 150
#define tempoFinecorsaCentrale 600
#define giriFinecorsa 1.1
char ControllaFinecorsa(int vel);
unsigned int countFinecorsa;

		//CASELLANERA
uint8_t ControllaCasellaNera(int statoazione);