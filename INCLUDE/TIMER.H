#ifndef __TIMER__
#define __TIMER__

#include "common.h"

#define getClock() (internalClock)

extern int  startNewTimer( word delay );
extern void stopNewTimer ( void );
extern void setClockRate ( word clock );

extern word userTimerFunc( void );

extern unsigned long internalClock;

#endif
