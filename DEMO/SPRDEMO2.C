#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <time.h>

#include "common.h"
#include "palette.h"
#include "vgaspr.h"
#define ESC	 27

void vgaSprBGRecover( int x, int y, int xe, int ye )
{
    vgaPmPageCopy( activePage, x, y,
        BACKGROUNDPAGE, x, y, xe-x+1, ye-y+1 );
}

void vgaSprBGWrite( int x, int y, int xe, int ye )
{
    vgaPmPageCopy( BACKGROUNDPAGE, x, y,
        activePage, x, y, xe-x+1, ye-y+1 );
}

void main( void )
{
    int i, j, page = 0;
	clock_t start, now, clktick=CLK_TCK;
	int spx[30], spy[30];

    byte *buf[2], ptb[256][3];

    if ( !(buf[0] = (byte *) malloc( 32000 )) ) ;
    if ( !(buf[1] = (byte *) malloc( 32000 )) ) ;

    vgaPmSetGraphMode();
    vgaSprInit();

    for ( i = 0; i < 64; i++ )
	{
        for ( j = 0; j < 3; j++ )
        {
            ptb[i][j] = i;
            ptb[i+64][j] = ptb[i+128][j] = ptb[i+192][j] = 0;
		}
        ptb[i+64][0] = ptb[i+128][1] = ptb[i+192][2] = i;
	}
    for ( i = 0; i < 320; i++ )
        for ( j = 0;j < 200; j++ )
            vgaPmPutPixel( i, j, (byte)(( i+j ) % 256 ));
    vgaSetNPalette( 0, 256, ptb[0] );

    for ( i = 1; i < 4; i++ )
	{
        vgaPmSetDispPage( i );
        vgaPmPageCopy( i, 0, 0, 0, 0, 0, 320, 200 );
	}
    vgaPmSetDispPage( 1 );

    vgaPmSetActivePage( SPRITEPAGE );

    vgaPmBoxFill( 0, 0, 30, 30, 191 );
    vgaPmBoxFill( 10, 10, 20, 20, invisibleColor );
    vgaPmGetImage( 0, 0, 30, 30, buf[0] );
    vgaSprSet( 2, buf[0] );
    spx[2] = 100; spy[2] = 85;

    vgaPmBoxFill( 0, 0, 30, 30, 127 );
    vgaPmBoxFill( 10, 10, 20, 20, invisibleColor );
    vgaSprSet( 1, VRAMSPRITE );
    vgaSprSetVRamBuf( 1, 0, 0, 30, 30 );
    spx[1] = 48; spy[1] = 85;

    vgaPmBoxFill( 100, 100, 105, 105, 63 );
    vgaPmGetImage( 100, 100, 105, 105, buf[0] );
    vgaPmBoxFill( 100, 100, 105, 105, 254 );
    vgaPmGetImage( 100, 100, 105, 105, buf[1] );
    vgaSprSet( 0, OUTBUFFER );
    vgaSprSetBuf( 0, buf[0] );
    spx[0] = 148; spy[0] = 85;

    vgaSprSet( 3, VRAMSPRITE );
    vgaSprSetVRamBuf( 3, 100, 100, 105, 105 );
    spx[3] = 200; spy[3] = 85;

    for( j = 0; j < 2; j++ )
	{
        vgaPmSetDispPage( j );
        for ( i = 0; i < 4; i++ )
            vgaSprPut( i, spx[i], spy[i] );
	}

    j = 0;
    start = clock();
    detailCrushCheckEnabled = true;

    while ( true )
	{
        vgaPmSetDispPage( page );
        page = 1 - page;
        vgaPmSetActivePage( page );

        now = clock();
        if ( (int)((now - start)*10/clktick) )
		{
            start = clock();
            vgaSprSetBuf( 0, buf[page] );
		}

        vgaSprPut( 0, spx[0], spy[0] );
        if ( spx[j] != vgaSprGetXloc( j ) || spy[j] != vgaSprGetYloc( j ) )

        if ( vgaSprPut( j, spx[j], spy[j] ) ) ;

        for ( i = 0; i < 4; i++ )
		{
            if ( vgaSprCheckCrush( i, j ) )
                vgaPmBoxFill( 300, i*20+20, 310, i*20+30,
                    vgaSprGetBuf( i ) ? *( vgaSprGetBuf( i )+5 ) : 127 );
			else
                vgaSprBGRecover( 300, i*20+20, 310, i*20+30 );
		}

        if ( !kbhit() ) continue;
        switch( getch() )
		{
            case ESC : break;
            case '4' : spx[j]--;   continue;
            case '8' : spy[j]--;   continue;
            case '6' : spx[j]++;   continue;
            case '2' : spy[j]++;   continue;
            case 'a' : j+=3; j%=4; continue;
            case 's' : j++; j%=4;  continue;
            default  : continue;
		}
		break;
	}

    vgaSprClose();

    if ( buf[0] )
    {
        free( buf[0] );
        buf[0] = 0;
    }
    if ( buf[1] )
    {
        free( buf[1] );
        buf[1] = 0;
    }

    vgaSetMode( 0x03 );
}

