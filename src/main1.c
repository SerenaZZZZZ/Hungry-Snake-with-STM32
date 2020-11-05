/*//Step1
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include "tty.h"
#include "screen.h"

int main(void)
{
    int x,y;
    tty_init();
    raw_mode();
    cursor_off();
    // Fill the terminal with a colorful pattern.
    for(y=0; y<24; y++) {
        setpos(0,y);
        for(x=0; x<80; x++) {
            fgbg(((x%16)<<4) + (y%16));
            putchar('A' + (x%26));
        }
    }
    x=0; y=0;
    for(;;) {
        char c = ' ';
        if (available())
            c = getchar();
        switch(c) {
            case 'w': if (y>0) y--; break;
            case 'a': if (x>0) x--; break;
            case 's': if (y<23) y++; break;
            case 'd': if (x<79) x++; break;
            default: break;
        }
        setpos(x,y);
        fgbg(0x0f);
        putchar(' ');
    }
    return 0;
}
*/
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "tty.h"
#include "snake.h"

void TIM2_IRQHandler(void)
{
    TIM2->SR &= ~TIM_SR_UIF;
    animate();
}

//===================================
// Disable timer 2.  Wait for a key press.
void freeze(void)
{
    TIM2->CR1 &= ~TIM_CR1_CEN;
    while(!available())
        ;
    getchar();
    TIM2->CR1 |= TIM_CR1_CEN;
}

int get_seed(void)
{
    return TIM2->CNT;
}

//============================================================================
// Enable clock to timer2.
// Setup prescalar and arr so that the interrupt is triggered every 125 ms.
// Enable the timer 2 interrupt.
// Start the timer.
void setup_timer2() {
    // Enable timer 2 here.
    // Make it generate an interrupt 8 times per second.
    // Set the prescaler to 48 so that the free-running
    // counter goes from 0 ... 124999.
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2 ->PSC = 48000 - 1;
    TIM2 ->ARR = 125 - 1;
    TIM2 ->DIER |= TIM_DIER_UIE;
    NVIC ->ISER[0] = 1 << TIM2_IRQn;
    TIM2 ->CR1 |= TIM_CR1_CEN;//enable timer 2
    NVIC_SetPriority(TIM2_IRQn, 2);
    // Also, make sure TIM2 has lower priority than USART1
    // by using the NVIC_SetPriority() function.  Use a higher
    // number (lower priority) than you did for USART1.
}

int main(void)
{
    tty_init();
    raw_mode();
    cursor_off();
    init();
    splash();
    setup_timer2();
    for(;;)
        asm("wfi");
    return 0;
}
