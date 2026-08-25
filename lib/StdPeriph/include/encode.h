#ifndef _ENCODE_H
#define _ENCODE_H

#include "stm32f10x.h"

int16_t Encode_Get(void);
uint8_t Encode_Button_Get(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void Encode_Init(void);
void Encode_Poll(void);
uint8_t Encode_GetState(void);

#endif