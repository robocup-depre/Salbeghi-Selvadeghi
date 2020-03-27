
#define F_CPU 16000000ul
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "NeoPixel.h"
#define LED_FOTO_STRIP_PORT PORTB
#define LED_FOTO_STRIP_DDR  DDRB
#define LED_FOTO_STRIP_PIN  PB7
#define LED_S_STRIP_PORT PORTB
#define LED_S_STRIP_DDR  DDRB
#define LED_S_STRIP_PIN  PB6

rgb_color colore_fotoresistenza[LED_COUNT_FOTORESISTENZA];
rgb_color colore_seganle[LED_COUNT_SEGNALE];


// 
// void SpegniFotoresistenza(){
// 	ColoreFotoresistenza(0,0,0);
// }

void SpegniLED1(){
	ColoreLED1(SPENTO);
}
void SpegniLED2(){
	ColoreLED2(SPENTO);
}
void SpegniLED3(){
	ColoreLED3(SPENTO);
}
 
void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count)
{
		LED_S_STRIP_PORT &= ~(1<<LED_S_STRIP_PIN);
		LED_S_STRIP_DDR |= (1<<LED_S_STRIP_PIN);
		cli();
		while(count--)
		{
			asm volatile(
			"ld __tmp_reg__, %a0+\n"
			"ld __tmp_reg__, %a0\n"
			"rcall send_led_strip_byte%=\n"
			"ld __tmp_reg__, -%a0\n"
			"rcall send_led_strip_byte%=\n"
			"ld __tmp_reg__, %a0+\n"
			"ld __tmp_reg__, %a0+\n"
			"ld __tmp_reg__, %a0+\n"
			"rcall send_led_strip_byte%=\n"
			"rjmp led_strip_asm_end%=\n"
			"send_led_strip_byte%=:\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"ret\n"
			"send_led_strip_bit%=:\n"
			"sbi %2, %3\n"
			#if F_CPU != 8000000
			"rol __tmp_reg__\n"
			#endif
			#if F_CPU == 16000000
			"nop\n" "nop\n"
			#elif F_CPU != 8000000
			#error "Unsupported F_CPU"
			#endif
			"brcs .+2\n" "cbi %2, %3\n"
			#if F_CPU == 16000000
			"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
			#endif
			"brcc .+2\n" "cbi %2, %3\n"
			"ret\n"
			"led_strip_asm_end%=: "
			: "=b" (colors)
			: "0" (colors),
			"I" (_SFR_IO_ADDR(LED_S_STRIP_PORT)),
			"I" (LED_S_STRIP_PIN)
			);
		}
		sei();
		_delay_us(80);
}

// void __attribute__((noinline)) led_fotoresistenza_strip_write(rgb_color * colors, unsigned int count)
// {
// 	LED_FOTO_STRIP_PORT &= ~(1<<LED_FOTO_STRIP_PIN);
// 	LED_FOTO_STRIP_DDR |= (1<<LED_FOTO_STRIP_PIN);
// 	cli();
// 	while(count--)
// 	{
// 		asm volatile(
// 		"ld __tmp_reg__, %a0+\n"
// 		"ld __tmp_reg__, %a0\n"
// 		"rcall send_led_strip_byte%=\n"
// 		"ld __tmp_reg__, -%a0\n"
// 		"rcall send_led_strip_byte%=\n"
// 		"ld __tmp_reg__, %a0+\n"
// 		"ld __tmp_reg__, %a0+\n"
// 		"ld __tmp_reg__, %a0+\n"
// 		"rcall send_led_strip_byte%=\n"
// 		"rjmp led_strip_asm_end%=\n"
// 		"send_led_strip_byte%=:\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"rcall send_led_strip_bit%=\n"
// 		"ret\n"
// 		"send_led_strip_bit%=:\n"
// 		"sbi %2, %3\n"
// 		#if F_CPU != 8000000
// 		"rol __tmp_reg__\n"
// 		#endif
// 		#if F_CPU == 16000000
// 		"nop\n" "nop\n"
// 		#elif F_CPU != 8000000
// 		#error "Unsupported F_CPU"
// 		#endif
// 		"brcs .+2\n" "cbi %2, %3\n"
// 		#if F_CPU == 16000000
// 		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
// 		#endif
// 		"brcc .+2\n" "cbi %2, %3\n"
// 		"ret\n"
// 		"led_strip_asm_end%=: "
// 		: "=b" (colors)
// 		: "0" (colors),
// 		"I" (_SFR_IO_ADDR(LED_FOTO_STRIP_PORT)),
// 		"I" (LED_FOTO_STRIP_PIN)
// 		);
// 	}
// 	sei();
// 	_delay_us(80);
// 	
// }
// 

void aggiorna_segnale(){
	led_strip_write(colore_seganle,LED_COUNT_SEGNALE);
}

rgb_color scegli_colore(enum colori COLORE){
	rgb_color colors[1];
	switch (COLORE){
		case RED:
		colors[0].red=255;
		colors[0].green=0;
		colors[0].blue=0;
		break;
		case BLUE:
		colors[0].red=0;
		colors[0].green=0;
		colors[0].blue=255;
		break;
		case GREEN:
		colors[0].red=0;
		colors[0].green=255;
		colors[0].blue=0;
		break;
		case SPENTO:
		colors[0].red=0;
		colors[0].green=0;
		colors[0].blue=0;
		break;
		case YELLOW:
		colors[0].red=255;
		colors[0].green=255;
		colors[0].blue=0;
		break;
		case ORANGE:
		colors[0].red=255;
		colors[0].green=127;
		colors[0].blue=0;
		break;
		case WHITE:
		colors[0].red=255;
		colors[0].green=255;
		colors[0].blue=255;
		break;
		
		case PINKColor:
		colors[0].red=255;
		colors[0].green=0;
		colors[0].blue=255;
		break;
		
		case PURPLE:
		colors[0].red=100;
		colors[0].green=0;
		colors[0].blue=100;
		break;
		
		case GRAY:
		colors[0].red=150;
		colors[0].green=150;
		colors[0].blue=150;
		break;
		
		default:
		colors[0].red=0;
		colors[0].green=0;
		colors[0].blue=0;
		break;
	}
	return colors[0];
}


// void ColoreFotoresistenza(uint8_t R, uint8_t G, uint8_t B ){
// 	for(uint8_t i=0;i<LED_COUNT_FOTORESISTENZA;i++){
// 		colore_fotoresistenza[i].red=R;
// 		colore_fotoresistenza[i].green=G;
// 		colore_fotoresistenza[i].blue=B;
// 	}
// 	led_fotoresistenza_strip_write(colore_fotoresistenza, LED_COUNT_FOTORESISTENZA);
// }

void ColoreLED1(enum colori col){
	colore_seganle[0]=scegli_colore(col);
	aggiorna_segnale();
}
void ColoreLED2(enum colori col){
	colore_seganle[1]=scegli_colore(col);
	aggiorna_segnale();
}
void ColoreLED3(enum colori col){
	colore_seganle[2]=scegli_colore(col);
	aggiorna_segnale();
}
