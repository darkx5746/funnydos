#include <dos.h>
#include <mem.h>
#include "adlib.h"

#define OPR_NUM			18
#define OPR_PARAM_NUM	14

#define PRM_KSL         0
#define PRM_MULTI       1
#define PRM_FEED_BACK   2       /* use for opr. 0 only */
#define PRM_ATTACK      3
#define PRM_SUSTAIN     4
#define PRM_EGTYPE  	5
#define PRM_DECAY       6
#define PRM_RELEASE     7
#define PRM_LEVEL       8
#define PRM_AM          9
#define PRM_VIB         10
#define PRM_KSR         11
#define PRM_FM          12      /* use for opr. 0 only */
#define PRM_WAVE_SELECT 13

#define PRM_AM_DEPTH    14
#define PRM_VIB_DEPTH   15
#define PRM_NOTE_SELECT 16
#define PRM_PERCUSSION  17

#define TOM_PITCH       24      /* best frequency, in range of 0 to 95 */
#define TOM_TO_SD       7       /* 7 half-tones between voice 7 & 8 */
#define SD_PITCH        (TOM_PITCH + TOM_TO_SD)
#define DEF_PITCH_RANGE 1

#define getOprParam(opr, prm)   (paramOpr[(opr)][(prm)])
#define sndInput()				inportb( adlibPortAddr )


/* Function prototypes ------------------------------------------------ */


int  adlibInstalled( void );
void initSound( void );
void setPercMode( int );

void noteOn( word voice, word pitch );
void noteOff( word voice );
void setVoiceInstr( word voice, byte *paramArray );
void setVoiceVolume( word voice, word volume );
void setVoicePitch( word voice, word pitchBend );

static void setGlobalData( byte amD, byte vibD, byte noteS );
static void setPitchRange( word pitchR );

static void initOperators( void );
static void setOperator( int opr, byte *param, byte waveSelect );

static void sndWaveSelectMode( int state );	/* 0x01 */
static void sndAVEKM( int opr );			/* 0x20 - 0x35 */
static void sndKslLevel( int opr);			/* 0x40 - 0x55 */
static void sndAttackDecay( int opr );		/* 0x60 - 0x75 */
static void sndSustainRelease( int opr );	/* 0x80 - 0x95 */
static void sndFreqNum( int voice );		/* 0xa0 - 0xb8 */
static void sndAmVibRhythm( void );			/* 0xbd */
static void sndFeedbackFm( int opr );		/* 0xc0 - 0xc8 */
static void sndWaveSelect( int opr );		/* 0xe0 - 0xf5 */

static void sndOutput( word addr, byte data);


/* Global variables --------------------------------------------------- */


static const int adlibPortAddr = 0x388;	/* ADLIB card io-port address */

static byte percBits;				/* control bits of percussive voices */
static const byte percMasks[] = {
	0x10, 0x08, 0x04, 0x02, 0x01
};

static byte isPercMode;         /* mode flag (percussion/melody) */
static byte voiceNum;			/* 9 or 11, depending on 'isPercMode'*/

static byte bxReg[11];			/* current value of 0xB0 - 0xB8 reg */
static byte pitchRange;			/* pitch variation, half-tone [+1,+12] */
static byte voiceKeyOn[11];     /* state of keyOn bit of each voice */
static byte voiceNote[11];		/* pitch of last note-on of each voice */
static word voicePitchBend[11];	/* current pitch bend of each voice */
static byte voiceVolume[11];	/* volume for each voice */

static byte amDepth;            /* chip global parameters .. */
static byte vibDepth;
static byte noteSel;

static byte paramOpr[OPR_NUM][OPR_PARAM_NUM]; /* all the parameters of oprators */

