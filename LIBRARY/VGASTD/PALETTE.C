#include <conio.h>
#include <dos.h>
//#include <mem.h>
#include <stdlib.h>
#include <string.h>
#include "vgastd.h"
#pragma inline

void vgaSetOnePalette( byte color, byte r, byte g, byte b )
{
	outp( 0x3c8, color );
	outp( 0x03c9, r );
	outp( 0x03c9, g );
	outp( 0x03c9, b );

}

void vgaGetOnePalette( byte color, byte *r, byte *g, byte *b )
{
	outp( 0x3c8, color );
	*r = inp( 0x03c9 );
	*g = inp( 0x03c9 );
	*b = inp( 0x03c9 );
}

int vgaSetAllPalette( byte *palette )
{
	if( !palette ) return FAIL;

	asm {
		mov ah, 0x10;
		mov al, 0x12;
		mov bx, 0;
		mov cx, 0x100;
		les dx, palette;
		int	0x10;
	}

	return SUCCESS;
}

int vgaGetAllPalette( byte *palette )
{
	if( !palette ) return FAIL;

	asm {
		mov ah, 0x10;
		mov al, 0x17;
		mov bx, 0;
		mov cx, 0x100;
		les dx, palette;
		int	0x10;
	}

	return SUCCESS;
}

int vgaSetNPalette( int color, int num, byte *buffer )
{
	register i;
	byte *buf = buffer;

	if( !buffer ) return FAIL;

	outp( 0x3c8, color );

	for ( i = 0 ; i < num ; i++ )
	{
		outp( 0x03c9, *buf++ );
		outp( 0x03c9, *buf++ );
		outp( 0x03c9, *buf++ );
	}

	return SUCCESS;
}