#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sys/cm4.h"
#include "sys/devices.h"
#include "sys/init.h"
#include "sys/clock.h"

typedef uint8_t bool;

static volatile char c=0;

void init_LD2(){
	RCC.AHB1ENR |= 0x01;
	GPIOA.MODER = (GPIOA.MODER & 0xFFFFF3FF) | 0x00000400;
	GPIOA.OTYPER &= (0x1<<5);
	GPIOA.OSPEEDR |= 0x03<<10;
	GPIOA.PUPDR &= 0xFFFFF3FF;
}

void init_PB(){
	GPIOC.MODER = (GPIOC.MODER & 0xF3FFFFFF) ;
}

void tempo_500ms(){
	volatile uint32_t duree;
	for (duree = 0; duree < 5600000 ; duree++){
		;
	}

}

void init_USART(){
	GPIOA.MODER = (GPIOA.MODER & 0xFFFFFF0F) | 0x000000A0;
	GPIOA.AFRL = (GPIOA.AFRL & 0xFFFF00FF) | 0x00007700;
	USART2.BRR = get_APB1CLK()/9600;
	USART2.CR3 = 0;
	USART2.CR2 = 0;
}

void _putc(char c){
	while( (USART2.SR & 0x80) == 0);  
	USART2.DR = c;
}

void _puts(char *c){
	int len = strlen(c);
	for (int i=0;i<len;i++){
		_putc(c[i]);
	}
}

char _getc(){
	while ( (USART2.SR & 0x20) == 0); 
	return USART2.DR;

}

void systick_init(uint32_t freq){
	uint32_t p = get_SYSCLK()/freq;
	SysTick.LOAD = (p-1) & 0x00FFFFFF;
	SysTick.VAL = 0;
	SysTick.CTRL |= 7;
}

void __attribute__((interrupt)) SysTick_Handler(){
}

void setLed(bool on) {
    if (on) {
        GPIOA.ODR |= (1 << 5);
    } else {
        GPIOA.ODR &= ~(1 << 5);
    }
}

bool isButtonPressed() {
    return (GPIOC.IDR & (1 << 13)) ? 0 : 1;
}

char bitsToHex(uint8_t bits4) {
    if (bits4 < 10) return '0' + bits4;
    if (bits4 < 16) return 'A' + bits4 - 10;
    return '-';
}

int main() {
    init_USART();

    printf("Quel est votre caractère ASCII porte bonheur ? ");
    char porteBonheur = _getc();
    printf("\nAh ouais c'est un caractère cool '%c' en sah\n", porteBonheur);
    _puts("J'aime l'harmonie que forment ses composantes hexadecimales: ");

    char hex[] = {
            bitsToHex(porteBonheur >> 4),
            bitsToHex(porteBonheur & 0b1111),
            0,
    };

    _puts(hex);
    _putc('\n');

	return 0;
}

