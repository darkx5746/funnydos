#ifndef __ROL__
#define __ROL__

#include "common.h"

#define ROL_MAJ_VERSION 0
#define ROL_MIN_VERSION 4
#define ROL_IDSTR_SIZE  15

typedef struct {
    int ticks;
    float tempo;
} Tempo_Event;

typedef struct {
    int pitch;
    int length;
} Note_Event;

typedef struct {
    int ticks;
    char instrName[9];
    char filler;
    int instrIndex;
} Instr_Event;

typedef struct {
    int ticks;
    float volume;
} Volume_Event;

typedef struct {
    int ticks;
    float pitchAccuracy;
} Pitch_Event;

typedef struct {
    word            noteTotLen;
    word            noteChanges;
    Note_Event      *note;
    word            instrChanges;
    Instr_Event     *instr;
    word            volumeChanges;
    Volume_Event    *volume;
    word            pitchChanges;
    Pitch_Event     *pitch;
} Voice_Event;

typedef struct {
    int   majorVersion;
    int   minorVersion;
    char  filler1[40];
    int   ticksPerBeat;
    int   beatsPerMeasure;
    int   axisY, axisX;
    char  filler2;
    byte  musicMode;
    byte  filler3[90+38+15];
    float basicTempo;
} Rol_Header;

#endif

