#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "Movimenti.h"
#include "Sensori.h"
#include "serial.h"
#include "NeoPixel.h"
#include "PID-PWM.h"
#include "Kit.h"
#include "BNO.h"

/*--------------FUNZIONI MOTORI----------*/
//variabili 
	//gira
		float GIniziali, GUltimo, GAdesso, GMancanti, velGyZ, GiriGira ;
		float Gr;
		int k;
		int8_t VersoGira;
	//parallelo
		double temp;
		float DiAv, DiDi, differenza;
	//rileva vittime
		float tDx, tSx, diSx, diDx;
		uint8_t vitt_rampa_dx, vitt_rampa_sx;
	//casella nera	
		uint8_t controllo_colore;
	//finecorsa
		uint8_t FC, FDx, FSx;
	//avanti 30cm
		float distanzaCorrente, distanzaAvSX, distanzaAvDX, distanzaDxAv, distanzaSxAv, correzione;
		int dist_av, inc_av, dist_pr;



void Gira(float Gradi){
	set_giri(0);
	GIniziali=gradiGiroscopio(asseZ);
	k=0;
	GUltimo=GIniziali;
	if((int)Gradi%90==0){
		Gr=Gradi-GIniziali;
		if(GIniziali>45 && GIniziali<=135) Gr+=90;
		else if(GIniziali>135 && GIniziali<=225) Gr+=180;
		else if(GIniziali>225 && GIniziali<=315) Gr+=270;
		if(Gradi>=0){
			if(Gr<0)Gr+=360;
			}else{
			if(Gr<-360)Gr+=360;
			if(Gr>0)Gr-=360;
		}
		Gradi=Gr;
	}
	if(Gradi>0)
	VersoGira=1;
	else VersoGira =-1;
	set_giri_coppia(giriGiraInizio*VersoGira*-1.0,giriGiraInizio*VersoGira);
	do{
		GAdesso=gradiGiroscopio(asseZ);
		if((GAdesso-GUltimo)>180) k--;
		else if((GAdesso-GUltimo)<-180) k++;
		GMancanti=Gradi-GAdesso-(k*360)+GIniziali;
		if(Gradi<0)GMancanti*=-1;
		if((GMancanti<=gradiretta)&&(GMancanti>=-5))
		GiriGira=(GMancanti*angPwmGira)+giriGiraFine;
		else
		GiriGira=giriGiraInizio;
		set_giri_coppia(GiriGira*VersoGira*-1.0,GiriGira*VersoGira);
		GUltimo=GAdesso;
	}while(GMancanti>0);
	StopMotori();
	if((90==ABS(Gradi)||(Gradi==180)))
	ultimoGira=Gradi;
}


void parallelototale(){  //prove a cambiare valori di pwm e tollerranza e se non si riese eliminare
// 	Serial_Send_String("Parallelo \n");
// 	for (int i=-1;i<=1;i+=2)
// 	{
// 		set_giri(0);
// 		if(i==1){
// 			DiAv=DistanzaLaser(DestraAvanti);
// 			DiDi=DistanzaLaser(DestraDietro);
// 		}else{
// 			DiAv=DistanzaLaser(SinistraAvanti);
// 			DiDi=DistanzaLaser(SinistraDietro);
// 		}
// 		differenza = (DiAv-DiDi)*i;
// 		while(!((ABS(differenza)<tolleranzaParallelo)||(ABS(differenza)>tolleranzaMassimaParallelo)||(((DiAv+DiDi)/2)>distanzaMinimaParalleo)))
// 		{	
// 			Serial_Send("par\n");
// 		    if(i==1){
// 			    DiAv=DistanzaLaser(DestraAvanti);
// 			    DiDi=DistanzaLaser(DestraDietro)-0.3;
// 		    }else{
// 			    DiAv=DistanzaLaser(SinistraAvanti);
// 			    DiDi=DistanzaLaser(SinistraDietro)-0.3;
// 		    }
// 		    differenza = (DiAv-DiDi)*i ;
// 			temp=differenza*pwmParallelo+0.2;
// 			if(temp>max_giri)temp=max_giri;
// 			set_giri_coppia(-temp,temp);
// 		}
// 		set_giri(0);
// 	}
// 	Serial_Send("fine parallelo \n");
}


uint8_t rilevaVittima(){
	uint8_t valore =0;
	if(rileva_vittime==1){
		if(statokit==1){
			tSx=TemperaturaSX();
			diSx= (DistanzaLaser(SinistraDietro)+DistanzaLaser(SinistraAvanti))/2;
			if ((tSx>=temperaturaMinVittime)&&(diSx<20)){
				StopMotori();
				valore=1;
				statokit=0;
				rilasciakit(Sinistra,1);
			}
			tDx=TemperaturaDX();
			diDx= (DistanzaLaser(DestraDietro)+DistanzaLaser(DestraAvanti))/2;
			if ((tDx>=temperaturaMinVittime)&&(diDx<20)){
				StopMotori();
				valore=1;
				statokit=0;
				rilasciakit(Destra,1);
			}
		}
	}
	return valore;
}

