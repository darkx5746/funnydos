#include <mem.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include "vgastd.h"
#include "palette.h"

typedef struct pcxhdr {
	char Manufacturer;
	char Version;
	char Encoding;
	char Bitsperpixel;
	int  x1,y1,x2,y2;
	int  HRes;
	int  VRes;
	unsigned char Colormap[16][3];
	char Reserved;
	char NPlanes;
	unsigned BytesperLine;
	int  PaletteInfo;
	char Filler[58];
} PCXHDR;

byte pcxPalette[256];

static int optimizePalette( int skipColor, FILE *fp)
{
	byte 	temp[256];
	byte 	*pal;
	byte 	*pcxPal;
	byte 	r, g, b;
	int  	diff1, diff2,
			i, j, color;

    randomize();

	pal = (byte *) malloc( 768 );
	if ( !pal )
	{
        setError( "no memory in optimizePalette (PCX.C)" );
		return FAIL;
	}

	pcxPal = (byte *) malloc( 768 );

	if ( !pcxPal )
	{
		setError("no memory in optimizePalette (PCX.C)");
		free( pal );
		return FAIL;
	}

	fseek( fp, -768L, SEEK_END );
	fread( pcxPal, 768, 1, fp );

	if ( skipColor == 0 )
	{
		for ( i = 0; i < 768; i++ )
			*(pcxPal+i) >>= 2;
		vgaSetAllPalette( pcxPal );
		for ( i = 0; i < 256; i++ )
			pcxPalette[i] = i;
		return SUCCESS;
	}

    for ( i = 0; i < 256 - skipColor; i++ )
	{
		color = random( 256 );
		temp[i] = color;

        for ( j = 0; j < i; j++ )
			if ( temp[j] == temp[i] ) --i;

		r = *( pcxPal + color * 3 );
		g = *( pcxPal + color * 3 + 1 );
		b = *( pcxPal + color * 3 + 2 );

        vgaSetOnePalette( skipColor + i, r>>2, g>>2, b>>2 );
	}

	vgaGetAllPalette( pal );

    for ( i = 0; i < 256; i++ )
	{
		r = *( pcxPal + i * 3 );
		g = *( pcxPal + i * 3 + 1 );
		b = *( pcxPal + i * 3 + 2 );

		r >>= 2;
		g >>= 2;
		b >>= 2;

		diff1 = 32767; /* the largest values of INT type */

        for ( j = skipColor + 1; j < 256; j++ )
		{
			diff2 = abs( r - *( pal + j * 3 ) ) +
					abs( g - *( pal + j * 3 + 1 ) ) +
					abs( b - *( pal + j * 3 + 2 ) );

			if ( diff1 >= diff2 )
			{
				diff1 = diff2;
				pcxPalette[i] = j;
			}
		}
	}

	free( pal );
	free( pcxPal );

	return SUCCESS;
}

static int pcxLineRead( byte *buff, FILE *fpt, int bytes)
{
	int c, i;
	int n=0;

	do
	{
		c = ( fgetc( fpt ) & 0xff );
		if ( ( c & 0xc0 ) == 0xc0 )
		{
			i = c & 0x3f;
			c = fgetc( fpt );
			while ( i-- ) buff[n++] = pcxPalette[c];
		}
		else
		{
			buff[n++] = pcxPalette[c];
		}
	} while ( n < bytes );

	return n;
}

static int pcxZoomX( byte *buff, int putX1, int putX2, int pcxX1, int pcxX2 )
{
	register i;
	unsigned n;
	int 	 putSize;
	int 	 pcxSize;
	byte 	 *dest;

	putSize = putX2 - putX1;
	pcxSize = pcxX2 - pcxX1;

	if ( putSize > pcxSize ) return pcxSize;

	n = ( pcxSize * 10 ) / putSize;

	dest = (byte *) malloc( putSize );
	if ( !dest )
	{
		setError("no memory in pcxZoomX (PCX.C)");
		return FAIL;
	}

    for( i = 0; i <= putSize; i++ )
		*( dest + i ) = *( buff + ( n * i ) / 10 );

	memcpy( buff, dest , putSize );

	free( dest );
	return putSize;
}

