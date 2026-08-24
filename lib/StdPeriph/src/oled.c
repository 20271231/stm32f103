#include "oled.h"
#include "oled_Font.h"

// 基于软件 I2C (GPIO bit-bang) 的简单驱动，使用 PB8=SCL, PB9=SDA

static void OLED_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(OLED_PORT, &GPIO_InitStruct);

    GPIO_SetBits(OLED_PORT, OLED_SCL_PIN | OLED_SDA_PIN);
}

static void I2C_Delay(void)
{
    Delay_us(5);
}

static void I2C_Start(void)
{
    GPIO_SetBits(OLED_PORT, OLED_SDA_PIN);
    GPIO_SetBits(OLED_PORT, OLED_SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(OLED_PORT, OLED_SDA_PIN);
    I2C_Delay();
    GPIO_ResetBits(OLED_PORT, OLED_SCL_PIN);
}

static void I2C_Stop(void)
{
    GPIO_ResetBits(OLED_PORT, OLED_SDA_PIN);
    GPIO_SetBits(OLED_PORT, OLED_SCL_PIN);
    I2C_Delay();
    GPIO_SetBits(OLED_PORT, OLED_SDA_PIN);
    I2C_Delay();
}

static void I2C_WriteByte(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        if (data & 0x80) GPIO_SetBits(OLED_PORT, OLED_SDA_PIN);
        else GPIO_ResetBits(OLED_PORT, OLED_SDA_PIN);
        I2C_Delay();
        GPIO_SetBits(OLED_PORT, OLED_SCL_PIN);
        I2C_Delay();
        GPIO_ResetBits(OLED_PORT, OLED_SCL_PIN);
        data <<= 1;
    }
    // 忽略 ACK
    GPIO_SetBits(OLED_PORT, OLED_SDA_PIN);
    I2C_Delay();
    GPIO_SetBits(OLED_PORT, OLED_SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(OLED_PORT, OLED_SCL_PIN);
}

static void OLED_WriteCmd(uint8_t cmd)
{
    I2C_Start();
    I2C_WriteByte((OLED_I2C_ADDR << 1) | 0); // address + write
    I2C_WriteByte(0x00); // control byte: Co = 0, D/C# = 0 -> command
    I2C_WriteByte(cmd);
    I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    I2C_Start();
    I2C_WriteByte((OLED_I2C_ADDR << 1) | 0);
    I2C_WriteByte(0x40); // control byte: Co = 0, D/C# = 1 -> data
    I2C_WriteByte(data);
    I2C_Stop();
}

void OLED_Init(void)
{
    OLED_GPIO_Init();

    // 无独立 RESET 引脚时可通过电源复位
    Delay_ms(100);

    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0x20);
    OLED_WriteCmd(0x10);
    OLED_WriteCmd(0xB0);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x81);
    OLED_WriteCmd(0x7F);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0xA8);
    OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xD3);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0xD5);
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xD9);
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDA);
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0xDB);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D);
    OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xAF);

    OLED_Clear();
}

void OLED_SetPos(uint8_t x, uint8_t y)
{
    OLED_WriteCmd(0xB0 + y);
    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);
    OLED_WriteCmd((x & 0x0F) | 0x00);
}

void OLED_Clear(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        OLED_SetPos(0, page);
        for (uint8_t col = 0; col < 128; col++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char chr)
{
    const unsigned char *p = oled_get_font(chr);
    OLED_SetPos(x, y);
    for (int i = 0; i < 6; i++)
    {
        OLED_WriteData(p[i]);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    uint8_t curx = x;
    while (*str)
    {
        OLED_ShowChar(curx, y, *str++);
        curx += 6;
        if (curx + 6 > 128) { curx = 0; y++; }
        if (y >= 8) break;
    }
}

void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    char buf[12];
    buf[len] = '\0';
    for (int i = len - 1; i >= 0; i--)
    {
        buf[i] = '0' + (num % 10);
        num /= 10;
    }
    OLED_ShowString(x, y, buf);
}