void rilevaVittimaRampa(){
	if(rileva_vittime==1){
		tSx=TemperaturaSX();
		tDx=TemperaturaDX();
		if((tSx>=temperaturaMinVittime) && (vitt_rampa_sx==0)){
			vitt_rampa_sx=1;
			StopMotori();
			rilasciakit(Sinistra,1);
		}
		else if((tDx>=temperaturaMinVittime) && (vitt_rampa_dx==0)){
			vitt_rampa_dx=1;
			StopMotori();
			rilasciakit(Destra,1);
		}
	}
}

char ControllaFinecorsa(int vel){
	char val=0;
	if(countFinecorsa<=4){ 
		if(DistanzaLaser(AvantiDestra)>=6.5  &&  DistanzaLaser(AvantiSinistra)>=6.5){
			FC=finecorsaCentrale();
			FDx=finecorsaDX();
			FSx=finecorsaSX();
			if((FC==1)||((FDx==1)&&(FSx==1))){
				ColoreLED2(PURPLE);
				dritto(4,-1.2);
				Gira(-ultimoGira);
				parallelototale();
				Avanti30cm(vel);
				val=1;
			}
			FDx=finecorsaDX();
			if(FDx==1){
				ColoreLED2(ORANGE);
				dritto(5,-1);
				Gira(-35);
				dritto(16,1);
				Gira(0);
				dritto(12,-1);
				parallelototale();
				ColoreLED2(SPENTO);
				Avanti30cm(vel);
				val=1;
			}
			FSx=finecorsaSX();
			if(FSx==1){
				ColoreLED2(PINKColor);
				set_giri(-giriFinecorsa);//cambiare
				dritto(5,-1);
				Gira(35);
				dritto(16,1);
				Gira(-0.1);
				dritto(12,-1);
				parallelototale();
				ColoreLED2(SPENTO);
				Avanti30cm(vel);
				val=1;
			}
		}
	}else
	{
		val=0;
		countFinecorsa=0;
	}
	if(val==1)countFinecorsa++;
	return val;
}




void Avanti30cm(int vel){
	ColoreLED2(GRAY);
	distanzaAvDX = DistanzaLaser(AvantiDestra);
	distanzaAvSX = DistanzaLaser(AvantiSinistra);
	if(distanzaAvSX<distanzaAvDX)
		distanzaCorrente=distanzaAvSX;
	else
		distanzaCorrente=distanzaAvDX;
	inc_av=0;
	resetta_distanza();
	//provare a fare battere il robot con i finercorsa avanti invece di guardare che la distanza sia minore di 5 e poi farlo indietreggiare di 5cm 
	while(!((distanzaCorrente<=5) || (((distanzaCorrente>20) && (distanzaPercorsa())>=30) && (inc_av==piano)))){
		distanzaAvDX = DistanzaLaser(AvantiDestra);
		distanzaAvSX = DistanzaLaser(AvantiSinistra);
		distanzaCorrente=(distanzaAvSX<distanzaAvDX)? distanzaAvSX:distanzaAvDX;
		
		inc_av=inclinazione();
		while(inc_av==errore_inc){
			set_giri(-1);
			inc_av=inclinazione();
		}
		if(inc_av!=piano){
			//rilevaVittimaRampa();
			distanzaDxAv=DistanzaLaser(DestraAvanti);
			distanzaSxAv=DistanzaLaser(SinistraAvanti);
			if(distanzaSxAv<15 && distanzaDxAv<15 )
				correzione=kp_salita*(distanzaDxAv-distanzaSxAv);
			else
				correzione=0;
			if(correzione>0.5)correzione=0;
			set_giri_coppia(0.8-correzione,0.8+correzione);
		}else{
			/*PID GIROSCOPIO DA VEDERE POI
							GA=gradiGiroscopio(asseZ);
							if((GA-GU)>180) kg--;
							else if((GA-GU)<-180) kg++;
							GU=GA;
							GA+=(kg*360);
							err=(GA-point);
							propg = err*kpg;
							integ += err*kig;
							m=millis();
							derg= ((err-olderr)/((m-oldm)/1000.0))*kdg;
							//Serial_Send(m-oldm); Serial_Send(" ");
							oldm=m;
							olderr=err;
							if(integ>limint)integ=limint;
							if(integ<-limint)integ=-limint;
							//	integ=0;
							corr=propg+integ+derg;
							oldc=corr;
							// 			Serial_Send(err); Serial_Send(" ");
							// 			Serial_Send(propg*10); Serial_Send(" ");
							// 			Serial_Send(integ*10); Serial_Send(" ");
							// 			Serial_Send(derg*10); Serial_Send(" ");
							// 			Serial_Send(corr*10); Serial_Send(" ");
							set_giri_coppia(1+corr, 1-corr);
			*/
			set_giri(vel);	
			if(ColoreCasella()!=Nero){
				rilevaVittima();
				if(ControllaFinecorsa(vel)) break;
			}
		}
	}
	vitt_rampa_dx=0;
	vitt_rampa_sx=0;
	StopMotori();
	statokit=1;
	parallelototale();
}
