#include <dos.h>
#include <alloc.h>
#include <mem.h>

#include "palette.h"

void vgaDecPalette( byte *palette, int skipColor, int count )
{
	register i;

	skipColor++;

	for( i = skipColor * 3 ; i < 768  ; i++ )
	{
		if ( *( palette+i ) >= count )
			 *( palette+i ) -= count;
		else
			 *( palette+i ) = 0;
	}
}

void vgaIncPalette( byte *palette, int skipColor, int count )
{
	register i;

	skipColor++;

	for( i = skipColor * 3 ; i < 768 ; i++ )
	{
        if ( *( palette+i ) + count <= 63 )
			 *( palette+i ) += count;
		else
			 *( palette+i ) = 63;
	}
}

int vgaFadeInPalette( byte *palette, int skipColor )
{
	register i;
	byte *temp;

	if( !palette ) return FAIL;

	temp = ( byte * )malloc( 768 );
	memcpy( temp, palette, 768 );

	for( i = 63  ; i >= 0 ; i-- )
	{
		delay( 2 );
		vgaDecPalette( temp, skipColor, i );
		vgaSetAllPalette( temp );
        memcpy( temp, palette, 768 );
	}
    free( temp );

	return SUCCESS;
}

int vgaFadeOutPalette( byte *palette, int skipColor )
{
	register i;
    byte *temp;

    if( !palette ) return FAIL;

    temp = ( byte * )malloc( 768 );
    memcpy( temp, palette, 768 );

    for( i = 0 ; i <= 63 ; i++ )
	{
		delay( 2 );
		vgaDecPalette( temp, skipColor, 1 );
		vgaSetAllPalette( temp );
	}
	free( temp );

	return SUCCESS;
}

int vgaLightInPalette( byte *palette, int skipColor )
{
	register i;
    byte *temp;

    if( !palette ) return FAIL;

	temp = ( byte * ) malloc( 768 );
	memcpy( temp, palette, 768 );

	for( i = 63  ; i >= 0 ; i-- )
	{
		delay( 2 );
		vgaIncPalette( temp, skipColor, i );
		vgaSetAllPalette( temp );
		memcpy( temp, palette, 768 );
	}
	free( temp );

	return SUCCESS;
}

int vgaLightOutPalette( byte *palette, int skipColor )
{
	register i;
    byte *temp;

    if( !palette ) return FAIL;

    temp = ( byte * )malloc( 768 );
    memcpy( temp, palette, 768 );

	for( i = 0 ; i <= 63 ; i++ )
	{
		delay( 2 );
		vgaIncPalette( temp, skipColor, 1 );
        vgaSetAllPalette( temp );
	}
    free( temp );

	return SUCCESS;
}

int  vgaRotatePalette( byte *palette, int begin, int end, int count )
{
	byte *temp1, *temp2;

	if( !palette ) return FAIL;

	temp1  = (byte *)malloc( 768 );
	temp2  = (byte *)malloc( count * 3 );
	memcpy( temp1, palette, 768 );

	memcpy( temp2, temp1 + begin * 3, count * 3 );
	memmove( temp1 + begin * 3, temp1 + ( begin + count ) * 3, ( end - begin - count ) * 3 );
	memcpy( temp1 + ( end - count ) * 3, temp2, count * 3 );

	vgaSetAllPalette( temp1 );

	free( temp1 );
	free( temp2 );

	return SUCCESS;
}
