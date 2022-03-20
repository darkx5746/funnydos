#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>

#include "vgapm.h"
#include "pmpcx.h"

#define ESC	 27

void vgaPmHScroll( int loc )
{
	vgaPmVRamCopy( 0, 0, 0, 2, loc, 0, 80-loc, 200 );
	vgaPmVRamCopy( 0, 80-loc, 0, 3, 0, 0, loc, 200 );
}

void main( void )
{
	int i, j, k=0;

    char *fname="hscroll1.pcx";

	vgaPmSetGraphMode();
	vgaPmSetDispPage( 0 );

	vgaPmSetActivePage( 2 );
    *( fname+7 ) = k+'1';
	k++;
	k %= 4;
	vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );
	vgaPmSetActivePage( 3 );
	k=1;
    *( fname+7 ) = k+'1';
	k++;
	k%=4;
	vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );

	while ( true )
	{
		for ( i = 0; i < 80; i++ )
		{
			vgaPmHScroll( i );
			if ( kbhit() )
				if ( getch() == ESC ) goto Exit;
		}

		for ( i = 79; i >= 0; i-- )
		{
			vgaPmHScroll( i );
			if ( kbhit() )
				if ( getch() == ESC ) goto Exit;
		}
	}

Exit:
	vgaPmEndGraphMode();
}

