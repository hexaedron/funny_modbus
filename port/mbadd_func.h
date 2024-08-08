#ifndef _ADD_FUNC_H
#define _ADD_FUNC_H

#include "ch32v003fun.h"

typedef enum
{
    MB_STOP_1,  
    MB_STOP_0_5,
    MB_STOP_1_5,
    MB_STOP_2
} eMBStopBits;

static void 
xMBPortSerialSetStopBits( eMBStopBits stopBits )
{
    switch (stopBits)
    {
        case MB_STOP_1:
            USART1->CTLR2 = USART_StopBits_1;
        break;

        case MB_STOP_1_5:
            USART1->CTLR2 = USART_StopBits_1_5;
        break;

        case MB_STOP_0_5:
            USART1->CTLR2 = USART_StopBits_0_5;
        break;

        case MB_STOP_2:
            USART1->CTLR2 = USART_StopBits_2;
        break;
    
        default:
            USART1->CTLR2 = USART_StopBits_1;
        break;
    }
}



#endif