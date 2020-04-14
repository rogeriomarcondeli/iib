
/////////////////////////////////////////////////////////////////////////////////////////////

/*
 * timer.c
 *
 *  Created on: 6 de set de 2017
 *      Author: allef.silva
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "board_drivers/hardware_def.h"

/////////////////////////////////////////////////////////////////////////////////////////////

volatile static uint32_t millis = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
static void int_timer_0_handler(void)
{

    //
    // Clear the timer interrupt.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    millis++;

}

/////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt. 100us
//
//*****************************************************************************

uint8_t mili_second = 0;

static void int_timer_1_handler(void)
{


    // Clear the timer interrupt.
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    task_100_us();

    if(mili_second >= 10)
    {
        task_1_ms();
        mili_second = 0;
    }
    else mili_second++;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void timer_init(void)
{

    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    //TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SYSCLOCK / 1000) - 1);
    //TimerLoadSet(TIMER1_BASE, TIMER_A, (SYSCLOCK / 10000) - 1);


    // Setup the interrupts for the timer timeouts.
    IntEnable(INT_TIMER0A);
    //IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    //TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerIntRegister(TIMER0_BASE, TIMER_A, int_timer_0_handler);
    //TimerIntRegister(TIMER1_BASE, TIMER_A, int_timer_1_handler);

    IntPrioritySet(INT_TIMER0A, 2);


    // Enable the timers.
    TimerEnable(TIMER0_BASE, TIMER_A);
    //TimerEnable(TIMER1_BASE, TIMER_A);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void timer_task_init(void)
{

    // Enable timer 1.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    //TimerLoadSet(TIMER1_BASE, TIMER_A, (SYSCLOCK / 10000) - 1);
    TimerLoadSet(TIMER1_BASE, TIMER_A, (SYSCLOCK / 1000) - 1);

    IntPrioritySet(INT_TIMER1A, 0);

    // Setup the interrupts for the timer timeouts.
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerIntRegister(TIMER1_BASE, TIMER_A, int_timer_1_handler);


    // Enable the timers.
    TimerEnable(TIMER1_BASE, TIMER_A);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void delay_ms(uint32_t time)
{
    volatile uint32_t temp = millis;
    while ((millis - temp) < time);
}

/////////////////////////////////////////////////////////////////////////////////////////////



