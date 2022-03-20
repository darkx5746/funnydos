#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#include "vgapm.h"

byte invisibleColor = 0x80;

word vgaPmGetImage( int x, int y, int xe, int ye, byte *buffer )
{
    int i, j, k, offs, xsize, ysize, xend, yend;
	byte *buf;

    if ( !buffer ) return FAIL;

	xsize = xe - x + 1;
	ysize = ye - y + 1;

    buffer[0] = (byte)xsize;
    buffer[1] = (byte)(xsize>>8);
    buffer[2] = (byte)ysize;
    buffer[3] = (byte)(ysize>>8);

    if ( x>319 || y>199 || xe<0 || ye<0 ) return false;

    xend = min( x+xsize+3, 319 );
	yend = y + ysize - 1;

    if ( xend<0 || yend<0 ) return false;

    for ( k = 0; k < 4; k++ )
	{
		outp( 0x3ce, 0x04 );
		outp( 0x3cf, k );

		buf=buffer+4;

        for ( j = y; j <= yend; j++, buf += xsize )
		{
			if ( j<0 || j>199 ) continue;
			for ( i = (max( x, 0 )) & 0xffc; i <= xend; i += 4 )
			{
				offs = i + k - x;
                if ( offs < 0 ) continue;
                if ( offs >= xsize ) continue;
                *( buf+offs ) =
                    *( VRAMADDRESS+vRamAddrTable[activePage][j]+( i>>2 ) );
			}
		}
	}
    outpw( 0x3ce, 0x0004 );
    return (word)(buf - buffer);
}

word vgaPmPutImage( int x, int y, byte *buffer )
{
    int i, j, k, offs, xsize, ysize, xend, yend;
	byte *buf;
	int planeNum = 0x0100;

    if ( !buffer ) return FAIL;

    if ( x>319 || y>199 ) return false;

    xsize = buffer[0];
    xsize|= (int)(buffer[1]<<8);
    ysize = buffer[2];
    ysize|= (int)(buffer[3]<<8);

    xend = min( x+xsize+3, 319 );
	yend = y + ysize - 1;

    if ( xend<0 || yend<0 ) return false;

    for ( k = 0; k < 4; k++, planeNum <<= 1 )
	{
        outpw( 0x3c4, planeNum | 0x02 );

		buf=buffer+4;

        for ( j = y; j <= yend; j++, buf += xsize )
		{
            if ( j<0 || j>199 ) continue;
            for ( i = (max( x, 0 )) & 0xffc; i <= xend; i += 4 )
			{
				offs = i + k - x;
                if ( offs < 0 ) continue;
                if ( offs >= xsize ) continue;

                *( VRAMADDRESS+vRamAddrTable[activePage][j]+( i>>2 ) ) =
                *( buf+offs ) ;
			}
		}
	}
    outpw( 0x03c4, 0x0f02 );
    return (word)(buf - buffer);
}

word vgaPmPutImageInviCol( int x, int y, byte *buffer )
{
    int i, j, k, offs, xsize, ysize, xend, yend;
	byte *buf;
	int planeNum = 0x0100;

    if ( !buffer ) return FAIL;

    if ( x>319 || y>199 ) return false;

    xsize = buffer[0];
    xsize |= (int)(buffer[1]<<8);
    ysize = buffer[2];
    ysize |= (int)(buffer[3]<<8);

    xend = min( x+xsize+3, 319 );
	yend = y + ysize - 1;

    if ( xend<0 || yend<0 ) return false;

    for ( k = 0; k < 4; k++, planeNum <<= 1 )
	{
        outpw( 0x3c4, planeNum | 0x02 );

        buf = buffer + 4;

        for ( j = y; j <= yend; j++, buf += xsize )
		{
            if ( j<0 || j>199 ) continue;
            for ( i = (max( x, 0 )) & 0xffc; i <= xend; i += 4 )
			{
				offs = i + k - x;
                if ( offs < 0 ) continue;
                if ( offs >= xsize ) continue;
                if ( *(buf+offs) != invisibleColor )
                    *(VRAMADDRESS+vRamAddrTable[activePage][j]+( i>>2 )) =
                    *(buf+offs) ;
			}
		}
	}
    outpw( 0x03c4, 0x0f02 );
    return (word)(buf - buffer);
}

