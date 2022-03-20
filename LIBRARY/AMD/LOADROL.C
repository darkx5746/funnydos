#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include "rol2amd.h"


static int loadTempoEvent( void );
static int loadNoteEvent( Voice_Event * );
static int loadInstrEvent( Voice_Event * );
static int loadVolumeEvent( Voice_Event * );
static int loadPitchEvent( Voice_Event * );
static int generateInstrTable( void );


Rol_Header		rolHd;
Rol_InstrData	instrTable[ROL_MAX_INSTR_NUM];

word instrNum;
word voiceNum;
word tempoChanges;

Tempo_Event *tempo;
Voice_Event *voice;

static FILE *rolFp;


int loadRol( char *fname )
{
    int i;

	if ( rolFp ) closeRol();

	if ( !(rolFp = fopen( fname, "rb" )) )
        errExit( "ROL file open error!" );

    /* read ROL header */
	fread( &rolHd, sizeof( Rol_Header ), 1, rolFp );
    if ( (rolHd.majorVersion != ROL_MAJ_VERSION)
        || (rolHd.minorVersion != ROL_MIN_VERSION) )
            errExit( "Invalid ROL file." );

    voiceNum = ( rolHd.musicMode ? 9 : 11 );

    loadTempoEvent();

    if ( !(voice = malloc( voiceNum * sizeof(Voice_Event) )) )
        errExit( ERR_MEMORY );

    for ( i = 0; i < voiceNum; i++ )
	{
        loadNoteEvent( &voice[i] );
        loadInstrEvent( &voice[i] );
        loadVolumeEvent( &voice[i] );
        loadPitchEvent( &voice[i] );
	}

    generateInstrTable();

	return SUCCESS;
}

int closeRol( void )
{
	int i;

	if ( !rolFp ) return FAIL;

    for ( i = 0; i < voiceNum; i++ )
	{
        free( voice[i].note );
        free( voice[i].instr );
        free( voice[i].volume );
        free( voice[i].pitch );
	}
    free( voice );

    free( tempo );

    fclose( rolFp );

	return SUCCESS;
}

static int loadTempoEvent( void )
{
	fread( &tempoChanges, 2, 1, rolFp );
    if ( !tempoChanges ) return SUCCESS;
    if ( !(tempo = malloc( (tempoChanges + 1) * sizeof(Tempo_Event) )) )
        errExit( ERR_MEMORY );
	if ( fread( tempo, sizeof(Tempo_Event), tempoChanges, rolFp ) != tempoChanges )
        errExit( "Tempo event reading error." );

    return SUCCESS;
}

static int loadNoteEvent( Voice_Event *ve )
{
    int i, len;
    char idString[ROL_IDSTR_SIZE];

	fread( &idString, ROL_IDSTR_SIZE, 1, rolFp );
	fread( &ve->noteTotLen, 2, 1, rolFp );
    if ( !ve->noteTotLen )
    {
        ve->noteChanges = 0;
        return SUCCESS;
    }
	if ( !(ve->note = (Note_Event *)malloc( (word)32000 )) )
        errExit( ERR_MEMORY );
    for ( len = i = 0; len < ve->noteTotLen; len += ve->note[i++].length )
        if ( !fread( &ve->note[i], sizeof(Note_Event), 1, rolFp ) )
            errExit( "Note event reading error." );
    if ( !(ve->note = realloc( ve->note, i * sizeof(Note_Event) )) )
        errExit( ERR_MEMORY );
    ve->noteChanges = i;

    return SUCCESS;
}

static int loadInstrEvent( Voice_Event *ve )
{
	char idString[ROL_IDSTR_SIZE];

	fread( &idString, ROL_IDSTR_SIZE, 1, rolFp );
	fread( &ve->instrChanges, 2, 1, rolFp );
    if ( !ve->instrChanges ) return SUCCESS;
    if ( !(ve->instr = malloc( (ve->instrChanges + 1) * sizeof(Instr_Event) )) )
        errExit( ERR_MEMORY );
    if ( fread( ve->instr, sizeof(Instr_Event), ve->instrChanges, rolFp ) != ve->instrChanges )
        errExit( "Instrument event reading error" );

    return SUCCESS;
}

static int loadVolumeEvent( Voice_Event *ve )
{
	char idString[ROL_IDSTR_SIZE];

	fread( &idString, ROL_IDSTR_SIZE, 1, rolFp );
	fread( &ve->volumeChanges, 2, 1, rolFp );
    if ( !ve->volumeChanges ) return SUCCESS;
    if ( !(ve->volume = malloc( (ve->volumeChanges + 1) * sizeof(Volume_Event) )) )
        errExit( ERR_MEMORY );
    if ( fread( ve->volume, sizeof(Volume_Event), ve->volumeChanges, rolFp ) != ve->volumeChanges )
        errExit( "Volume event reading error" );

    return SUCCESS;
}

static int loadPitchEvent( Voice_Event *ve )
{
	char idString[ROL_IDSTR_SIZE];

	fread( &idString, ROL_IDSTR_SIZE, 1, rolFp );
	fread( &ve->pitchChanges, 2, 1, rolFp );
    if ( !ve->pitchChanges ) return SUCCESS;
    if ( !(ve->pitch = malloc( (ve->pitchChanges + 1) * sizeof(Pitch_Event) )) )
        errExit( ERR_MEMORY );
    if ( fread( ve->pitch, sizeof(Pitch_Event), ve->pitchChanges, rolFp ) != ve->pitchChanges )
        errExit( "Pitch event reading error" );

    return SUCCESS;
}

static int generateInstrTable( void )
{
	int i, j, k;
    int instrIndex = 0;
    char *instrName;

    for ( i = 0; i < voiceNum; i++ )
		for ( j = 0; (j < voice[i].instrChanges) && (instrIndex < ROL_MAX_INSTR_NUM); j++ )
        {
            instrName = voice[i].instr[j].instrName;
			for ( k = 0; k < instrIndex; k++ )
				if ( !stricmp( instrName, instrTable[k].name ) )
                {   /* found same instrument in table */
                    voice[i].instr[j].instrIndex = k;
                    break;
                }

            if ( k == instrIndex ) /* new instrument */
            {
                voice[i].instr[j].instrIndex = instrIndex;
				strcpy( instrTable[instrIndex++].name, instrName );
            }
        }

    instrNum = instrIndex;

	return SUCCESS;
}

