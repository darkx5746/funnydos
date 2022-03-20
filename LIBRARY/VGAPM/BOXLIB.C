#include "boxlib.h"

/* -- sublibrary dealing Rect box -----------------------------------*/

Box sizeBox( int x, int y, int xsize, int ysize )
{
	static Box temp;

	temp.x = x;
	temp.y = y;
	temp.xsize = xsize;
	temp.ysize = ysize;
	temp.xend = x+xsize-1;
	temp.yend = y+ysize-1;

	return temp;
}

Box endBox( int x, int y, int xend, int yend )
{
	static Box temp;
	temp.x = x;
	temp.y = y;
	temp.xend = xend;
	temp.yend = yend;
    resetBoxSize( &temp );
	return temp;
}

void resetBoxSize( Box* temp )
{
    static int xsize, ysize;

	xsize = temp->xend - temp->x + 1;
	ysize = temp->yend - temp->y + 1;

    temp->xsize = ( xsize <= 0 ? 0 : xsize );
    temp->ysize = ( ysize <= 0 ? 0 : ysize );
}

void resetBoxEnd( Box* temp )
{
	temp->xend = temp->xsize + temp->x - 1;
	temp->yend = temp->ysize + temp->y - 1;
}

Box unionBox( Box b1, Box b2 )
{
	static Box b;

    b.x = min( b1.x, b2.x );
    b.y = min( b1.y, b2.y );
    b.xend = max( b1.xend, b2.xend );
    b.yend = max( b1.yend, b2.yend );
    resetBox( &b );

	return b;
}

Box crossBox ( Box b1, Box b2 )
{
	static Box b;

    b.x = max( b1.x, b2.x );
    b.y = max( b1.y, b2.y );
    b.xend = min( b1.xend, b2.xend );
    b.yend = min( b1.yend, b2.yend );
    resetBox( &b );

	return b;
}

