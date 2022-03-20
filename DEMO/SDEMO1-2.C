#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>

#include "vgaspr.h"

void main( void )
{
	int i, x, y, xstep, ystep;

	vgaPmSetGraphMode();
	vgaSprInit();

	vgaPmSetDispPage( PAGE0 );
	for ( i = 0; i < 32000; i++ )
		vgaPmPutPixel( rand() % 320, rand() % 200, rand() % 256 );

	vgaPmPageCopy( BACKGROUNDPAGE, 0, 0, PAGE0, 0, 0, 320, 200 );
	vgaPmPageCopy( PAGE1, 0, 0, PAGE0, 0, 0, 320, 200 );

	vgaPmSetActivePage( SPRITEPAGE );
	vgaPmClearScr( invisibleColor );

	for ( i = 0; i < 3; i++ )
		vgaPmBox( i, i, 50-i, 50-i, 15 );
	vgaSprSet( 0, VRAMSPRITE );
    vgaSprSetVRamBuf( 0, 0, 0, 50, 50 );

	vgaPmSetDispPage( PAGE0 );

	x = y = 100;
	xstep = ystep = 4;

	while ( !kbhit() )
	{
		x += xstep;
		y += ystep;

		if ( x <= 0 ) {
			x -= xstep;
			xstep = 4;
			x += xstep;
		}
		else
		if ( x >= 320 ) {
			x -= xstep;
			xstep =- 4;
			x += xstep;
		}

		if ( y<=0 ) {
			y -= ystep;
			ystep = 4;
			y += ystep;
		}
		else
		if ( y >= 200 ) {
			y -= ystep;
			ystep =- 4;
			y += ystep;
		}
		vgaPmSetActivePage( 1 );

		vgaSprPut( 0, x-vgaSprGetXsize( 0 )/2, y-vgaSprGetYsize( 0 )/2 );

		vgaPmSetDispPage( 1 );
		vgaPmFullPageCopy( 0, 1 );
		vgaPmSetDispPage( 0 );
	}

	vgaSprClose();
    vgaPmEndGraphMode();
}

