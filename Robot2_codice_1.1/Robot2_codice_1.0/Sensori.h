

//SENSORI
//IR0

#define ArgentoMIN 840
#define BiancoMIN 550 //600   //NeroMIN
#define NeroMIN    200 //900
#define Bianco 0
#define Nero 1
#define Argento 2	

#define piano 0
#define salita 1
#define discesa 2
#define errore_inc 3
#define gradiSalita_errore 35
#define gradiDiscesa_errore -35
#define gradiSalita 10//15
#define gradiDiscea -10//-15
#define gradiMinDiscesa -5//-10
#define gradiMinSalita 5//10

float DistanzaLaser(uint8_t ind);

uint8_t  inclinazione();

void init_Fotoresistenza();
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



#define AvantiSinistra 2 
#define AvantiDestra 2 
#define DestraAvanti 4 
#define DestraDietro 6 
#define SinistraAvanti 3 
#define SinistraDietro 5 

#define tipoLaser_AvSx 0 // 0 per corti, 1 per lunghi
#define tipoLaser_AvDx 0
#define tipoLaser_DxAv 0
#define tipoLaser_DxDi 0
#define tipoLaser_SxAv 0
#define tipoLaser_SxDi 0

float TemperaturaSX();
float TemperaturaDX();

