/**
 * STM32F103C8T6 板载 LED 闪烁 —— 函数封装版
 *
 * 结构分四层:
 *   1. 寄存器宏定义   —— 用绝对地址访问硬件
 *   2. LED 驱动函数   —— LED_Init / LED_On / LED_Off
 *   3. 两种延时实现   —— 同时保留,二选一使用:
 *        A. Delay_sw()  软件循环延时:不依赖任何外设,但时长受优化等级影响
 *        B. Delay_ms()  SysTick 轮询延时:精确毫秒级,不受优化等级影响
 *   4. main           —— 通过 USE_SYSTICK_DELAY 宏切换两种延时
 *
 * 硬件:板载 LED 接 PC13,低电平点亮
 */

#include <stdint.h>

/* ════════════════ 0. 编译期开关 ════════════════ */
#define USE_SYSTICK_DELAY 0 /* 1 = 用 SysTick 精确延时; 0 = 用软件循环延时 */
#define LED_BLINK_MS 1000   /* LED 亮/灭各持续的毫秒数(SysTick 模式下使用) */

/* ════════════════ 1. 寄存器宏定义 ════════════════ */

/* ---- RCC(复位和时钟控制) ---- */
#define RCC_BASE 0x40021000UL
#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x18))

#define RCC_APB2ENR_IOPCEN ((uint32_t)0x00000010) /* bit4: GPIOC 时钟使能 */

/* ---- GPIOC ---- */
#define GPIOC_BASE 0x40011000UL // 0x40000000 + 0x10000 + 0x1000
#define GPIOC_CRH (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))
#define GPIOC_BSRR (*(volatile uint32_t *)(GPIOC_BASE + 0x10))
#define GPIOC_BRR (*(volatile uint32_t *)(GPIOC_BASE + 0x14))

/* GPIOC CRH 中控制 PC13 的 4 位字段: CNF[23:22] + MODE[21:20] */
#define GPIO_CRH_PC13_MASK (0xFUL << 20)
#define GPIO_CRH_PC13_PP_50M (0x3UL << 20) /* CNF=00 通用推挽, MODE=11 输出 50MHz */

/* ---- SysTick(Cortex-M3 内核定时器,属于内核外设) ---- */
#define SYST_BASE 0xE000E010UL
#define SYST_CTRL (*(volatile uint32_t *)(SYST_BASE + 0x00))
#define SYST_LOAD (*(volatile uint32_t *)(SYST_BASE + 0x04))
#define SYST_VAL (*(volatile uint32_t *)(SYST_BASE + 0x08))

#define SYST_CTRL_ENABLE (1UL << 0)     /* bit0:  使能计数器 */
#define SYST_CTRL_CLKSOURCE (1UL << 2)  /* bit2:  1=HCLK(72MHz), 0=HCLK/8 */
#define SYST_CTRL_COUNTFLAG (1UL << 16) /* bit16: 计数到 0 时硬件置 1,读后自动清零 */

/* ════════════════ 2. LED 驱动函数 ════════════════ */

/** 初始化 PC13 为推挽输出,初始状态 = 灭(输出高) */
static void LED_Init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPCEN; /* ① 开 GPIOC 时钟 */

    GPIOC_CRH = (GPIOC_CRH & ~GPIO_CRH_PC13_MASK) | GPIO_CRH_PC13_PP_50M; /*    写入: 推挽 + 50MHz */

    GPIOC_BSRR = (1UL << 13); /* ③ 初始输出高 = 灭 */
}

/** 点亮 LED:PC13 输出低电平(低电平点亮) */
static void LED_On(void)
{
    GPIOC_BRR = (1UL << 13); /* BRR 写 1 → 对应 ODR 位清 0(原子操作) */
}

/** 熄灭 LED:PC13 输出高电平 */
static void LED_Off(void)
{
    GPIOC_BSRR = (1UL << 13); /* BSRR 低 16 位写 1 → 对应 ODR 位置 1(原子操作) */
}

/* ════════════════ 3A. 延时实现 A:软件循环(粗略) ════════════════ */

/**
 * 粗略软件延时。时长 = loops × 每循环周期数 / 主频。
 * -O2 下 volatile while 约每循环 5 个周期:
 *   0x300000 × 5 / 72MHz ≈ 0.21s
 * 注意:换优化等级或换主频,时长就变。
 */
__attribute__((always_inline)) static void Delay_sw(volatile uint32_t loops)
{
    while (loops--)
        ; /* 空循环 */
}

/* ════════════════ 3B. 延时实现 B:SysTick 轮询(精确 ms) ════════════════ */

/**
 * 精确毫秒延时,轮询方式,不占用中断。
 *
 * 原理:
 *   SysTick 是 24 位向下计数器,从 LOAD 值数到 0 置 COUNTFLAG。
 *   时钟源选 HCLK(72MHz),LOAD = 72000-1 → 每 72000 个时钟 = 精确 1ms。
 *   COUNTFLAG 读 SYST_CTRL 时硬件自动清零,所以每读一次 = 过了 1ms。
 *
 * 注意:若系统时钟未跑到 72MHz(如 HSE 失败落到 HSI),
 *       延时会按比例变长,但 LED 仍会闪烁。
 */
__attribute__((always_inline)) static void Delay_ms(uint32_t ms)
{
    SYST_LOAD = 72000UL - 1;                            /* 重装载值: 1ms @72MHz */
    SYST_VAL = 0;                                       /* 清当前计数值,从满值开始数 */
    SYST_CTRL = SYST_CTRL_ENABLE | SYST_CTRL_CLKSOURCE; /* 开计数器,HCLK 源,不开中断 */

    while (ms--)
    {
        while ((SYST_CTRL & SYST_CTRL_COUNTFLAG) == 0)
            ; /* 死等 1ms 到(读 CTRL 时 flag 自动清零) */
    }

    SYST_CTRL = 0; /* 用完关闭,省电 */
}

void ControlTheBlinkingOfTheOn_boardLED(void)
{
    LED_Init();
    if (USE_SYSTICK_DELAY)
    {
        /* —— 方式 B:SysTick 精确延时 —— */
        while (1)
        {
            LED_On();
            Delay_ms(LED_BLINK_MS);
            LED_Off();
            Delay_ms(LED_BLINK_MS);
        }
    }
    else
    {
        /* —— 方式 A:软件循环延时 —— */
        while (1)
        {
            LED_On();
            Delay_sw(0x300000); /* ≈0.21s @72MHz,-O2 */
            LED_Off();
            Delay_sw(0x300000);
        }
    }
    /* —— 方式 B:SysTick 精确延时 —— */
    while (1)
    {
        LED_On();
        Delay_ms(LED_BLINK_MS);
        LED_Off();
        Delay_ms(LED_BLINK_MS);
    }
}
int main(void)
{
    ControlTheBlinkingOfTheOn_boardLED();
}