/* default data */
static byte pianoOp0[] = { 1,  1, 3, 15,  5, 0, 1,  3, 15, 0, 0, 0, 1, 0 };
static byte pianoOp1[] = { 0,  1, 1, 15,  7, 0, 2,  4,  0, 0, 0, 1, 0, 0 };
static byte bdOpr0[] =   { 0,  0, 0, 10,  4, 0, 8, 12, 11, 0, 0, 0, 1, 0 };
static byte bdOpr1[] =   { 0,  0, 0, 13,  4, 0, 6, 15,  0, 0, 0, 0, 1, 0 };
static byte sdOpr[] =    { 0, 12, 0, 15, 11, 0, 8,  5,  0, 0, 0, 0, 0, 0 };
static byte tomOpr[] =   { 0,  4, 0, 15, 11, 0, 7,  5,  0, 0, 0, 0, 0, 0 };
static byte cymbOpr[] =  { 0,  1, 0, 15, 11, 0, 5,  5,  0, 0, 0, 0, 0, 0 };
static byte hhOpr[] =    { 0,  1, 0, 15, 11, 0, 7,  5,  0, 0, 0, 0, 0, 0 };

/* melodic mode oprator index numbers for each voice. */
static byte oprMeloVoice[9][2] = {
	{0, 3},     /* voice 0 */
	{1, 4},     /* 1 */
	{2, 5},     /* 2 */
	{6, 9},     /* 3 */
	{7, 10},    /* 4 */
	{8, 11},    /* 5 */
	{12, 15},   /* 6 */
	{13, 16},   /* 7 */
	{14, 17}    /* 8 */
};

/* 
	melodic mode oprator index numbers for each voice.
	255 indicates that there is only one opr used by a voice.
*/
static byte oprPercVoice[11][2] = {
	{0, 3},     /* voice 0 */
	{1, 4},     /* 1 */
	{2, 5},     /* 2 */
	{6, 9},     /* 3 */
	{7, 10},    /* 4 */
	{8, 11},    /* 5 */
	{12, 15},   /* Bass Drum */
	{16, 255},  /* SD */
	{14, 255},  /* TOM */
	{17, 255},  /* TOP-CYM */
	{13, 255}   /* HH */
};

/* offset of each oprator. */
static const char offsetOpr[] = {
	 0,  1,  2,  3,  4,  5,
     8,  9, 10, 11, 12, 13,
    16, 17, 18, 19, 20, 21
};

/*  modulator (operator 0) or carrier (operator 1) */
static const char carrierOpr[] = {
    0, 0, 0,        /* 1 2 3 */
    1, 1, 1,        /* 4 5 6 */
    0, 0, 0,        /* 7 8 9 */
    1, 1, 1,        /* 10 11 12 */
    0, 0, 0,        /* 13 14 15 */
    1, 1, 1,        /* 16 17 18 */
};

/*
	This table gives the voice number associated with each opr.
    (melodic mode only)
	voice = fn (opr)
*/
static const char voiceMOpr[] = {
    0, 1, 2,
    0, 1, 2,
    3, 4, 5,
    3, 4, 5,
    6, 7, 8,
    6, 7, 8,
};

/*
    This table gives the voice number  (0-10) associated with each
	opr (percussive mode only),
	voice = fn (opr)
*/
static const char voicePOpr[] = {
    0, 1, 2,
    0, 1, 2,
    3, 4, 5,
    3, 4, 5,
    VOICE_BD, VOICE_HIHAT, VOICE_TOM,
    VOICE_BD, VOICE_SD, VOICE_CYMB
};

static word freqNumTable[12] = {
    343, 363, 385, 408, 432, 458,
    485, 514, 544, 577, 611, 647
};


/* Global functions --------------------------------------------------- */


int adlibInstalled( void )
{
	int i;

    sndOutput( 4, 0x60 );
    sndOutput( 4, 0x80 );

    if ( (sndInput() & 0xe0) != 0 )
		return FALSE;

    sndOutput( 2, 0xff );
    sndOutput( 4, 0x21 );

    for ( i = 0; i < 200; i++ )
        sndInput();

    if ( (sndInput() & 0xe0) != 0xc0 )
		return FALSE;

    return TRUE;
}


void initSound( void )
{
    int i;

	for ( i = 0x01; i <= 0xf5; i++ )
		sndOutput( i, 0 );      /* clear all registers */

	for ( i = 0; i < 11; i++ )
    {
		voicePitchBend[i] = MID_PITCH;
        voiceKeyOn[i] = 0;
        voiceNote[i] = 0;
    }

    for ( i = 0; i < 11; i++ )
        voiceVolume[i] = 0;

	setPercMode( OFF );
	setPitchRange( DEF_PITCH_RANGE );
    setGlobalData( 0, 0, 0 );
	sndWaveSelectMode( ON );
}


