/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */
#include "stdbool.h"
/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR( void );

/* ----------------------- Start implementation -----------------------------*/
uint16_t timeout = 0;

BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
    timeout = (usTim1Timerout50us == 0) ? 1 : usTim1Timerout50us;

    // Enable TIM2
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;
    // Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;

    // We need a prescaler here so we can count up to seconds
    TIM2->PSC = (FUNCONF_SYSTEM_CORE_CLOCK / 1000 / 20) - 1;

    TIM2->CTLR1 |= TIM_ARPE | TIM_CounterMode_Down;


    // Reload immediately
    TIM2->SWEVGR |= TIM_PSCReloadMode_Immediate;

    TIM2->DMAINTENR |= TIM_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);

    return TRUE;
}


inline void
vMBPortTimersEnable(  )
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */

    // Reload counter
    TIM2->ATRLR = timeout*2 - 1;
    

    // Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;
    TIM2->DMAINTENR |= TIM_UIE;
}

inline void
vMBPortTimersDisable(  )
{
    /* Disable any pending timers. */

    // Disable TIM2
	TIM2->CTLR1 &= ~TIM_CEN;
    TIM2->DMAINTENR &= ~TIM_UIE;
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR( void )
{
    ( void )pxMBPortCBTimerExpired(  );
}



    
void TIM2_IRQHandler(void) INTERRUPT_HANDLER;
void TIM2_IRQHandler(void)
{
    prvvTIMERExpiredISR(); 
    TIM2->INTFR = ~TIM_FLAG_Update;
    TIM2->SWEVGR &= TIM_UG;
}
