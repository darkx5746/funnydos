#include "aeffect.h"
#include "adlib.h"

word setAdlibEffectVolume( word );
word setAdlibEffectVoice( word );
int  adlibEffectOn( int pitch, InstrDef *instr );
void adlibEffectOff( void );


static word effectVolume = MAX_VOLUME,
			effectVoice  = 0;


word setAdlibEffectVolume( word volume )
{
	if ( (effectVolume = volume * MAX_VOLUME / 100) > 100 )
		effectVolume = 100;
	setVoiceVolume( effectVoice, effectVolume );

	return effectVolume;
}

word setAdlibEffectVoice( word voice )
{
	effectVoice = ( voice > 11 || voice < 1 ? effectVoice : voice-1 );

	return effectVoice;
}

int adlibEffectOn( int pitch, InstrDef *instr )
{
	if ( !instr ) return FAIL;

	noteOff( effectVoice );
	setVoiceInstr( effectVoice, (byte *)instr );
	setVoiceVolume( effectVoice, effectVolume );
	noteOn( effectVoice, pitch );

	return SUCCESS;
}

void adlibEffectOff( void )
{
	noteOff( effectVoice );
}
