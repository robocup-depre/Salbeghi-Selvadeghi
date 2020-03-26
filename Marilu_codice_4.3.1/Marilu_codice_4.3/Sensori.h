

//SENSORI

void delay(int tempo);
//IR0

#define ArgentoMIN 800
#define BiancoMIN 550 //600   //NeroMIN
#define NeroMIN  200 //900
#define Bianco 0
#define Nero 1
#define Argento 2	

#define piano 0
#define salita 1
#define discesa 2
#define errore_inc 3
#define gradiSalita_errore 35//15
#define gradiDiscesa_errore -35//15
#define gradiSalita 10//15
#define gradiDiscea -10//-15
#define gradiMinDiscesa -5//-10
#define gradiMinSalita 5//10

uint8_t  inclinazione();

float DistanzaIR(uint8_t POS);

float Fotoresistenza();

uint8_t ColoreCasella();

uint8_t pulsante();
uint8_t finecorsaDX();
uint8_t finecorsaSX();
uint8_t finecorsaCentrale();
uint8_t finecorsaDietroDx();
uint8_t finecorsaDietroSx();
void initPulsanti();
void stampaPulsFinecorsa();


#define AvantiSinistra 0  //A0
#define AvantiDestra 1 //A1
#define DestraAvanti 2 //A2
#define DestraDietro 3 //A3
#define SinistraAvanti 4  //A4
#define SinistraDietro 5  //A5

float TemperaturaSX();
float TemperaturaDX();

