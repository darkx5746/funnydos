#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "vgastd.h"
#include "stdpcx.h"
#include "palette.h"


char *getOneStr( char *dest, char *src )
{
	while ( isspace( *src ) ) src++;
	while ( *src != '\0' && !isspace( *src ) )
		*dest++ = *src++;

	*dest = '\0';

	return src;
}

void cutPcx( char *descFname, char *imageFname )
{
	byte *imgBuf;
	char strBuf[81], tmpStr[81], *tmpBuf;
	int size, sx, sy, ex, ey;
	FILE *descFp, *imgFp;

	if ( !(descFp = fopen( descFname, "rb" )) )
		errExit( "Can't open description file." );
	if ( !(imgFp = fopen( imageFname, "wb" )) )
		errExit( "Image file open error!" );

	while ( fgets( strBuf, 80, descFp ) != NULL )
	{
		tmpBuf = getOneStr( tmpStr, strBuf );
		if ( *tmpStr == '\0' ) break;
		sx = atoi( tmpStr );
		tmpBuf = getOneStr( tmpStr, tmpBuf );
		sy = atoi( tmpStr );
		tmpBuf = getOneStr( tmpStr, tmpBuf );
		ex = atoi( tmpStr );
		tmpBuf = getOneStr( tmpStr, tmpBuf );
		ey = atoi( tmpStr );

		if ( sx < 0 || sy < 0 || ex >= 320 || ey >= 200 || sx >= ex || sy >= ey )
			errExit(
				"Invalid range given.\n"
				"Range must be x : 0 - 319, y : 0 - 199." );

		size = (ex-sx+1) * (ey-sy+1) + 4;

		if ( !(imgBuf = (byte *) malloc( size )) )
			errExit( ERR_MEMORY );

		vgaStdGetImage( sx, sy, ex, ey, imgBuf );

		fwrite( imgBuf, size, 1, imgFp );
	}

	fclose( imgFp );
	fclose( descFp );
}

void cutPalette( char *fname )
{
    FILE *palFp;
    byte pcxPal[768];

    if ( !(palFp = fopen( fname, "wb" )) )
        errExit( "Palette file open error" );

    vgaGetAllPalette( pcxPal );

    fwrite( pcxPal, 768, 1, palFp );

    fclose( palFp );
}


void main( int argc, char *argv[] )
{
    if ( argc != 4 && argc != 5 )
		errExit(
			"PCX2IMG - converts PCX cuts to a bitmap image\n\n"
			"Usage: PCX2IMG <pcx_fname> <description_fname> <image_fname> [<pal_fname>]\n"
            "Description file format: <sx> <sy> <ex> <ey>\n" );

	vgaSetMode( VGA256COL );

	if ( vgaStdPcxCutDisp( argv[1], 0, 0, 0, 319, 199 ) == FAIL )
		errExit( NULL );

	cutPcx( argv[2], argv[3] );

    if ( argc == 5 )
        cutPalette( argv[4] );

    vgaSetMode( 3 );
}

