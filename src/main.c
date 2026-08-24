#include "stm32f10x.h"
#include "Delay.h"
#include "oled.h"

int main(void)
{

    OLED_Init();
    OLED_ShowString(0, 0, "helloWorld");
    while (1)
    {
        // 可在此处更新显示内容或循环演示
    }

    return 0;
}