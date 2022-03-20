#ifndef __ADLIB__
#define __ADLIB__

#include "common.h"

/* melodic voice numbers: */
#define VOICE_MELO0     0
#define VOICE_MELO1     1
#define VOICE_MELO2     2
#define VOICE_MELO3     3
#define VOICE_MELO4     4
#define VOICE_MELO5     5
#define VOICE_MELO6     6
#define VOICE_MELO7     7
#define VOICE_MELO8     8

/* percussive voice numbers: */
#define VOICE_BD        6
#define VOICE_SD        7
#define VOICE_TOM       8
#define VOICE_CYMB      9
#define VOICE_HIHAT     10

#define MAX_VOLUME      0x7f
#define MAX_PITCH       0x3fff
#define MID_PITCH       0x2000

/* function prototypes */

extern int adlibInstalled( void );
extern void initSound( void );
extern void setPercMode( int );

extern void setVoiceInstr( word voice, byte *paramArray );
extern void setVoiceVolume( word voice, word volume );
extern void setVoicePitch( word voice, word pitchBend );
extern void noteOn( word voice, word pitch );
extern void noteOff( word voice );

#endif

