#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amd.h"
#include "adlib.h"
#include "amdmusic.h"
#include "timer.h"

#define NOTE_EVENT      0
#define INSTR_EVENT     1
#define VOLUME_EVENT    2
#define PITCH_EVENT     3
#define VOICE_EVENT_NUM 4


/* function prototypes ------------------------------------------------ */


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

word userTimerFunc( void );

static void closeNext( AMD_Music * );
static void linkMusic( AMD_Music * );
static void unlinkMusic( AMD_Music * );


/* global variables --------------------------------------------------- */


static Music_Status musicStatus = STOPPED;
static AMD_Music    *curMusic = NULL;
static AMD_Music    *musicList = NULL;
static word         basicTempo = 100;
static word         basicVolume = 100;
static word         tmpVol = MAX_VOLUME;


/* global fuctions ---------------------------------------------------- */


int amdInitMusic( void )
{
	initSound();

	startNewTimer( 0 );
	setClockRate( (word)(1193180L / BASIC_CLOCK_HZ) );

	return ( adlibInstalled() ? SUCCESS : FAIL );
}

int amdCloseMusic( void )
{
    if ( musicList ) closeNext( musicList );

    setClockRate( 0 );
    stopNewTimer();

	return SUCCESS;
}

AMD_Music *amdOpen( const char *fname, int autoPlayMode )
{
    int i, j;
	word eventCount;
    AMD_Header hd;
	AMD_Music *amPt;
    AMD_InstrTable tmpInstr;
    FILE *fp;

    if ( !(fp = fopen( fname, "rb" )) )
	{
        setError( "AMD file open error." );
        return NULL;
	}

    /* load & check header */

    fread( &hd, sizeof(AMD_Header), 1, fp );
    if ( strcmp( hd.id, AMD_ID_STR ) )
	{
        setError( "Not AMD file." );
		return NULL;
    }
	if ( (hd.majVer != AMD_MAJ_VER) || (hd.minVer != AMD_MIN_VER) )
    {
		setError( "Different AMD version." );
        return NULL;
    }

    /* init amPt */

    if ( !(amPt = malloc( sizeof(AMD_Music) )) )
    {
		setError( ERR_MEMORY );
        return NULL;
    }

    amPt->percMode = hd.mode;
    amPt->disableVoice = autoPlayMode >> 1;
    amPt->autoPlayMode = autoPlayMode & ON;
    amPt->setPosFlag = OFF;
    amPt->voiceNum = ( hd.mode ? 11 : 9 );
	amPt->totalTicks = hd.totalTicks;
    amPt->ticksPerMin = hd.ticksPerBeat * hd.beatsPerMinute;
	amPt->curTick = 0;
    amPt->curTempo = 100;

    /* load instrument table */

    fread( &amPt->instrNum, 2, 1, fp );

	if ( !(amPt->instrTable =
         malloc( amPt->instrNum * sizeof(InstrDef) )) )
    {
		setError( ERR_MEMORY );
        return NULL;
    }

    for ( i = 0; i < amPt->instrNum; i++ )
    {
        fread( &tmpInstr, sizeof(AMD_InstrTable), 1, fp );
        memcpy( &amPt->instrTable[i], (byte *)&tmpInstr + 2, 28 );
	}

	/* load tempo data */

	fread( &eventCount, 2, 1, fp );

    if ( !(amPt->tempoEvent =
        malloc( (eventCount+1) * sizeof(AMD_TempoEvent) )) )
    {
		setError( ERR_MEMORY );
        return NULL;
    }

    fread( amPt->tempoEvent, sizeof(AMD_TempoEvent), eventCount, fp );
    amPt->tempoEvent[eventCount].length = amPt->totalTicks -
        ( eventCount ? amPt->tempoEvent[eventCount-1].length : 0 );
    amPt->tempoEvent[eventCount].data = 100;
    amPt->tempoPt = amPt->tempoEvent;

    /* load voice data */

    for ( i = 0; i < amPt->voiceNum; i++ )
		for ( j = 0; j < VOICE_EVENT_NUM; j++ )
        {
			fread( &eventCount, 2, 1, fp );
            if ( !(amPt->voiceEvent[i][j] =
                malloc( (eventCount+1) * sizeof(AMD_VoiceEvent) )) )
            {
                setError( ERR_MEMORY );
				return NULL;
            }

			fread( amPt->voiceEvent[i][j], sizeof(AMD_VoiceEvent),
                eventCount, fp );
            amPt->voiceEvent[i][j][eventCount].length = amPt->totalTicks -
                ( eventCount ? amPt->voiceEvent[i][j][eventCount-1].length
                : 0 );
            amPt->voiceEvent[i][j][eventCount].data = 0;
            amPt->voicePt[i][j] = amPt->voiceEvent[i][j];   
        }

    if ( ftell(fp) != hd.sizeOfHeader + hd.sizeOfInstrTable +
		hd.sizeOfTempoData + hd.sizeOfVoiceData )
    {
		setError( "AMD file size conflicts." );
        return NULL;
    }

    fclose( fp );

    linkMusic( amPt );

	return amPt;
}

