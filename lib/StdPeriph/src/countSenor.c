#include "stm32f10x.h"

uint16_t countSenor_count;

void COuntSenor_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitTypeDef;

    GPIO_InitTypeDef.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitTypeDef.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitTypeDef);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);

    EXTI_InitTypeDef EXTI_InitTypeDef;
    EXTI_InitTypeDef.EXTI_Line = EXTI_Line14;
    EXTI_InitTypeDef.EXTI_LineCmd = ENABLE;
    EXTI_InitTypeDef.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitTypeDef.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitTypeDef);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
}

void EXTI15_10_IRQHandler()
{
    if (EXTI_GetITStatus(EXTI_Line14) == SET)
    {
        countSenor_count++;
        EXTI_ClearITPendingBit(EXTI_Line14);
    }
}

uint16_t GetCountSenorNum(void)
{
    return countSenor_count;
}