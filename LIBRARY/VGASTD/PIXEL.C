#include "vgastd.h"

byte vgaStdGetPixel( int x, int y )
{
    if ( x < 0 || x >= VGA256XSIZE ||
         y < 0 || y >= VGA256YSIZE )
        return 0;

    return ( *(VRAMADDRESS + y * VGA256XSIZE + x) );
}

void vgaStdPutPixel( int x, int y, int color )
{
    if ( x < 0 || x >= VGA256XSIZE ||
         y < 0 || y >= VGA256YSIZE )
        return;

	*(VRAMADDRESS + y * VGA256XSIZE + x) = ( byte ) color;
}

