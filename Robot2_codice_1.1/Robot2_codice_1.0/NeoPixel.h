enum colori{
 	RED,
 	BLUE, 
 	YELLOW, 
 	WHITE, 
 	ORANGE, 
 	GREEN, 
 	SPENTO, 
 	PINKColor, 
 	PURPLE, 
 	GRAY
};

#define LED_COUNT_FOTORESISTENZA 4
#define LED_COUNT_SEGNALE 3

typedef struct rgb_colors
{
	unsigned char red, green, blue;
} rgb_color;


//id ColoreFotoresistenza(uint8_t R, uint8_t G, uint8_t B );

void ColoreLED1(enum colori col);
void ColoreLED2(enum colori col);
void ColoreLED3(enum colori col);
//void ColoreFotoresistenza(uint8_t R, uint8_t G, uint8_t B);
void SpegniLED1();
void SpegniLED2();
void SpegniLED3();
//void SpegniFotoresistenza();

uint8_t LedGara;