void setPercMode( int musicMode )
{
	isPercMode = musicMode;
	if ( isPercMode )
	{
		/* set the frequency for tom & snare drum: */
		voiceNote[VOICE_TOM] = TOM_PITCH;
		voicePitchBend[VOICE_TOM] = MID_PITCH;
		sndFreqNum(VOICE_TOM);
		voiceNote[VOICE_SD] = SD_PITCH;
		voicePitchBend[VOICE_SD] = MID_PITCH;
		sndFreqNum(VOICE_SD);
	}
	voiceNum = isPercMode ? 11 : 9;
	percBits = 0;

	initOperators();
	sndAmVibRhythm();
}


/*
	struct {
		byte opr0Prm [13];
		byte opr1Prm [13];
		byte opr0WaveSel;
		byte opr1WaveSel;
	} paramArray;
*/
void setVoiceInstr( word voice, byte *paramArray )
{
	word wave0, wave1;
	byte *prm1, *wavePtr;
	byte *oprs;

    if ( voice >= voiceNum ) return;

	wavePtr = paramArray + 2 * (OPR_PARAM_NUM - 1);
    wave0 = *wavePtr++;
    wave1 = *wavePtr;
	prm1 = paramArray + OPR_PARAM_NUM - 1;

	oprs = ( isPercMode ? oprPercVoice[voice] : oprMeloVoice[voice] );

    setOperator( oprs[0], paramArray, wave0 );

	if ( oprs[1] != 255 )
        setOperator( oprs[1], prm1, wave1 );
}


/*
	The resulting output level is (instrVolume * volume / 127).
	0 <= volume <= 127 (0x7f == MAX_VOLUME)
*/
void setVoiceVolume( word voice, word volume )
{
	byte *oprs;

    if ( voice >= voiceNum ) return;

	if (volume > MAX_VOLUME) 
		volume = MAX_VOLUME;

    voiceVolume[voice] = volume;

	oprs = ( isPercMode ? oprPercVoice[voice] : oprMeloVoice[voice] );

	sndKslLevel( oprs[0] );

	if ( oprs[1] != 255 )
		sndKslLevel( oprs[1] );
}


/*
	A value of 0 means - half_tone * pitchRange,
	0x2000 means no variation (exact pitch) and
	0x3fff means + half_tone * pitchRange.

	Does not affect the percussive voices, except for the bass drum.

	0 <= pitchBend <= 0x3fff, 0x2000 == exact tuning
*/
void setVoicePitch( word voice, word pitchBend )
{
    if ( (!isPercMode && voice < 9) || (voice <= VOICE_BD) )
    {
		/* melodic + bass-drum */
        if ( pitchBend > MAX_PITCH )
            pitchBend = MAX_PITCH;

		voicePitchBend[voice] = pitchBend;
		sndFreqNum( voice );
    }
}


/*
    Routine to start a note playing.
	0 <= pitch <= 127, 60 == MID_C  (the card can play between 12 and 107 )
*/
void noteOn( word voice, word pitch )
{
	if ( pitch <= 12 )
		pitch = 0;
	else if ( pitch > 107 )
		pitch = 95;
	else pitch -= 12;

	if ( (!isPercMode && voice < 9) || (isPercMode && voice < VOICE_BD) )
	{
		/* this is a melodic voice */
		voiceNote[voice] = pitch;
		voiceKeyOn[voice] = 0x20;
		sndFreqNum( voice );
	}
    else if ( isPercMode && (voice <= VOICE_HIHAT) )
    {
        /* this is a percussive voice */
        if ( voice == VOICE_BD )
        {
            voiceNote[VOICE_BD] = pitch;
			sndFreqNum( voice );
        }
        else if ( voice == VOICE_TOM )
        {
            /* for the last 4 percussions, only the TOM may change in frequency,
                modifying the three others: */
            if ( voiceNote[VOICE_TOM] != pitch )
            {
                voiceNote[VOICE_TOM] = pitch;
                voiceNote[VOICE_SD] = pitch + TOM_TO_SD;
				sndFreqNum( VOICE_TOM );
				sndFreqNum( VOICE_SD );
            }
		}
		percBits |= percMasks[voice - VOICE_BD];
		sndAmVibRhythm();
	}
}


