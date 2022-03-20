#include "vgapm.h"

void vgaPmPaint( int x, int y, int col )
{
    int i, j, curCol;

    if ( x < 0 || x >= 320 || y >= 200 ) return;

    curCol = vgaPmGetPixel( x, y );

    if ( curCol == col ) return;

    for ( i = x; i >= 0; i-- )
        if ( vgaPmGetPixel( i, y ) != curCol ) break;
	i++;
    for ( j = i; j < 320; j++ )
	{
        if ( vgaPmGetPixel( j, y ) != curCol ) break;
	}
	j--;
    vgaPmHLine( i, y, j, col );
    for ( j = i; j < 320; j++ )
	{
        if ( vgaPmGetPixel( j, y ) != col ) break;
        if ( vgaPmGetPixel( j, y-1 ) == curCol && y-1 < 200 )
            vgaPmPaint( j, y-1, col );
        if ( vgaPmGetPixel( j, y+1 ) == curCol && y+1 < 200 )
            vgaPmPaint( j, y+1, col );
	}
}

void vgaPmCircle( int xx, int yy, int radius, int color )
{
    int y, d, x;

	y = radius;
	d = 3 - y * 2;
	x = 0;

    while ( x < y )
	{
        vgaPmPutPixel( xx+x, yy+y, color );
        vgaPmPutPixel( xx - x, yy + y, color );
        vgaPmPutPixel( xx + x, yy - y, color );
        vgaPmPutPixel( xx - x, yy - y, color );
        vgaPmPutPixel( xx - y, yy + x, color );
        vgaPmPutPixel( xx + y, yy - x, color );
        vgaPmPutPixel( xx - y, yy - x, color );
        vgaPmPutPixel( xx + y, yy + x, color );
        if ( d < 0 )
            d += (x * 4 + 6);
		else
		{
            d += ((x - y) * 4 + 10);
			y--;
		}
		x++;
	}
    if ( x == y )
	{
        vgaPmPutPixel( xx + x, yy + y, color );
        vgaPmPutPixel( xx - x, yy + y, color );
        vgaPmPutPixel( xx + x, yy - y, color );
        vgaPmPutPixel( xx - x, yy - y, color );
        vgaPmPutPixel( xx - y, yy + x, color );
        vgaPmPutPixel( xx + y, yy - x, color );
        vgaPmPutPixel( xx - y, yy - x, color );
        vgaPmPutPixel( xx + y, yy + x, color );
	}
}

