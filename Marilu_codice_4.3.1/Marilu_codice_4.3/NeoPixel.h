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

#define LED_COUNT 1

typedef struct rgb_color
{
	unsigned char red, green, blue;
} rgb_color;

rgb_color colors[LED_COUNT];

void led_color(rgb_color * colors);
void led1_color(rgb_color * colors);

void ColoreLED(enum colori);
void ColoreLED1(enum colori);

void SpegniLED1();
void SpegniLED();

uint8_t LedGara;