#include "stm32f10x.h"
#include "Delay.h"
#include "led.h"
#include "key.h"

uint8_t KeyNum;

int main(void)
{

    LED_INIT();
    Key_Init();
    /**
     * A功能
     * 1.A1和A2分别接入led1、led2
     * 2.led1、led2正极接入stm32开发板3v引脚
     * ３.按键１、按键２一侧分别接入stm32开发板B11、B1;一侧分别接入GND
     * 可以实现一个按键实现亮灯,可以实现一个按键实现灭灯效果
     * 
     * B功能
     * 一个按键单独控制led的亮和熄灭
     */

    while (1)
    {
        KeyNum = Key_GetNum();
        if (KeyNum == 1)
            //LED1_ON();//A
            LED1_Turn();//B功能
        if (KeyNum == 2)
            //LED1_OFF();//A
            LED2_Turn();//B功能
    }

    return 0;
}