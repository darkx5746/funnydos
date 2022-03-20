#include <stdio.h>
#include <string.h>

#include "rol2amd.h"

static void convertHeader( void );
static void convertInstrTable( void );
static void convertTempoData( void );
static void convertVoiceData( void );
static void convertNoteData( Voice_Event *ve );
static void convertInstrData( Voice_Event *ve );
static void convertVolumeData( Voice_Event *ve );
static void convertPitchData( Voice_Event *ve );

static FILE *amdFp;
static AMD_Header amdHd;

int convertRol2Amd( char *fname )
{
    long calcSize;

	if ( !(amdFp = fopen( fname, "wb+" )) )
		errExit( "AMD file open error" );

	convertHeader();
	convertInstrTable();
	convertTempoData();
	convertVoiceData();

    calcSize = amdHd.sizeOfHeader + amdHd.sizeOfInstrTable
             + amdHd.sizeOfTempoData + amdHd.sizeOfVoiceData;

	if ( ftell(amdFp) != calcSize )
		errExit( "Converting fail!\b" );

	return SUCCESS;
}

static void convertHeader( void )
{
    Voice_Event *ve;
	word i;

	strcpy( amdHd.id, AMD_ID_STR );
	amdHd.majVer = AMD_MAJ_VER;
	amdHd.minVer = AMD_MIN_VER;
	amdHd.mode = !rolHd.musicMode;
	amdHd.ticksPerBeat = rolHd.ticksPerBeat;
	amdHd.beatsPerMinute = rolHd.basicTempo;
	amdHd.beatsPerMeasure = rolHd.beatsPerMeasure;
	amdHd.sizeOfHeader = sizeof(AMD_Header);
	amdHd.sizeOfInstrTable = instrNum * sizeof(AMD_InstrTable) + 2;
	amdHd.sizeOfTempoData = tempoChanges * sizeof(AMD_TempoEvent) + 2;

	for ( i = amdHd.totalTicks = 0, amdHd.sizeOfVoiceData = 0; i < voiceNum; i++ )
	{
        ve = &voice[i];
		amdHd.totalTicks = ( ve->noteTotLen > amdHd.totalTicks ) ? ve->noteTotLen : amdHd.totalTicks;
		amdHd.sizeOfVoiceData += ( ve->noteChanges + ve->instrChanges
			+ ve->volumeChanges + ve->pitchChanges ) * sizeof(AMD_VoiceEvent)
			+ 2 * 4;
	}

	fwrite( &amdHd, sizeof(AMD_Header), 1, amdFp );

    printf( "Music mode     = %s\n", (amdHd.mode ? "Percussive" : "Melody") );
    printf( "TotalTicks     = %u\n", amdHd.totalTicks );
    printf( "TicksPerMinute = %u\n", amdHd.ticksPerBeat * amdHd.beatsPerMinute );
}

static void convertInstrTable( void )
{
    printf( "Instruments    = %d\n", instrNum );
	fwrite( &instrNum, 2, 1, amdFp );
	fwrite( &instrTable[0], sizeof(AMD_InstrTable), instrNum, amdFp );
}

static void convertTempoData( void )
{
    word i, len, newTempo;

    printf( "Tempo changes  = %u\n\n", tempoChanges );
    fwrite( &tempoChanges, 2, 1, amdFp );
    if ( !tempoChanges ) return;

    tempo[tempoChanges].ticks = amdHd.totalTicks;
    for ( i = 0; i < tempoChanges; i++ )
	{
        len = tempo[i+1].ticks - tempo[i].ticks;
        fwrite( &len, 2, 1, amdFp );
        newTempo = tempo[i].tempo * 100;
		fwrite( &newTempo, 2, 1, amdFp );
	}
}

static void convertVoiceData( void )
{
	int i;

    for ( i = 0; i < voiceNum; i++ )
	{
        printf( "Voice %02d : ", i );
		convertNoteData( &voice[i] );
		convertInstrData( &voice[i] );
		convertVolumeData( &voice[i] );
		convertPitchData( &voice[i] );
		printf( "\n" );
	}
}

static void convertNoteData( Voice_Event *ve )
{
	word i;

	printf( "note = %5u, ", ve->noteChanges );
	fwrite( &ve->noteChanges, 2, 1, amdFp );
	if ( !ve->noteChanges ) return;

	for ( i = 0; i < ve->noteChanges; i++ )
	{
		fwrite( &ve->note[i].length, 2, 1, amdFp );
		fwrite( (byte *) &ve->note[i].pitch, 1, 1, amdFp );
	}
}

static void convertInstrData( Voice_Event *ve )
{
    word i, len;

    printf( "instr = %5u, ", ve->instrChanges );
    fwrite( &ve->instrChanges, 2, 1, amdFp );
	if ( !ve->instrChanges ) return;

    ve->instr[ve->instrChanges].ticks = amdHd.totalTicks;
    for ( i = 0; i < ve->instrChanges; i++ )
    {
        len = ve->instr[i+1].ticks - ve->instr[i].ticks;
        fwrite( &len, 2, 1, amdFp );
        fwrite( &ve->instr[i].instrIndex, 1, 1, amdFp );
    }
}

static void convertVolumeData( Voice_Event *ve )
{
    word i, len;
    byte newVolume;

    printf( "volume = %5u, ", ve->volumeChanges );
    fwrite( &ve->volumeChanges, 2, 1, amdFp );
    if ( !ve->volumeChanges ) return;

    ve->volume[ve->volumeChanges].ticks = amdHd.totalTicks;
    for ( i = 0; i < ve->volumeChanges; i++ )
	{
        len = ve->volume[i+1].ticks - ve->volume[i].ticks;
        fwrite( &len, 2, 1, amdFp );
        newVolume = ve->volume[i].volume * 100;
		fwrite( &newVolume, 1, 1, amdFp );
	}
}

static void convertPitchData( Voice_Event *ve )
{
    word i, len;
	byte newPitch;

    printf( "pitch = %5u", ve->pitchChanges );
    fwrite( &ve->pitchChanges, 2, 1, amdFp );
    if ( !ve->pitchChanges ) return;

    ve->pitch[ve->pitchChanges].ticks = amdHd.totalTicks;
    for ( i = 0; i < ve->pitchChanges; i++ )
	{
        len = ve->pitch[i+1].ticks - ve->pitch[i].ticks;
        fwrite( &len, 2, 1, amdFp );
		newPitch = ve->pitch[i].pitchAccuracy * 100;
		fwrite( &newPitch, 1, 1, amdFp );
	}
}

