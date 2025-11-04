/*
 * Copyright (c) 2022 
 * Computer Science and Engineering, University of Dhaka
 * Credit: CSE Batch 25 (starter) and Prof. Mosaddek Tushar
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
 
#include <cm4.h>
#include <sys_clock.h>
#include <syscall.h>

// A global variable to hold the tick count.
// 'volatile' is crucial here. It tells the compiler that this variable can change
// at any time (e.g., inside an interrupt), preventing optimizations that might
// cause incorrect behavior.
static volatile uint32_t g_sys_tick_count = 0;

/************************************************************************************
* __SysTick_init(uint32_t reload) 
* Function initialize the SysTick clock. The function with a weak attribute enables 
* redefining the function to change its characteristics whenever necessary.
**************************************************************************************/
void __SysTick_init(uint32_t reload)
{
    //Disable SysTick during setup
    SYSTICK->CTRL = 0;

    //Set the reload value. The timer will count down from this value.
    // The value is (reload - 1) because the countdown includes 0.
    SYSTICK->LOAD = reload - 1;

    // 3. Set the interrupt priority to the lowest level
    NVIC_SetPriority(SysTick_IRQn, 15); // 15 is the lowest interrupt value
    
    // 4. Reset the current SYSTICK counter value
    SYSTICK->VAL = 0;

    // 5. Select Processor Clock (AHB), enable SYSTICK interrupt, and enable the timer
    SYSTICK->CTRL = SysTick_CTRL_CLKSOURCE_Msk | // Use processor clock
                    SysTick_CTRL_TICKINT_Msk |   // Enable interrupt
                    SysTick_CTRL_ENABLE_Msk;     // Enable SysTick
}

void SysTickIntDisable(void)
{
    // Clear the TICKINT bit to disable the interrupt
    SYSTICK->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void SysTickIntEnable(void)
{
    // Set the TICKINT bit to enable the interrupt
    SYSTICK->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

/************************************************************************************
* __sysTick_enable(void) 
* The function enables the SysTick clock if already not enabled. 
* redefining the function to change its characteristics whenever necessary.
**************************************************************************************/
void __SysTick_enable(void)
{
    // Set the ENABLE bit to start the timer
    SysTickIntEnable();
}

void __sysTick_disable(void)
{
    // Clear the ENABLE bit to stop the timer
    SysTickIntDisable();
}

uint32_t __getSysTickCount(void)
{
    // Atomically return the current tick count
    return g_sys_tick_count;
}

/************************************************************************************
* __updateSysTick(uint32_t count) 
* Function reinitialize the SysTick clock. The function with a weak attribute enables 
* redefining the function to change its characteristics whenever necessary.
**************************************************************************************/
void __updateSysTick(uint32_t reload)
{
    // Set a new reload value
    SYSTICK->LOAD = reload - 1;
    // Reset the current counter to start fresh
    SYSTICK->VAL = 0;
}

/************************************************************************************
* __getTime(void) 
* Function return the SysTick elapsed time from the begining or reinitialing.
**************************************************************************************/
uint32_t __getTime(void)
{
    return g_sys_tick_count;
}

// NOTE: The following functions were nested inside __getTime in your template.
// I have moved them outside to be valid C functions.

uint32_t __get__Second(void){
    // Assuming 1ms tick, divide by 1000 to get seconds
    return g_sys_tick_count / 1000;
}

uint32_t __get__Minute(void){
    // Convert total seconds to minutes
    return __get__Second() / 60;
}

uint32_t __get__Hour(void){
    // Convert total minutes to hours
    return __get__Minute() / 60;
}

/************************************************************************************
* SysTick_Handler(void)
* This is the Interrupt Service Routine (ISR) for the SysTick timer.
* It is called automatically every time the SysTick counter reaches 0.
* The name "SysTick_Handler" is fixed by ARM CMSIS and is defined in your
* startup file (e.g., startup_stm32f446retx.s).
**************************************************************************************/
void SysTick_Handler(void)
{
    // This is the core of the tick timer. Every time the interrupt fires,
    // we increment our global tick counter.
    g_sys_tick_count++;
}

void __enable_fpu()
{
    // Enable the Floating Point Unit (FPU).
    // Required for any floating point operations.
    SCB->CPACR |= ((0xFUL<<20));

}

/**
 * __disable_irq() - Disable all interrupts globally
 *
 * Uses the ARM CPSID instruction to set PRIMASK
 */
void __disable_irq(void)
{
    __asm volatile ("cpsid i" : : : "memory");
}

/**
 * __enable_irq() - Enable all interrupts globally
 *
 * Uses the ARM CPSIE instruction to clear PRIMASK
 */
void __enable_irq(void)
{
    __asm volatile ("cpsie i" : : : "memory");
}

uint8_t ms_delay(uint32_t delay)
{
    uint32_t start_tick = g_sys_tick_count;
    // Wait until the elapsed time is greater than or equal to the delay
    while ((g_sys_tick_count - start_tick) < delay)
    {
        // This is a blocking wait.
        // The subtraction handles the rollover of g_sys_tick_count correctly.
    }
    return 0;
}

uint32_t getmsTick(void)
{
    return g_sys_tick_count;
}

uint32_t wait_until(uint32_t delay)
{
    // This function behaves identically to ms_delay
    uint32_t start_tick = g_sys_tick_count;
    while ((g_sys_tick_count - start_tick) < delay) {}
    return 0;
}

void SYS_SLEEP_WFI(void)
{
    // Wait For Interrupt: a low-power mode where the CPU stops until an
    // interrupt (like SysTick) occurs.
    __WFI();
}