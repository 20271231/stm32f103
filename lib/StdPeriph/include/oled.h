#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"
#include "Delay.h"

// 可在工程中覆盖以下宏来指定引脚与端口
// 本驱动使用 4 线 I2C: GND,VCC,SCL,SDA
// 默认 SCL -> PB8, SDA -> PB9（可根据开发板调整）
#ifndef OLED_PORT
#define OLED_PORT GPIOB
#endif
#ifndef OLED_SCL_PIN
#define OLED_SCL_PIN GPIO_Pin_8
#endif
#ifndef OLED_SDA_PIN
#define OLED_SDA_PIN GPIO_Pin_9
#endif

// I2C 设备 7-bit 地址（常见 SSD1306 为 0x3C）
#ifndef OLED_I2C_ADDR
#define OLED_I2C_ADDR 0x3C
#endif

// 基本函数
void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetPos(uint8_t x, uint8_t y);
void OLED_ShowChar(uint8_t x, uint8_t y, char chr);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);

#endif
