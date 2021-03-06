//#define F_CPU 16000000UL
#include <avr/io.h>
//costanti PID a 1 /s
#define kp 150.0//50//150//150//5//5//150   //150
#define ki 2//0.02//0.015             //0.002*2
#define kd 7//700//500//200//1000/    //7/0.002
#define km -0.1

//limiti correzzioni PID
#define limiteProp 200//100//50//200
#define limiteIntegrale 0.15//0.15//0.15//1//0.3
#define limiteDerivata 200//20//200

//principali PID
#define periodoPID 10000.0 //us max:4000*numromotori
#define numeroMotori 4 //max 8
#define NumeroMisure 3.0
#define  NumeroImpulsiGiro 990.0
#define diametro_ruota 7.0
#define tolleranza_zero 0.1
#define max_giri 2.5
#define tolleranza_giri_temp 0.5


volatile double correzionePID[numeroMotori],  giri_temp[numeroMotori],  giri_old[numeroMotori];
//variabili pubbliche
volatile double giri_attuali[numeroMotori], giri_voluti[numeroMotori];  
volatile double distanza[numeroMotori];
volatile double  pwm_attuali[numeroMotori];

//definizione nome motori
#define MAvDx 0  
#define MAvSx 1	
#define MDiDx 2	
#define MDiSx 3	

//definizione registri pwm motori
#define RegPwmMotAvDx OCR4B
#define RegPwmMotAvSx OCR4A
#define RegPwmMotDiDx OCR3A
#define RegPwmMotDiSx OCR4C

//definizione interrup esterni motori (FASE A)
#define nINT2 MDiDx   
#define nINT3 MDiSx
#define nINT5 MAvDx
#define nINT4 MAvSx


//definizione porta fase B e numero pin su porta
#define PinEncB PINL
#define DDRPinEncB DDRL
#define PEncBAvDx PL5  
#define PEncBAvSx PL4
#define PEncBDiDx PL6
#define PEncBDiSx PL7
//verde pin 45 
//definizione settaggio porta driver
#define PortaDriver PORTA
#define DDRPortaDriver DDRA
#define N1PDiSx PA4  
#define N2PDiSx PA5
#define N1PDiDx PA3
#define N2PDiDx PA2
#define N1PAvSx PA1
#define N2PAvSx PA0
#define N1PAvDx PA7
#define N2PAvDx PA6

//definizone TOP PWM
#define  TopPwmMot (1023/100)

//definizione verso motori
#define VersoAvDx -1
#define VersoAvSx 1
#define VersoDiDx -1 
#define VersoDiSx 1
//funzioni
void init_PID();
void stop_PID();
void start_PID();
void resetta_PID();
void fISR(uint8_t n);

void resetta_distanza();

//void setPWMn(float PWM, uint8_t N);

void set_giri(float voluti);
void set_giri_coppia(float volutiDx, float volutiSx);


void dritto(float centimetri, float giri);




float distanzaPercorsa();
void StopMotori();


//millis

void reset_millis();
unsigned long millis();


#define k_giri_salita 0.5