#include "stm32f10x.h"
#include "Delay.h"
#include "oled.h"
#include "encode.h"

int16_t Num;

int main(void)
{
    OLED_Init();
    Encode_Init();
    OLED_ShowString(1, 1, "Num:");

    while (1)
    {
        /* handle rotation */
        Encode_Poll(); /* polling fallback */
        int16_t delta = Encode_Get();
        if (delta != 0)
        {
            Num += delta;
            OLED_ShowNum(1, 5, Num, 5);
        }

        /* debug: show raw pins and state */
        uint8_t a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
        uint8_t b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
        OLED_ShowString(1, 7, "A:");
        OLED_ShowNum(3, 7, a, 1);
        OLED_ShowString(5, 7, "B:");
        OLED_ShowNum(7, 7, b, 1);
        OLED_ShowString(9, 7, "S:");
        OLED_ShowNum(11, 7, Encode_GetState(), 1);

        /* handle button press */
        if (Encode_Button_Get())
        {
            /* on button press, reset number */
            Num = 0;
            OLED_ShowNum(1, 5, Num, 5);
            OLED_ShowString(1, 7, "Reset");
            Delay_ms(500);
            OLED_ShowString(1, 7, "     ");
        }
    }

    return 0;
}