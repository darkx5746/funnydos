#include <conio.h>
#include <dos.h>

#include "vgapm.h"

void vgaPmPutPixel( int x, int y, byte color )
{
	word absAddr, offset, planeNum;

    if ( x<0 || x>319 || y<0 || y>199 ) return;

	absAddr = vRamAddrTable[activePage][y];
	offset = x & 0x03;
	absAddr += x >> 2;
    planeNum = 0x0100 << ( offset );

    outpw( 0x3c4,planeNum | 0x02 );

    *(VRAMADDRESS + absAddr) = color;

    outpw( 0x03c4, 0x0f02 );
}

byte vgaPmGetPixel( int x, int y )
{
	word absAddr, offset;
	byte color;

    if ( x<0 || x>319 || y<0 || y>199 ) return 0;

	absAddr = vRamAddrTable[activePage][y];
	offset = x & 0x03;
	absAddr += x >> 2;

    outp( 0x3ce, 0x04 );
    outp( 0x3cf, offset );

    color = *( VRAMADDRESS + absAddr );

    outpw( 0x3ce, 0x0004 );

	return color;
}