void noteOff( word voice )
{
    if ( (!isPercMode && voice < 9) || (voice < VOICE_BD) )
    {
        voiceKeyOn[voice] = 0;
		bxReg[voice] &= ~0x20;
		sndOutput( 0xb0 + voice, bxReg[voice] );
    }
    else if ( isPercMode && (voice <= VOICE_HIHAT) )
    {
		percBits &= ~percMasks[voice - VOICE_BD];
		sndAmVibRhythm();
    }
}


/* Static functions --------------------------------------------------- */


/* 1 <= pitchRange <= 12 (in half-tones). */
static void setPitchRange( word pitchR )
{
	if (pitchR > 12) pitchR = 12;
	if (pitchR < 1)  pitchR = 1;

	pitchRange = pitchR;
}


/* Set the 3 global parameters AmDepth, VibDepth & NoteSel */
static void setGlobalData( byte amD, byte vibD, byte noteS )
{
    amDepth = amD;
    vibDepth = vibD;
	noteSel = noteS;

	sndAmVibRhythm();

	sndOutput( 0x08, (noteSel ? 0x40 : 0) );
}


/*
    In melodic mode, initialize all voices to electric-pianos.
	In percussive mode, initialize the first 6 voices to electric-pianos
    and the percussive voices to their default instrs.
*/
static void initOperators( void )
{
    int i;

	for ( i = 0; i < OPR_NUM; i++ )
		if ( carrierOpr[i] )
            setOperator( i, pianoOp1, 0 );
		else
            setOperator( i, pianoOp0, 0 );

    if ( isPercMode )
    {
        setOperator( 12, bdOpr0, 0 );
        setOperator( 15, bdOpr1, 0 );
        setOperator( 16, sdOpr, 0 );
        setOperator( 14, tomOpr, 0 );
        setOperator( 17, cymbOpr, 0 );
        setOperator( 13, hhOpr, 0 );
	}
}


static void setOperator( int opr, byte *param, byte waveSel )
{
	memcpy( &paramOpr[opr][0], param, OPR_PARAM_NUM - 1 );

	paramOpr[opr][OPR_PARAM_NUM-1] = waveSel &= 0x3;

	sndAVEKM( opr );
	sndKslLevel( opr );
	sndAttackDecay( opr );
	sndSustainRelease( opr );
	sndAmVibRhythm();
	sndFeedbackFm( opr );
	sndWaveSelect( opr );
}


/* Enable/disable (mode = ON/OFF) the wave-select parameters. */
static void sndWaveSelectMode( int mode )
{
	int i;

	/* initialize each opr */
	for ( i = 0; i < OPR_NUM; i++ )
		sndOutput( 0xe0 + offsetOpr[i], 0 );

	sndOutput( 1, (mode ? 0x20 : 0) );
}


/*  AM, VIB, EGTYPE, KSR, MULTI */
static void sndAVEKM( int opr )
{
    word t1;

    t1 = getOprParam( opr, PRM_AM ) ? 0x80 : 0;
    t1 |= getOprParam( opr, PRM_VIB ) ? 0x40 : 0;
    t1 |= getOprParam( opr, PRM_EGTYPE ) ? 0x20 : 0;
    t1 |= getOprParam( opr, PRM_KSR ) ? 0x10 : 0;
    t1 |= getOprParam( opr, PRM_MULTI ) & 0xf;
	sndOutput( 0x20 + (int)offsetOpr[opr], t1 );
}


