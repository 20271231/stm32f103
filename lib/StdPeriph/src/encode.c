#include "stm32f10x.h"
#include "Delay.h"

static volatile int16_t Encode_Count = 0;
static volatile uint8_t Button_Pressed = 0;
/* last encoder state: (a<<1)|b where a=CLK(PB1), b=DT(PB0) */
static volatile uint8_t last_state = 0;

/* transition table: index = (prev<<2)|curr -> delta (+1 CW, -1 CCW, 0 invalid/no move) */
static const int8_t quad_table[16] = {
    0,  1, -1,  0,
   -1,  0,  0,  1,
    1,  0,  0, -1,
    0, -1,  1,  0
};

/*
  Connections (per user):
  - CLK -> PB1 (旋转脉冲)
  - DT  -> PB0 (方向/相位)
  - SW  -> PB2 (按键)
*/

void Encode_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    /* PB0, PB1 as input pull-up (CLK/DT) */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* initialize last_state from current inputs */
    {
        uint8_t a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
        uint8_t b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
        last_state = (a << 1) | b;
    }

    /* EXTI for PB0 (EXTI0), PB1 (EXTI1), PB2 (EXTI2) */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource2);

    EXTI_InitTypeDef EXTI_InitStructure;
    /* Configure EXTI1 for CLK rising edge only */
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStructure);

    /* EXTI2 (button) on falling edge (active low) */
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStruct);
}

int16_t Encode_Get(void)
{
    int16_t temp;
    temp = Encode_Count;
    Encode_Count = 0;
    return temp;
}

uint8_t Encode_Button_Get(void)
{
    if (Button_Pressed)
    {
        Button_Pressed = 0;
        return 1;
    }
    return 0;
}

/* Polling fallback: call periodically from main loop if interrupts are not firing */
void Encode_Poll(void)
{
    uint8_t a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
    uint8_t b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
    uint8_t curr = (a << 1) | b;
    if (curr != last_state)
    {
        int8_t d = quad_table[(last_state << 2) | curr];
        Encode_Count += d;
        last_state = curr;
    }
}

uint8_t Encode_GetState(void)
{
    return last_state;
}

/* Simple debounced encoder via edge IRQs.
   We read both CLK (PB1) and DT (PB0) on edges to determine direction.
*/
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        uint8_t a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1); /* CLK */
        uint8_t b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0); /* DT */
        uint8_t curr = (a << 1) | b;
        int8_t d = quad_table[(last_state << 2) | curr];
        Encode_Count += d;
        last_state = curr;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        /* button on PB2, active low */
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2) == 0)
        {
            Button_Pressed = 1;
        }
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}
