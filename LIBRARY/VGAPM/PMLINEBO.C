#include <conio.h>
#include <dos.h>

#include "vgapm.h"

void vgaPmBoxFill( int x, int y, int xe, int ye, int color )
{
    int i, j, k, offs, xsize, xend;
	int planeNum = 0x0100;

    xsize = xe-x+1;

    xend = x + xsize + 3;

	for ( k = 0; k < 4; k++, planeNum <<= 1 )
	{
		outpw( 0x3c4, planeNum | 0x02 );

		for ( j = y; j <= ye; j++ )
		{
			for ( i = x & 0xffc; i <= xend; i += 4 )
			{
				offs = i + k - x;
				if ( offs < 0 ) continue;
				if ( offs >= xsize ) continue;

				*(VRAMADDRESS+vRamAddrTable[activePage][j]+( i>>2 )) = color;
			}
		}
	}
	outpw( 0x03c4, 0x0f02 );
}

void vgaPmHLine( int x, int y, int xe, int color )
{
	vgaPmBoxFill( x, y, xe, y, color );
}

void vgaPmVLine( int xs, int ys, int ye, int color )
{
	vgaPmBoxFill( xs, ys, xs, ye, color );
}

void vgaPmBox( int xs, int ys, int xe, int ye, int color )
{
	vgaPmHLine( xs, ys, xe, color );
	vgaPmHLine( xs, ye, xe, color );
	vgaPmVLine( xs, ys, ye, color );
	vgaPmVLine( xe, ys, ye, color );
}

