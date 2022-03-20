#ifndef __ADLIB_EFFECT__
#define __ADLIB_EFFECT__

#include "common.h"
#include "amdmusic.h"

/* volume 0 - 100 */
extern word setAdlibEffectVolume( word volume );
/* voice 0 - 10 */
extern word setAdlibEffectVoice( word voice );

/* pitch 0 - 60 */
extern int  adlibEffectOn( int pitch, InstrDef *instr );
extern void adlibEffectOff( void );

#endif

