#include "stm32f10x.h"
#include "Delay.h"
#include "led.h"

int main(void)
{

    LED_INIT();

    while (1)
    {
        GPIO_Write(GPIOA, ~0x0001);
        Delay_ms(100);
        GPIO_Write(GPIOA, ~0x0002);
        Delay_ms(100);
        GPIO_Write(GPIOA, ~0x0004);
        Delay_ms(100);
        GPIO_Write(GPIOA, ~0x0008);
        Delay_ms(100);
        GPIO_Write(GPIOA, ~0x0010);
        Delay_ms(100);
        GPIO_Write(GPIOA, ~0x0020);
        Delay_ms(100);
        GPIO_Write(GPIOA, ~0x0040);
        Delay_ms(100);
        GPIO_Write(GPIOA, ~0x0080);
        Delay_ms(100);
    }

    return 0;
}