int amdClose( AMD_Music *amPt )
{
    int i, j;

    if ( !amPt ) return FAIL;

    if ( (curMusic == amPt) && (musicStatus != STOPPED) )
		amdStop();

	free( amPt->instrTable );
    free( amPt->tempoEvent );

    for ( i = 0; i < amPt->voiceNum; i++ )
        for ( j = 0; j < VOICE_EVENT_NUM; j++ )
			free( amPt->voiceEvent[i][j] );

    unlinkMusic( amPt );

    free( amPt );

    return SUCCESS;
}

int amdPlay( AMD_Music *amPt )
{
	if ( !amPt ) return FAIL;

	if ( curMusic == amPt )
	{
		if ( musicStatus == PAUSED )
		{
			amdContinue();
			return SUCCESS;
		}
		else if ( musicStatus == PLAYING )
			return SUCCESS;
	}
	else
		amdStop();

	if ( amPt->curTick >= amPt->totalTicks ) return FAIL;

	curMusic = amPt;
	setPercMode( amPt->percMode );

	if ( !amdSetPos( amPt, amPt->curTick ) ) return FAIL;

	amPt->curDelay = BASIC_CLOCK_HZ * 60 / amPt->ticksPerMin;
	musicStatus = PLAYING;

	return SUCCESS;
}

int amdStop( void )
{
	int i;

	if ( !curMusic ) return FAIL;

    if ( musicStatus != STOPPED )
    {
        musicStatus = STOPPED;

        for ( i = 0; i < curMusic->voiceNum; i++ )
			noteOff(i);
	}

	return SUCCESS;
}

int amdPause( void )
{
	int i;

	if ( (musicStatus == STOPPED ) || !curMusic ) return FAIL;

    if ( musicStatus == PAUSED ) return SUCCESS;

    musicStatus = PAUSED;
	for ( i = 0; i < curMusic->voiceNum; i++ )
        noteOff(i);

    return SUCCESS;
}

int amdContinue( void )
{
	if ( musicStatus != PAUSED ) return FAIL;

	musicStatus = PLAYING;

	return SUCCESS;
}

int amdRewind( AMD_Music *amPt )
{
    int i, j;
    Music_Status savedStatus;

	if ( !amPt ) return FAIL;

    savedStatus = musicStatus;
    if ( curMusic == amPt )
        musicStatus = PAUSED;

	amPt->curTick = 0;
    amPt->tempoPt = amPt->tempoEvent;
	amPt->tempoLen = 0;

	for ( i = 0; i < amPt->voiceNum; i++ )
    {
		for ( j = 0; j < VOICE_EVENT_NUM; j++ )
		{
			amPt->voicePt[i][j] = amPt->voiceEvent[i][j];
			amPt->voiceLen[i][j] = 0;
        }

        if ( curMusic == amPt )
        {
			noteOff( i );
            setVoiceVolume( i, 0 );
        }
    }

	musicStatus = savedStatus;

    return SUCCESS;
}

Music_Status amdGetMusicStatus( void )
{
	return musicStatus;
}

word amdGetSize( AMD_Music *amPt )
{
    if ( !amPt ) return 0;

    return amPt->totalTicks;
}

word amdGetPos( AMD_Music *amPt )
{
    if ( !amPt ) return 0;

	return amPt->curTick;
}

int amdSetPos( AMD_Music *amPt, word tick )
{
    int i, j;
	word len;
	AMD_TempoEvent *tempoPt;
	AMD_VoiceEvent *voicePt;
	Music_Status savedStatus;

    if ( !amPt ) return FAIL;

    if ( tick >= amPt->totalTicks ) return FAIL;

    if ( tick == 0 ) return amdRewind( amPt );

    savedStatus = musicStatus;
    if ( curMusic == amPt );
		musicStatus = PAUSED;

	amPt->setPosFlag = ON;
	amPt->curTick = tick;

	tempoPt = amPt->tempoEvent;
    len = tempoPt->length;
	while ( len < tick )
		len += (++tempoPt)->length;
	amPt->curTempo = tempoPt->data;
	amPt->tempoPt = tempoPt + 1;
    amPt->tempoLen = len;

    for ( i = 0; i < amPt->voiceNum; i++ )
        for ( j = 0; j < VOICE_EVENT_NUM; j++ )
		{
            voicePt = amPt->voiceEvent[i][j];
            len = voicePt->length;
            while ( len < tick )
                len += (++voicePt)->length;
			amPt->voicePt[i][j] = voicePt + 1;
			amPt->voiceLen[i][j] = len;
		}

    musicStatus = savedStatus;

	return SUCCESS;
}

