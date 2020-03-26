
#define F_CPU 16000000ul
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "NeoPixel.h"
#define LED_STRIP_PORT PORTG
#define LED_STRIP_DDR  DDRG
#define LED_STRIP_PIN  0
#define LED1_STRIP_PORT PORTG
#define LED1_STRIP_DDR  DDRG
#define LED1_STRIP_PIN  2


void SpegniLED(){
	ColoreLED(SPENTO);
}

void SpegniLED1(){
	ColoreLED1(SPENTO);
}
 
void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count)
{
		LED_STRIP_PORT &= ~(1<<LED_STRIP_PIN);
		LED_STRIP_DDR |= (1<<LED_STRIP_PIN);
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
			"I" (_SFR_IO_ADDR(LED_STRIP_PORT)),
			"I" (LED_STRIP_PIN)
			);
		}
		sei();
		_delay_us(80);
}

void __attribute__((noinline)) led1_strip_write(rgb_color * colors, unsigned int count)
{
	LED1_STRIP_PORT &= ~(1<<LED1_STRIP_PIN);
	LED1_STRIP_DDR |= (1<<LED1_STRIP_PIN);
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
		"I" (_SFR_IO_ADDR(LED1_STRIP_PORT)),
		"I" (LED1_STRIP_PIN)
		);
	}
	sei();
	_delay_us(80);
	
}

void led_color(rgb_color * colors){
	if(LedGara==1){
		led_strip_write(colors, LED_COUNT);
	}
	else{
		colors[0].red=0;
		colors[0].green=0;
		colors[0].blue=0;
		led_strip_write(colors, LED_COUNT);
	}
}

void led1_color(rgb_color * colors){
	
		led1_strip_write(colors, LED_COUNT);
	
}

void ColoreLED(enum colori COLORE){
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
		led_color(colors);
}
void ColoreLED1(enum colori COLORE){
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
		colors[0].red=145;
		colors[0].green=0;
		colors[0].blue=145;
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
	led1_color(colors);
}
