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

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mbconfig.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    USART1->CTLR1 &= ~(USART_FLAG_RXNE | USART_FLAG_TXE);

    if(xRxEnable) 
    {
        USART1->CTLR1 |= USART_FLAG_RXNE;
    }

    if(xTxEnable) 
    {
        USART1->CTLR1 |= USART_FLAG_TXE;
    }
}


#define APB_CLOCK FUNCONF_SYSTEM_CORE_CLOCK
#define OVER8DIV 4

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    // Hardware Serial Pins D5 / D6
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1; // Enable UART
    GPIOD->CFGLR &= ~(0xf<<(4*5));
    GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);

    GPIOD->CFGLR &= ~(0xf<<(4*6));
    GPIOD->CFGLR |= (GPIO_CNF_IN_PUPD)<<(4*6);

    uint16_t wordLength = (eParity == MB_PAR_NONE) ? USART_WordLength_8b : USART_WordLength_9b;
    uint16_t parity;

    switch (eParity)
    {
        case MB_PAR_NONE:
            parity =  USART_Parity_No;
        break;

        case MB_PAR_ODD:
            parity =  USART_Parity_Odd;
        break;

        case MB_PAR_EVEN:
            parity =  USART_Parity_Even;
        break;
    
        default:
            parity =  USART_Parity_No;
        break;
    }

    USART1->CTLR1 = wordLength | parity | USART_Mode_Rx | USART_Mode_Tx;
    USART1->CTLR2 = USART_StopBits_1;
    USART1->CTLR3 = USART_HardwareFlowControl_None;

    // Set Baudrate
    uint32_t integerDivider = ((25 * APB_CLOCK)) / (OVER8DIV * ulBaudRate);
    uint32_t fractionalDivider = integerDivider % 100;

    USART1->BRR = ((integerDivider / 100) << 4) | (((fractionalDivider * (OVER8DIV * 2) + 50) / 100) & 7);

    // Enable UART
    USART1->CTLR1 |= CTLR1_UE_Set;

    // Enable Interrupt
    NVIC_EnableIRQ(USART1_IRQn);

    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    USART1->DATAR = ucByte;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    while(!(USART1->STATR & USART_FLAG_RXNE));
    *pucByte = USART1->DATAR & (uint16_t)0x01FF;
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void USART1_IRQHandler( void ) INTERRUPT_HANDLER;
void USART1_IRQHandler(void) 
{
    if(USART1->STATR & USART_FLAG_RXNE) 
    {
        prvvUARTRxISR();
        return;
    }

    if(USART1->STATR & USART_FLAG_TXE) 
    {
        prvvUARTTxReadyISR();
        return;
    }
}