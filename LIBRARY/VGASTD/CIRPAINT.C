#include "vgastd.h"

void vgaStdCircle( int xx, int yy, int radius, int color )
{
	int y, d, x;

	y = radius;
	d = 3 - y * 2;
	x = 0;

	while ( x < y )
	{
        vgaStdPutPixel( xx + x, yy + y, color );
		vgaStdPutPixel( xx - x, yy + y, color );
		vgaStdPutPixel( xx + x, yy - y, color );
		vgaStdPutPixel( xx - x, yy - y, color );
		vgaStdPutPixel( xx - y, yy + x, color );
		vgaStdPutPixel( xx + y, yy - x, color );
		vgaStdPutPixel( xx - y, yy - x, color );
		vgaStdPutPixel( xx + y, yy + x, color );
		if ( d < 0 )
			d += ( x * 4 + 6 );
		else
		{
			d += ( ( x - y ) * 4 + 10 );
			y--;
		}
		x++;
	}
	if ( x == y )
	{
		vgaStdPutPixel( xx + x, yy + y, color );
		vgaStdPutPixel( xx - x, yy + y, color );
		vgaStdPutPixel( xx + x, yy - y, color );
		vgaStdPutPixel( xx - x, yy - y, color );
		vgaStdPutPixel( xx - y, yy + x, color );
		vgaStdPutPixel( xx + y, yy - x, color );
		vgaStdPutPixel( xx - y, yy - x, color );
		vgaStdPutPixel( xx + y, yy + x, color );
	}
}

void vgaStdPaint( int x, int y, int col )
{
	int i, j, curCol;

	if ( x < 0 || x >= 320 || y >= 200 ) return;

	curCol = vgaStdGetPixel( x, y );

	if ( curCol == col ) return;

	for ( i = x ; i >= 0 ; i-- )
        if ( vgaStdGetPixel( i, y ) != curCol ) break;
	i++;
    for ( j = i ; j < 320 ; j++ )
	{
        if ( vgaStdGetPixel( j, y ) != curCol ) break;
	}
	j--;
	vgaStdHLine( i, y, j, col );
	for ( j = i ; j < 320 ; j++ )
	{
		if ( vgaStdGetPixel( j, y ) != col ) break;
		if ( vgaStdGetPixel( j, y - 1 ) == curCol && ( y - 1 < 200 ) )
			vgaStdPaint( j, y - 1, col );
		if ( vgaStdGetPixel( j, y + 1 ) == curCol && ( y + 1 < 200 ) )
			vgaStdPaint( j, y + 1, col );
	}
}
