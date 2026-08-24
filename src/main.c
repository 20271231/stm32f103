#include "stm32f10x.h"
#include "Delay.h"
#include "buzzer.h"
#include "lightSenor.h"

uint8_t KeyNum;

int main(void)
{
    BUZZER_INIT();
    LightSenor_Init();

    while (1)
    {
        if (LightSenorGetNum() == 1)
        {
            BUZZER_ON();
        }
        else
        {
            BUZZER_OFF();
        }
    }

    return 0;
}