word amdSetTempo( word percent )
{
	basicTempo = ( percent ? percent : 1 ); /* avoid 'devide by zero' */

	if ( curMusic )
		curMusic->curDelay = BASIC_CLOCK_HZ * 60 / curMusic->ticksPerMin
			* 100 / curMusic->curTempo * 100 / basicTempo;

	return basicTempo;
}

word amdSetVolume( word percent )
{
	int i;

	basicVolume = ( percent > 100 ? 100 : percent );
	tmpVol = MAX_VOLUME * basicVolume / 100;

	if ( curMusic )
		for ( i = 0; i < curMusic->voiceNum; i++ )
			setVoiceVolume( i, (curMusic->curVolume[i] * tmpVol / 100) );

	return basicVolume;
}

word userTimerFunc( void )
{
	int i;
	byte instrIndex, noteData;
	word *tmpLen;
	AMD_VoiceEvent **tmpVp;

	if ( !curMusic ) return 0;

	if ( musicStatus != PLAYING ) return 0;

	if ( curMusic->tempoLen == curMusic->curTick )
	{
		curMusic->tempoLen += curMusic->tempoPt->length;
		curMusic->curTempo = (curMusic->tempoPt++)->data;
		curMusic->curDelay = BASIC_CLOCK_HZ * 60 / curMusic->ticksPerMin
			* 100 / curMusic->curTempo * 100 / basicTempo;
	}

	for ( i = 0; i < curMusic->voiceNum; i++ )
	{
        if ( curMusic->disableVoice == i+1 ) continue;

		tmpVp = curMusic->voicePt[i];
		tmpLen = curMusic->voiceLen[i];

		if ( curMusic->setPosFlag == ON )
        {
			noteOff( i );

            instrIndex = (tmpVp[INSTR_EVENT]-1)->data;
			setVoiceInstr( i, (byte *)&curMusic->instrTable[instrIndex] );
            curMusic->curVolume[i] = (tmpVp[VOLUME_EVENT]-1)->data;
			setVoiceVolume( i, (curMusic->curVolume[i] * tmpVol / 100) );
            setVoicePitch( i, (tmpVp[PITCH_EVENT]-1)->data * 0x52 );
		}

        if ( tmpLen[NOTE_EVENT] == curMusic->curTick )
            noteOff( i );

        if ( tmpLen[INSTR_EVENT] == curMusic->curTick )
		{
			tmpLen[INSTR_EVENT] += tmpVp[INSTR_EVENT]->length;
            instrIndex = (tmpVp[INSTR_EVENT]++)->data;
            setVoiceInstr( i, (byte *)&curMusic->instrTable[instrIndex] );
		}

        if ( tmpLen[VOLUME_EVENT] == curMusic->curTick )
		{
			tmpLen[VOLUME_EVENT] += tmpVp[VOLUME_EVENT]->length;
            curMusic->curVolume[i] = (tmpVp[VOLUME_EVENT]++)->data;
			setVoiceVolume( i, (curMusic->curVolume[i] * tmpVol / 100) );
        }

        if ( tmpLen[PITCH_EVENT] == curMusic->curTick )
		{
            tmpLen[PITCH_EVENT] += tmpVp[PITCH_EVENT]->length;
            setVoicePitch( i, (tmpVp[PITCH_EVENT]++)->data * 0x52 );
        }

        if ( tmpLen[NOTE_EVENT] == curMusic->curTick )
		{
			tmpLen[NOTE_EVENT] += tmpVp[NOTE_EVENT]->length;
            noteData = (tmpVp[NOTE_EVENT]++)->data;
            if ( noteData )
				noteOn( i, noteData );
		}
    }

	curMusic->setPosFlag = OFF;

	if ( ++curMusic->curTick >= curMusic->totalTicks )
    {
		musicStatus = PAUSED;
		amdRewind( curMusic );

		if ( curMusic->autoPlayMode == ON )
			musicStatus = PLAYING;
		else
			amdStop();
	}

	return curMusic->curDelay;
}


/* static functions --------------------------------------------------- */


static void closeNext( AMD_Music *amPt )
{
	if ( !amPt ) return;

	if ( amPt->next ) closeNext( amPt->next );

	amdClose( amPt );
}

static void linkMusic( AMD_Music *amPt )
{
	AMD_Music *tmpPt;

	if ( !amPt ) return;

	/* link to music list */
    if ( !musicList )
    {
        musicList = amPt;
		amPt->prev = NULL;
    }
    else
	{
        tmpPt = musicList;
		while ( tmpPt->next )
            tmpPt = tmpPt->next;

        tmpPt->next = amPt;
		amPt->prev = tmpPt;
    }
    amPt->next = NULL;
}

static void unlinkMusic( AMD_Music *amPt )
{
	if ( !amPt ) return;

    /* unlink */
    if ( amPt->prev )
		amPt->prev->next = NULL;
    else if ( amPt == musicList )
        musicList = NULL;

    if ( curMusic == amPt )
		curMusic = NULL;
}
