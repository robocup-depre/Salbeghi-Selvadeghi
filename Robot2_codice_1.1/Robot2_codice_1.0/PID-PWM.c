#define F_CPU 16000000UL
#include "PID-PWM.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"
#include <util/atomic.h>
#include "BNO.h"


volatile double proporzionale[numeroMotori], derivata[numeroMotori], integrale[numeroMotori], errore[numeroMotori], olderrore[numeroMotori];
volatile uint8_t N;
volatile unsigned int TOP_PID;
volatile double kit, kdt, Kgiri, periodoPID_s, kdist;
volatile uint8_t NPEncB[numeroMotori];						//NUMERI PIN ENCODER FASE B
volatile uint8_t NPDri1[numeroMotori];						//NUMERI PIN Driver 1
volatile uint8_t NPDri2[numeroMotori];						//NUMERI PIN Driver 2
volatile unsigned int rip_pid[numeroMotori];
#define  avanti 1
#define  indietro -1
volatile unsigned long mINIZIO[numeroMotori], mFINE[numeroMotori], mRIPORTI[numeroMotori],  counter_riporti[numeroMotori];
volatile uint8_t mUTILIZZO[numeroMotori],counterImpulsi[numeroMotori];
volatile int8_t mDIREZIONE[numeroMotori];
volatile int8_t verso[numeroMotori], ok_pid;
volatile uint8_t cia, cib, cic, cid, cie;						//vari counter
volatile float k_millis;
volatile uint32_t v_millis;

ISR(TIMER1_COMPA_vect){
	for(cia=0;cia<numeroMotori;cia++) counter_riporti[cia]++;   //aggiorno contatore overflow
	N++;													//aumento variabile principale motori
	if(N>=numeroMotori) N=0;								//controllo 
	rip_pid[N]++;											//aumento contatore delle ripetizioni pid
	v_millis+=k_millis;
	if(mUTILIZZO[N]==0){									//se non è stato ultilizato il valore
		giri_temp[N]=(Kgiri/((double)(mFINE[N]-mINIZIO[N]+(mRIPORTI[N]*TOP_PID))))*mDIREZIONE[N]*verso[N];  //salvo i giri al secondo
		if((ABS(giri_temp[N])<max_giri)&&(ABS(giri_temp[N]-giri_old[N])<tolleranza_giri_temp)) //controllo sul numero, non considerato se è troppo altro o se è troppo alta la differenza con al valore precendente
			giri_attuali[N]=giri_temp[N];					//salvo il valore buono
		giri_old[N]=giri_temp[N];							//aggiorno il vechhio valore
		mUTILIZZO[N]=1;										//imposto che è stato utilizzato
		ok_pid=1;											//chiedo di fare il pid
	}else{
		if(ABS(giri_attuali[N])<=tolleranza_zero){			//se il valore è minore di un certo numero, viene considerato direttamente 0
			giri_attuali[N]=0;								//imposto a 0
		}else giri_attuali[N]+=giri_attuali[N]*km;			//funzione per portare a 0 i giri se no ci sono misurazioni
		if(rip_pid[N]>5) ok_pid=1;							//se il pid non viene eseguito da 5 volte si richiede l'esecuzione
	}
	distanza[N]+=kdist*giri_attuali[N];						//aggiorno la distanza 
  	if((giri_voluti[N]==0) | (ok_pid==1)){					//se è richiesto di fare il pid o se è richiesto che sia fermo
		 ok_pid=0;											//il pid è stato eseguito
		if((giri_voluti[N]==0 && giri_attuali[N]!=0) || (giri_voluti[N]!=0) ){ //se i giri voluti sono diversi da fermo o sono zero ma quelli attuali diversi da fermo
   			errore[N]=giri_voluti[N]-giri_attuali[N];		//calcolo errore PID
   			proporzionale[N]=errore[N]*kp;					//correzione proporzionale
   			integrale[N]+=kit*rip_pid[N]*errore[N];			//correzione integrale
   			derivata[N]=((errore[N]-olderrore[N])*kdt)/(rip_pid[N]);  //correzione derivata
   			olderrore[N]=errore[N];							//aggiorno il vecchio errore
   			if(integrale[N]>limiteIntegrale) integrale[N]=limiteIntegrale; //limito intergale
   			else if(integrale[N]<-limiteIntegrale) integrale[N]=-limiteIntegrale; 
   			correzionePID[N]=proporzionale[N]+integrale[N]+derivata[N]+pwm_attuali[N]; //sollo le correzione e gli scorsi pwm
   			if(correzionePID[N]>1023) correzionePID[N]=1023; //limito correzione per duty cicle
   			else if(correzionePID[N]<-1023) correzionePID[N]=-1023;
		}else{
   			correzionePID[N]=0;//gradiGiroscopio(asseY)*0;								//k*gradi(asseX); //metto a 0 o in relazione con l'inclinazione
   		}
		rip_pid[N]=0;										//azzero le ripetizioni del pid
   		pwm_attuali[N]=correzionePID[N];					//aggiorno i pwm
  		if(correzionePID[N]==0) PortaDriver=0xFF;			//se a zero  imposto la porta dei motori tutto a 1
  		else if(correzionePID[N]<0){						//se è minore di zero
 			correzionePID[N]=-correzionePID[N];				//la porto positiva
 			PortaDriver=(PortaDriver & ~(1<<NPDri2[N]))|(1<<NPDri1[N]);  //imposto direzione inversa
 		}else{
 			PortaDriver=(PortaDriver & ~(1<<NPDri1[N]))|(1<<NPDri2[N]);  //imposto direzione
 		}
  		if(N==MAvDx)      RegPwmMotAvDx=(int)(correzionePID[N]);			//imposto il duty cicle
  		else if(N==MDiDx) RegPwmMotDiDx=(int)(correzionePID[N]);
  		else if(N==MDiSx) RegPwmMotDiSx=(int)(correzionePID[N]);
  		else if(N==MAvSx) RegPwmMotAvSx=(int)(correzionePID[N]);
  	}
}

