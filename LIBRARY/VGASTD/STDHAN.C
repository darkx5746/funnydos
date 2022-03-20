#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "stdhan.h"

extern const byte far engfont[256][16];
extern const byte far korfont[488*32];

static byte (*engfontPtr)[16] =(byte (*)[16]) engfont;
//static byte (*korfontPtr)[32] =(byte (*)[32]) korfont;

static byte (*hFont1Ptr)[20][32] =(byte(*)[20][32]) korfont;
static byte (*hFont2Ptr)[22][32] =(byte(*)[22][32]) (korfont+5120);
static byte (*hFont3Ptr)[28][32] =(byte(*)[28][32]) (korfont+5120+2816);

static const byte table[3][32] =
{
	{  0, 0, 1, 2, 3, 4, 5, 6,	   7, 8, 9, 10, 11, 12, 13, 14,
	  15, 16, 17, 18, 19, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0
	},
	{  0, 0, 0, 1, 2, 3, 4, 5,         0, 0, 6, 7, 8, 9, 10, 11,
	   0, 0, 12, 13, 14, 15, 16, 17,   0, 0, 18, 19, 20, 21, 0, 0
	},
	{  0, 0, 1, 2, 3, 4, 5, 6,         7, 8, 9, 10, 11, 12, 13, 14,
	  15, 16, 0, 17, 18, 19, 20, 21,  22, 23, 24, 25, 26, 27, 0, 0
	}
};

static const byte FstTABLE[2][20] =
{
	{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,	 1, 1, 1, 1, 1, 1, 0, 1, 1, 1 },
	{ 0, 2, 3, 3, 3, 3, 3, 3, 3, 3,	 3, 3, 3, 3, 3, 3, 2, 3, 3, 3 }
};

static const byte MidTABLE[3][22] =
{
	{ 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 },
	{ 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 }
};

//static byte *hFont1, *hFont2, *hFont3, *eFont;
static int currentX, currentY, foreColor;
static void orImg( byte  *src, byte  *dest, int bytes );
#if 0
static void mkImg( byte  *src, byte  *dest)
{
  register unsigned int i;
  for(i=0; i < 32; i++) dest[i] |= src[i];
}
#endif

static void vgaStdPutEng(int x,int y,int color,byte *buffer);
static void vgaStdPutHan(int x,int y,int color,byte *buffer);

void vgaStdSetFont( char *hanFontFile , char *engFontFile )
{
#if 0
	char str[80];
	FILE *font;

	if ( !( font = fopen( hanFontFile, "rb" ) ) )
	{
		strcpy(str,hanFontFile);
		strcat(str," doesn't exist!");
		errExit(str);
	}

	fread( hFont1, 5120, 1, font );
	fread( hFont2, 2816, 1, font );
	fread( hFont3, 3584, 1, font );
	fclose( font );

	if ( !( font = fopen( engFontFile, "rb" ) ) )
	{
		strcpy( str,hanFontFile );
		strcat( str," doesn't exist!" );
		errExit( str );
	}

	fread( eFont, 4096, 1, font );
	fclose( font );
#endif
}

void vgaStdOpenHan( char *hanFont, char *engFont )
{
#if 0
	if ( !( hFont1 = ( byte * ) malloc( 5120 ) ) ||
		 !( hFont2 = ( byte * ) malloc( 2816 ) ) ||
		 !( hFont3 = ( byte * ) malloc( 3584 ) ) ||
		 !( eFont  = ( byte * ) malloc( 4096 ) ) )
		{
			errExit( ERR_MEMORY );
		}

	vgaStdSetFont( hanFont, engFont );
#endif

	currentX = 0;
	currentY = 0;
	foreColor = 15;
}

