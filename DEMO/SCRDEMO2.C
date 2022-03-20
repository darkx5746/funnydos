#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>

#include "vgapm.h"
#include "pmpcx.h"
#define ESC	 27

int vgaPmHLeftScroll( void )
{
    register i;

    for ( i = 0; i < 200; i++ )
        vRamAddrTable[0][i]++;

    vRamStartAddr++;

    if ( vRamStartAddr > 79 )
	{
        vgaPmSetWriteMode( 1 );

        for ( i = 79; i < 16000; i += 80 )
            *(VRAMADDRESS+80+i) = *(VRAMADDRESS+16000*3+i+16);

        vRamStartAddr = 0;

        vgaPmSetStartAddr( vRamStartAddr );

		asm {
            push ds;
            push es;

            mov ax, VRAMSEGMENT;
            mov ds, ax;
            mov es, ax;
            mov si, 80;
            mov di, 0;

            mov cx, 16000;
            rep movsb;

            pop es;
            pop ds;
		}

		vgaPmSetWriteMode( 0 );

		for ( i = 0; i < 400; i++ )
			vRamAddrTable[0][i] = i*80;
	}
	else
	{
		vgaPmSetStartAddr( vRamStartAddr );
		vgaPmVRamCopy( 0, 79, 0, 3, vRamStartAddr-1, 0, 1, 200 );
	}
	return vRamStartAddr;
}

void main( void )
{
	int i, j, k=0;

    char *fname="hscroll1.pcx";

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
	k=1;
    *( fname+7 ) = k+'1';
	k++;
	k%=4;
	vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );

	for ( i = 0; i < 30000; i++ )
	{
		if ( !vgaPmHLeftScroll() )
		{
            *(fname + 7) = k + '1';
			k++;
			k %= 4;
			vgaPmPcxCutDisp( fname, 0, 0, 0, 320, 200 );
		}

		if ( kbhit() )
			if ( getch() == ESC ) break;
	}

	vgaPmEndGraphMode();
}