void fISR(uint8_t n){
	
 		counterImpulsi[n]++;								//aumeto numero misurazioni 
 		if(counterImpulsi[n]>=NumeroMisure){				//se maggiorni al numero impostato 
 			if((PinEncB & (1<<NPEncB[n]))==0)				//guardo stato seconda fase
 				mDIREZIONE[n]=avanti;						//se a 0 imposto la direzione ad avanti
  		else 
  				mDIREZIONE[n]=indietro;						// se diversa da 0 la imposto come  indietro
 			mINIZIO[n]=mFINE[n];							//aggirno misurazone di inizio
 			mFINE[n]=TCNT1;									//imposto misurazione di fine
 			counterImpulsi[n]=0;							//azzero numero misurazioni
 			mRIPORTI[n]=counter_riporti[n];					//imposto misurazione overflow con la variabile aggiornata nell'isr princiale
 			counter_riporti[n]=0;							//azzero il numero di overflow
 			mUTILIZZO[n]=0;									//imposto che non è stata utilizzata
 	}
}

void inizializzaCostanti(){
	periodoPID_s=(double)periodoPID/1000000.0;
	TOP_PID = F_CPU/(1000000.0/(periodoPID/numeroMotori))-1;
	kit=ki*periodoPID_s;
	kdt=kd/periodoPID_s;
	Kgiri = (double)(16000000.0*NumeroMisure)/NumeroImpulsiGiro;  //per calcolo giri al secondi
	kdist=(double)diametro_ruota*3.1416*periodoPID_s;             //per calcolo distanza percorsa
	k_millis=((float)periodoPID/4.0)/1000.0;
}

