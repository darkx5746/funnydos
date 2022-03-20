#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>

#include "vgapm.h"
#include "pmpcx.h"
#define ESC	 27

void vgaPmVScroll( int loc )
{
    int i;

    if ( loc < 0 || loc > 600 ) loc %= 600;

    vgaPmSetStartAddr( vRamStartAddr = loc*80 );

    for ( i = 0; i < 200; i++ )
        vRamAddrTable[0][i] = vRamStartAddr*80;
}

void main( void )
{
    int i, j, k=0;

    char *fname="vscroll1.pcx";

    vgaPmSetGraphMode(  );
    vgaPmSetDispPage( 0 );

    *( fname+7 ) = k+'1';
    k++;
    k %= 4;
    vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );
    vgaPmSetActivePage( 1 );
    *( fname+7 ) = k+'1';
    k++;
    k %= 4;
    vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );
    vgaPmSetActivePage( 2 );
    *( fname+7 ) = k+'1';
    k++;
    k %= 4;
	vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );
	vgaPmSetActivePage( 3 );
    *( fname+7 ) = k+'1';
	k++;
	k%=4;
	vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );

	while ( true )
	{

		for ( i = 0; i < 600 ; i++ )
		{
			vgaPmVScroll( i );

			if ( kbhit() )
				if ( getch() == ESC ) goto Exit;
		}

		for ( i = 600; i >= 0 ; i-- )
		{
			vgaPmVScroll( i );

			if ( kbhit() )
				if ( getch() == ESC ) goto Exit;
		}
	}

Exit:
	vgaPmEndGraphMode();
}

