#include <dos.h>

#include "timer.h"

#define OLD_TIMER_RATE  0x10000L
#define TIMER_INT_NO    0x08

int  startNewTimer( word delay );
void stopNewTimer( void );
void setClockRate( word clock );

extern word userTimerFunc( void );

static void (interrupt far *oldTimer)( void );
static void interrupt newTimer( void );

static int isNewTimerInstalled = FALSE;
static long cycleCount, currentRate;
static word delayCall, delayCount;
unsigned long internalClock = 0;

int startNewTimer( word dc )
{
    if ( isNewTimerInstalled ) return FAIL;

    cycleCount = 0L;
    delayCount = 0;
    delayCall = dc;
    isNewTimerInstalled = TRUE;
    currentRate = OLD_TIMER_RATE;

    oldTimer = getvect( TIMER_INT_NO );
    setvect( TIMER_INT_NO, newTimer );

    return SUCCESS;
}

void stopNewTimer( void )
{
    if ( !isNewTimerInstalled ) return;
    isNewTimerInstalled = FALSE;
    setvect( TIMER_INT_NO, oldTimer );
    setClockRate( 0 );
}

void setClockRate( word clock )
{
    currentRate = clock;

    outportb( 0x43, 0x34 );
    outportb( 0x40, (byte)clock );
    outportb( 0x40, (byte)(clock >> 8) );
}

static void interrupt newTimer( void )
{
    internalClock++;

    cycleCount += currentRate;

    if ( cycleCount >= OLD_TIMER_RATE )
    {
        cycleCount -= OLD_TIMER_RATE;
        (*oldTimer)();
    }
    else
        outportb( 0x20, 0x20 );

    if ( delayCount++ >= delayCall )
    {
        delayCount = 0;
        delayCall = userTimerFunc();
    }
}
