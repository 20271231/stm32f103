#include "stm32f10x.h"
#include "Delay.h"
#include "oled.h"
#include "countSenor.h"

int main(void)
{

    OLED_Init();
    COuntSenor_Init();
    OLED_ShowString(1,1,"Count:");
    while (1)
    {
        OLED_ShowNum(1,7,GetCountSenorNum(),5);
        // 可在此处更新显示内容或循环演示
        
    }

    return 0;
}