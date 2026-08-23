#include "stm32f10x.h"

static void SysTick_DelayTicks(uint32_t ticks)
{
    if (ticks == 0U)
    {
        return;
    }

    SysTick->LOAD = (ticks & SysTick_LOAD_RELOAD_Msk) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_CLKSOURCE;

    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG) == 0U)
    {
    }

    SysTick->CTRL = 0U;
}

void Delay_us(uint32_t us)
{
    uint32_t ticks;

    if (SystemCoreClock == 0U)
    {
        SystemCoreClockUpdate();
    }

    ticks = (SystemCoreClock / 1000000UL) * us;
    if ((ticks == 0U) && (us != 0U))
    {
        ticks = 1U;
    }

    SysTick_DelayTicks(ticks);
}

void Delay_ms(uint32_t ms)
{
    while (ms--)
    {
        Delay_us(1000U);
    }
}

void Delay_s(uint32_t s)
{
    while (s--)
    {
        Delay_ms(1000U);
    }
}
