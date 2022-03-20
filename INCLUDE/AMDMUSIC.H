#ifndef __AMD_MUSIC__
#define __AMD_MUSIC__

#include "amd.h"

#define BASIC_CLOCK_HZ      500

typedef enum { STOPPED = 0, PAUSED, PLAYING } Music_Status;

typedef struct {        /* instrument data actually needed */
    byte opr0Prm[13];
    byte opr1Prm[13];
    byte opr0WaveSel;
    byte opr1WaveSel;
} InstrDef;

typedef struct AMD_Music_t {
    struct AMD_Music_t *prev, *next;    /* linked list */

    byte percMode;              /* melody/percussive */
    byte disableVoice;          /* voice to disable */
    byte autoPlayMode;          /* OFF = stop after music end,
                                   ON = replay after music end, */
    byte setPosFlag;            /* set when amdSetPos() called */
    byte voiceNum;              /* ( percMode ? 11 : 9 ) */

    word totalTicks;            /* total playing ticks */
    word ticksPerMin;           /* ticksPerBeat * beatsPerMinute */

    word curTick;               /* indicate current playing tick */
    word curDelay;              /* userTimerFunc()'s delay value */
    word curTempo;              /* relative percent data */
    word curVolume[11];         /* relative percent data */

    word instrNum;              /* instrument number used */
    InstrDef *instrTable;       /* instrument data table */

    word tempoLen, voiceLen[11][4];     /* count each event length */

    AMD_TempoEvent *tempoPt, *tempoEvent;
	AMD_VoiceEvent *voicePt[11][4], *voiceEvent[11][4];
} AMD_Music;

int amdInitMusic( void );
int amdCloseMusic( void );

AMD_Music *amdOpen( const char *fname, int autoPlayMode );
int amdClose( AMD_Music * );

int amdPlay( AMD_Music * );
int amdStop( void );
int amdPause( void );
int amdContinue( void );
int amdRewind( AMD_Music * );

Music_Status amdGetMusicStatus( void );
word amdGetSize( AMD_Music * );
word amdGetPos( AMD_Music * );
int amdSetPos( AMD_Music *, word tick );

word amdSetTempo( word percent );
word amdSetVolume( word percent );

#endif