void vgaStdPutsxy( int x, int y, char *string )
{

	byte imageBuffer[32];

	byte data1, data2, first;
	byte mid, last;
	int b1, b2, b3;

	currentX = x;
	currentY = y;

	while ( TRUE )
	{

		data1 = *( string++ );
		if ( data1 == ( byte ) NULL ) return;
		if ( data1 == '\t' )
		{
			currentX += ( 8 - ( currentX % 8 ) ) * FONTXSIZE;
			continue;
		}

		if ( data1 == '\n' )
		{
			currentX=0;
			currentY += FONTYSIZE;
			continue;
		}

		if ( data1 > 127 )
		{
			if ( currentX > VGA256XSIZE - 2 * FONTXSIZE )
			{
			currentX = 0;
			currentY += FONTYSIZE;
			}

			data2 = *( string++ );

			first = ( data1 & 124 ) >> 2;
			mid   = ( data1 & 3 ) * 8 + ( data2 >> 5 );
			last  = ( data2 & 31 );

			first = table[0][first];
			mid   = table[1][mid];
			last  = table[2][last];

			   b3 = MidTABLE[0][mid];

			if ( !last )
			{
				b2 = FstTABLE[0][first];
				b1 = MidTABLE[1][mid];
			} else
			{
				b2 = FstTABLE[1][first];
				b1 = MidTABLE[2][mid];
			}

			memset( imageBuffer, 0, 32 );
#if 0
			if ( first ) orImg( &hFont1[( b1*20+first )<<5], imageBuffer, 32 );
			if ( mid )   orImg( &hFont2[( b2*22+mid )<<5], imageBuffer, 32 );
			if ( last )  orImg( &hFont3[( b3*28+last )<<5], imageBuffer, 32 );
#else
			if ( first ) orImg( (byte*)hFont1Ptr[b1][first], imageBuffer, 32);
			if ( mid )   orImg( (byte*)hFont2Ptr[b2][mid],   imageBuffer, 32);
			if ( last )  orImg( (byte*)hFont3Ptr[b3][last],  imageBuffer, 32);
#endif
			vgaStdPutHan( currentX, currentY, foreColor, imageBuffer );
			currentX += 2 * FONTXSIZE;
		 } else
		 {

			if ( currentX > VGA256XSIZE - FONTXSIZE )
			{
				currentX = 0;
				currentY += FONTYSIZE;
			}
//			memset( imageBuffer, 0, 16 );
//			memcpy( imageBuffer, &eFont[data1<<4], 16 );
//			vgaStdPutEng( currentX, currentY, foreColor, imageBuffer );
			vgaStdPutEng( currentX, currentY, foreColor, engfontPtr[data1]);
			currentX += FONTXSIZE;
		 }
	}
}

int vgaStdGetCenterOfStr( char *string )
{
	return ( ( VGA256XSIZE - strlen( string ) * FONTXSIZE ) / 2 );
}

void vgaStdPutsxyC( int y, char *string )
{
    int center = vgaStdGetCenterOfStr( string );

	vgaStdPutsxy( center, y, string );
}

void vgaStdPuts( char *string )
{
	vgaStdPutsxy( currentX, currentY, string );
}

void vgaStdSetPos( int x, int y )
{
	currentX = x;
	currentY = y;
}

void vgaStdGetPos( int *x, int *y )
{
	*x = currentX;
	*y = currentY;
}

void vgaStdSetForeColor( int color )
{
	foreColor = color;
}

int  vgaStdGetForeColor( void )
{
	return foreColor;
}

void vgaStdCloseHan( void )
{
#if 0
	free( hFont1 );
	free( hFont2 );
	free( hFont3 );
	free( eFont );
#endif
}
#pragma inline

static
void orImg( byte  *src, byte  *dest, int bytes )
{
	asm {
		push ds;
		push es;
		les bx, dest;
		lds si, src;
		mov cx, bytes;
		shr cx, 1;
	}
orimg:
	asm {
		mov ax, ds:[si];
		or es:[bx], ax;
		inc bx;
		inc bx;
		inc si;
		inc si;
		loop orimg;
		pop es;
		pop ds;
	}
}

static
void vgaStdPutEng( int x, int y, int fcolor, byte  *buff )
{
	asm {
		push ds;
		push es;
		mov ax, VRAMSEGMENT;
		mov es, ax;

		mov ax, y;
		mov bx, VGA256XSIZE;
		mul bx;
		add ax, x;
		mov di, ax;
		lds si, buff;
		mov dx, fcolor;
		mov bh, 0x80;

		mov cx, 16;
		push di;
	}
Estart:
    asm {
		mov ah, ds:[si];
	}
Einloop:
    asm {
		test ah, bh;
		jz  Eskip;
		mov es:[di], dl;
	}
Eskip:
	asm {
		inc di;
		shr bh, 1;
		jc  Eoutloop;
		jmp Einloop;
	}
Eoutloop:
    asm {
		pop di;
		add di, VGA256XSIZE;
		push di;
		mov bh, 0x80;
		inc si;
		loop Estart;
	}
Eend:
    asm {
		pop di;
		pop es;
		pop ds;
	}
}

static
void vgaStdPutHan(int x, int y, int fcolor, byte *buff)
{
    asm {
		push ds;
		push es;
		mov ax, VRAMSEGMENT;
		mov es, ax;

		mov ax, y;
		mov bx, VGA256XSIZE;
		mul bx;
		add ax, x;
		mov di, ax;
		lds si, buff;
		mov dx, fcolor;
		mov bx, 0x8000;

		mov cx, 16;
		push di;
	}
Hstart:
    asm {
		mov ax, ds:[si];
		xchg al, ah;
	}
Hinloop:
    asm {
		test ax, bx;
		jz  Hskip;
		mov es:[di], dl;
	}
Hskip:
    asm {
		inc di;
		shr bx, 1;
		jc  Houtloop;
		jmp Hinloop;
	}
Houtloop:
    asm {
		pop di;
		add di, VGA256XSIZE;
		push di;
		mov bx, 0x8000;
		inc si;
		inc si;
		loop Hstart;
	}
Hend:
    asm {
		pop di;
		pop es;
		pop ds;
	}
}