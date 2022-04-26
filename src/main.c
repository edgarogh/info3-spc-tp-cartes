#include <math.h>
#include "sys/devices.h"
#include "sys/init.h"
#include "sys/clock.h"

#include "screen.h"

#define DEBOUNCE(name, amount) static uint64_t name; if (millis - name < amount) { return; } else { name = millis; }

void init_PB(){
	GPIOC.MODER = (GPIOC.MODER & 0xF3FFFFFF) ;
}

void button_irqPC13_init() {
    init_PB();
    SYSCFG.EXTICR4 = (SYSCFG.EXTICR4 & ~(0xf<<4)) | (0x2<<4);

    // Setup interrupt for EXTI13, falling edge
    EXTI.IMR |= (1<<13);
    EXTI.RTSR &= ~(1<<13);
    EXTI.FTSR |= (1<<13);
    EXTI.PR |= (1<<13);

    // enable EXTI15-10 IRQ PC13
    NVIC.ISER[40/32]=(1<<(40%32));
}

const uint32_t MS100 = 5600000 / 50;
void tempo_10ms(){
    volatile uint32_t duree;
    for (duree = 0; duree < MS100; duree++) {
        ;
    }
}

void systick_init(uint32_t freq){
	uint32_t p = get_SYSCLK()/freq;
	SysTick.LOAD = (p-1) & 0x00FFFFFF;
	SysTick.VAL = 0;
	SysTick.CTRL |= 7;
}

enum {
    StIdle,
    StMeasuring,
    StPaused,
} current_state;

uint64_t millis = 0;
uint64_t chrono_millis = 0;

void __attribute__((interrupt)) EXTI15_10_Handler() {
    DEBOUNCE(EXTI15_10_Handler_last, 200);

    switch (current_state) {
        default:
        case StIdle:
            current_state = StMeasuring;
            break;
        case StMeasuring:
            current_state = StPaused;
            break;
        case StPaused:
            current_state = StMeasuring;
            break;
    }

    EXTI.PR |= (1<<13);
}

void __attribute__((interrupt)) SysTick_Handler(){
    millis++;
    if (current_state == StMeasuring) chrono_millis++;
}

_Noreturn int main() {
    systick_init(1000);
    screen_init();
    button_irqPC13_init();

    current_state = StIdle;

    while (1) {
        screen_writeCounter(chrono_millis / 100);
        screen_setOn((current_state != StPaused) || (millis % 500 < 250));
        tempo_10ms();
    }
}

