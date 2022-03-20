/*
    AMD - Adlib Music Data - by Gapacci.
    ver 1.0    1994/03/06
*/

#ifndef __AMD__
#define __AMD__

#include "common.h"


#define AMD_ID_STR      "AMD"
#define AMD_MAJ_VER     1
#define AMD_MIN_VER     0


typedef struct {
    byte mode, percVoice;
    byte op0[13];
    byte op1[13];
    byte wave0, wave1;
} AMD_Instr;

typedef struct {
    AMD_Instr instr;
	char name[9];
	byte flag;
} AMD_InstrTable;

typedef struct {
    word length;
    word data;
} AMD_TempoEvent;

typedef struct {
    word length;
    byte data;
} AMD_VoiceEvent;

typedef struct {
    char id[4];             /* "AMD" */
    byte majVer;
    byte minVer;
    word mode;              /* percussive, melody */
    word totalTicks;        /* total playing time */
    word ticksPerBeat;
    word beatsPerMinute;    /* basic tempo */
    word beatsPerMeasure;
    long sizeOfHeader;
    long sizeOfInstrTable;
    long sizeOfTempoData;
    long sizeOfVoiceData;
} AMD_Header;

#endif