int vgaStdPcxCutDisp( char *fname, int skipColor,
				int putX1, int putY1, int putX2, int putY2 )
{
	FILE    *fp;
	PCXHDR  *p;
	byte    *temp;
	char	str[80];
	int      i, j, n, bytes, wid;

	if (  !( p = (PCXHDR *)malloc( sizeof( PCXHDR ) ) )  )
	{
		setError("no memory in pcxCutDisp (PCX.C)");
		return FAIL;
	}

	fp = fopen( fname, "rb" );
	if (  !fp  )
	{
		strcpy( str, fname );
		strcat( str, " doesn't exits!");
		setError( str );
		free( p );
		return FAIL;
	}

	fread( ( void * )p,128,1,fp );
	bytes = p->BytesperLine;

	temp = (byte *) malloc( bytes );
	if ( !temp )
	{
		setError("no memory in pcxCutDisp (PCX.C)");
		return FAIL;
	}

	wid = p->x2 - p->x1 + 1;

	optimizePalette( skipColor, fp );

	fseek( fp, 128, 0 );

	for ( i = putY1; i < putY1 + p->y2 - p->y1 + 1; i++ )
	{
		if ( i >= VGA256YSIZE || i >= ( putY2 + 1 ) ) break;

		n = pcxLineRead( temp, fp, bytes );

		if ( n > wid ) n = wid;

		for( j = 0; j < n; j++ )
		{
            if ( putX1 + j > putX2 ) break;

			vgaStdPutPixel( putX1 + j, i, *(temp + j) );
		}
	}

	fclose( fp );

	free( temp );
	free( p );

	return SUCCESS;
}

int vgaStdPcxZoomDisp( char *fname, int skipColor,
				int putX1, int putY1, int putX2, int putY2 )
{
	FILE    *fp;
	PCXHDR  *p;
	byte *temp;
	char str[80];

	int i, j, n, y, bytes, wid;
	unsigned y_ratio=0, count=0;

	y = putY1;

	if (  !( p = (PCXHDR *) malloc( sizeof( PCXHDR ) ) )  )
	{
		setError("no memory in pcxZoomDisp (PCX.C)");
		return FAIL;
	}

	fp = fopen( fname,"rb" );
	if ( !fp )
	{
		strcpy( str, fname );
		strcat( str, " doesn't exits!");
		setError( str );
		free( p );
		return FAIL;
	}

	fread(  (void *)p,128,1,fp );

	bytes = p->BytesperLine;

	temp = (byte *) malloc( bytes );
	if ( !temp )
	{
		setError("no memory in pcxZoomDisp (PCX.C)");
		return FAIL;
	}

	wid = p->x2 - p->x1 + 1;

	optimizePalette( skipColor, fp );

	fseek( fp, 128, 0 );
	y_ratio = ( ( p->y2 - p->y1 ) * 10 ) / ( putY2 - putY1 );

    for ( i = y; i < y + p->y2 - p->y1; i++ )
	{
		if ( count+y >= VGA256YSIZE || count+y >= ( putY2 + 1 ) ) break;

		n = pcxLineRead( temp, fp, bytes );

		if ( ( y_ratio * count  ) / 10 == ( i - y ) ||
			( p->y2 - p->y1 ) < ( putY2 - putY1 ) )
		{
			n = pcxZoomX( temp, putX1, putX2, p->x1, p->x2 );

			if ( n > wid ) n = wid;

            for( j = 0; j < n; j++ )
				vgaStdPutPixel( putX1 + j, count+y, *( temp + j ) );

			++count;
		}
	}
	fclose( fp );

	free( temp );
	free( p );

	return SUCCESS;
}
