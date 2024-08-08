#include "ch32v003fun.h"

#include <stdbool.h>
#include <stdlib.h> 
#include <math.h>

#ifndef WCH_FAST_INTERRUPT_ENABLED
 #define WCH_FAST_INTERRUPT_ENABLED
#endif

#ifdef WCH_FAST_INTERRUPT_ENABLED
  #define INTERRUPT_HANDLER __attribute__((interrupt("WCH-Interrupt-fast")))
#else
  #define INTERRUPT_HANDLER __attribute__((interrupt)) 
#endif

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mbadd_func.h"


// from system.cpp
void system_initSystick();
void delay_ms(uint32_t);

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];

/* ----------------------- Start implementation -----------------------------*/
int main()
 {
	SystemInit();
	#ifdef WCH_FAST_INTERRUPT_ENABLED
		__set_INTSYSCR(0x3); // [Experimental] enable fast interrupt feature	
	#endif
	//system_initSystick();
	
	const UCHAR     ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
    eMBErrorCode    eStatus;

    eStatus = eMBInit( MB_RTU, 0x0A, 0, 115200, MB_PAR_NONE);
    xMBPortSerialSetStopBits(MB_STOP_2);

    eStatus = eMBSetSlaveID( 0x34, TRUE, ucSlaveID, 3 );

    usRegInputBuf[1] = 0xDEAD;
    usRegInputBuf[2] = 0xBEEF;

    /* Enable the Modbus Protocol Stack. */
    eStatus = eMBEnable(  );

    for( ;; )
    {
        ( void )eMBPoll(  );

        /* Here we simply count the number of poll cycles. */
        usRegInputBuf[0]++;
    }
		
}


eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart - 1);
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
    return MB_ENOREG;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