static void sndKslLevel( int opr )
{
	word t1, vc, isSingleOpr;

	vc = ( isPercMode ? voicePOpr[opr] : voiceMOpr[opr] );

    t1 = 63 - (getOprParam( opr, PRM_LEVEL ) & 63);    /* amplitude */
	isSingleOpr = isPercMode && (vc > VOICE_BD);

    if ( (carrierOpr[opr] || !getOprParam( opr, PRM_FM ) || isSingleOpr) )
        /* Change the 0 - 127 volume change value to 0 - 63 for the chip.
		   (MAX_VOLUME+1)/2 is added to avoid round-off errors. */
        t1 = (t1 * voiceVolume[vc] + (MAX_VOLUME + 1) / 2 ) >> 7;

    t1 = 63 - t1;
    t1 |= getOprParam( opr, PRM_KSL ) << 6;
	sndOutput( 0x40 + (int)offsetOpr[opr], t1 );
}


/*  ATTACK, DECAY */
static void sndAttackDecay( int opr )
{
    word t1;

    t1 = getOprParam( opr, PRM_ATTACK ) << 4;
    t1 |= getOprParam( opr, PRM_DECAY ) & 0xf;
	sndOutput( 0x60 + (int)offsetOpr[opr], t1 );
}


/*  SUSTAIN, RELEASE */
static void sndSustainRelease( int opr )
{
    word t1;

    t1 = getOprParam( opr, PRM_SUSTAIN ) << 4;
    t1 |= getOprParam( opr, PRM_RELEASE ) & 0xf;
	sndOutput( 0x80 + (int)offsetOpr[opr], t1 );
}


static void sndFreqNum( int voice )
{
	word octave, pitch;
	word fnum, bend, tmpFnum;

	if( voice >= voiceNum ) return;

	pitch = voiceNote[voice];
	bend = voicePitchBend[voice];

	octave = pitch/12;
	fnum = freqNumTable[pitch%12];

	if ( bend > 0x2000 )
	{
		bend -= 0x2000;
		tmpFnum = freqNumTable[(pitch+pitchRange)%12];
		if ( tmpFnum < fnum ) tmpFnum <<= 1;
		tmpFnum -= fnum;
		fnum = fnum + (word)(((long)tmpFnum*bend)>>13);

		while ( fnum > 0x3ff )
		{
			fnum >>= 1;
			octave++;
		}
	}
	else if ( bend < 0x2000 )
	{
		bend = 0x2000 - bend;
		tmpFnum = freqNumTable[(pitch-pitchRange)%12];
		if ( tmpFnum > fnum ) tmpFnum >>= 1;
		tmpFnum = fnum - tmpFnum;
		fnum = fnum - (word)(((long)tmpFnum*bend)>>13);

		while ( fnum < freqNumTable[0] )
		{
			fnum <<= 1;
			octave--;
		}
	}

	bxReg[voice] = ((octave & 0x7)<<2) | ((fnum>>8) & 0x3);

	sndOutput( 0xa0 + voice,  fnum & 0xff );
	sndOutput( 0xb0 + voice, bxReg[voice]|voiceKeyOn[voice] );
}


/*  Set the values: AM Depth, VIB depth & Rhythm (melo/perc mode) */
static void sndAmVibRhythm( void )
{
    word t1;

	t1 = amDepth ? 0x80 : 0;
    t1 |= vibDepth ? 0x40 : 0;
    t1 |= isPercMode ? 0x20 : 0;
    t1 |= percBits;
    sndOutput( 0xbd, t1 );
}


/*
    FEED-BACK and FM (connection).
    Applicable only to operator 0 in melodic mode.
*/
static void sndFeedbackFm( int opr )
{
	word t1;

	if ( carrierOpr[opr] ) return;

    t1 = getOprParam( opr, PRM_FEED_BACK ) << 1;
    t1 |= getOprParam( opr, PRM_FM ) ? 0 : 1;
	sndOutput( 0xc0 + (int)voiceMOpr[opr], t1 );
}


static void sndWaveSelect( int opr )
{
    word wave;

    wave = getOprParam( opr, PRM_WAVE_SELECT ) & 0x03;

	sndOutput( 0xe0 + offsetOpr[opr], wave );
}


static void sndOutput( word addr, byte data )
{
	asm {
		mov	dx, adlibPortAddr
		mov	ax, addr
		out	dx, al

		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx

		inc	dx
		mov	al, data
		mov ah, 0
		out	dx, al

		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx

		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx

		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx

		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
		in	al, dx
	}
}