void init_PID(){
	resetta_PID();
	//TIMER1 e INT
		OCR1A=TOP_PID;
		TIMSK1=(1<<OCIE1A); //abilito ISR
	
		DDRD&=~((1<<PD3)|(1<<PD2)); //imposto porte INT2e3
		PORTD|=(1<<PD3)|(1<<PD2);	//INT2->pin19  INT3->pin18
		DDRE&=~((1<<PE4)|(1<<PE5)); //imposto porte INT4e5
		PORTE|=(1<<PE4)|(1<<PE5);	//INT4->pin2  INT5->pin3
	
		EICRA|=(1<<ISC21)|(1<<ISC20); //imposto il fronte di discsa per richiedere interruzione
		EICRA|=(1<<ISC31)|(1<<ISC30);
		EICRB|=(1<<ISC41)|(1<<ISC40);
		
		EICRB|=(1<<ISC51)|(1<<ISC50);
		EIMSK|=(1<<INT2)|(1<<INT3)|(1<<INT4)|(1<<INT5);
	//
	//PWM
		DDRE|=(1<<PE3);
		DDRH|=(1<<PH3)|(1<<PH4)|(1<<PH5);
		DDRPortaDriver=0xFF;
		PortaDriver=0xFF;
		DDRPinEncB &= ~((1<<PEncBAvDx)|(1<<PEncBAvSx)|(1<<PEncBDiDx)|(1<<PEncBDiSx));
		
		TCCR3A|= (1<<COM3A1)|(0<<COM3A0);
		TCCR3A|= (1<<WGM31)|(1<<WGM30);
		TCCR3B|= (1<<WGM32)|(0<<WGM33);
		
		TCCR4A|= (1<<COM4A1)|(0<<COM4A0);
		TCCR4A|= (1<<COM4B1)|(0<<COM4B0);
		TCCR4A|= (1<<COM4C1)|(0<<COM4C0);
		TCCR4A|= (1<<WGM41)|(1<<WGM40);
		TCCR4B|= (1<<WGM42)|(0<<WGM43);
		
		TCCR4B|= (0<<CS42)|(1<<CS41)|(1<<CS40);
		TCCR3B|= (0<<CS32)|(1<<CS31)|(1<<CS30);
	//
	//VETTORI COSTANTI
		NPEncB[MAvDx]=PEncBAvDx;
		NPEncB[MAvSx]=PEncBAvSx;
		NPEncB[MDiDx]=PEncBDiDx;
		NPEncB[MDiSx]=PEncBDiSx;
		NPDri1[MAvDx]=N1PAvDx;
		NPDri1[MAvSx]=N1PAvSx;
		NPDri1[MDiDx]=N1PDiDx;
		NPDri1[MDiSx]=N1PDiSx;
		NPDri2[MAvDx]=N2PAvDx;
		NPDri2[MAvSx]=N2PAvSx;
		NPDri2[MDiDx]=N2PDiDx;
		NPDri2[MDiSx]=N2PDiSx;
		verso[MAvDx]=VersoAvDx;
		verso[MAvSx]=VersoAvSx;
		verso[MDiDx]=VersoDiDx;
		verso[MDiSx]=VersoDiSx;
	//
	stop_PID();
	resetta_distanza();
	sei();
}


void stop_PID(){
	TCCR1B=0;
	PORTA=0xFF;
	RegPwmMotAvDx=0;
	RegPwmMotDiDx=0;
	RegPwmMotDiSx=0;
	RegPwmMotAvSx=0;
}

void start_PID(){
	TCCR1B=(1<<WGM12)|(1<<CS10);
}

void resetta_distanza(){
	for(cie=0;cie<numeroMotori;cie++) distanza[cie]=0;
}

void resetta_PID(){
	N=0;
	for (cib=0;cib<numeroMotori;cib++)
	{
		giri_attuali[cib]=0;
		olderrore[cib]=0;
		integrale[cib]=0;
		mDIREZIONE[cib]=0;
		mINIZIO[cib]=0;
		mFINE[cib]=0;
		mRIPORTI[cib]=0;
		mUTILIZZO[cib]=1;
		pwm_attuali[cib]=0;
		counterImpulsi[cib]=1;
		counter_riporti[cib]=0;
		//non fondamentali
		errore[cib]=0;
		olderrore[cib]=0;
		proporzionale[cib]=0;
		derivata[cib]=0;
		correzionePID[cib]=0;
	}
	inizializzaCostanti();
}

float distanzaPercorsa(){
	float dist = distanza[0];
	for(cic=1;cic<numeroMotori;cic++)
		if(dist>distanza[cic])
			dist= distanza[cic];
	return dist;
}
void StopMotori(){
	set_giri(0);
}

void set_giri(float voluti){
	for(cid=0;cid<numeroMotori;cid++){
		giri_voluti[cid]=voluti;
		
	}
}

void set_giri_coppia(float volutiDx, float volutiSx){
	giri_voluti[MAvSx]=volutiSx;
	giri_voluti[MDiSx]=volutiSx;
	giri_voluti[MAvDx]=volutiDx;
	giri_voluti[MDiDx]=volutiDx;	
}
double distanza_old;
void dritto(float centimetri, float giri){
	distanza_old = distanzaPercorsa();
	set_giri(giri);
	while(ABS((distanzaPercorsa()-distanza_old))<centimetri);
	StopMotori();
}

void reset_millis(){
	v_millis=0;
}
unsigned long millis(){
	return v_millis;
}