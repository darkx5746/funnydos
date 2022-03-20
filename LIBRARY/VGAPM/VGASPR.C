#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dos.h>

#include "vgaspr.h"

/* static functions */

static int vgaSprRefreshUpper( int id );
static int vgaSprFreeBuf( int id );

/* global variable */

Spr_Attr sprAttrBuf[MAXSPRITE];

byte invisibleColorEnabled = true;
byte detailCrushCheckEnabled = false;

/* -- Plane Mode extra function   ----------------------------------- */

word vgaPmPutImageClip( int x, int y, byte * buffer,
                           int cxs, int cys, int cxe, int cye )
{
    int i, j, k, offs, xsize, ysize, xend, yend;
    byte *buf;
    int planeNum = 0x0100;

    if ( !buffer )
		return FAIL;

    if ( x > cxe || y > cye )
		return false;

	xsize = buffer[0];
    xsize |= ( int ) ( buffer[1] << 8 );
	ysize = buffer[2];
    ysize |= ( int ) ( buffer[3] << 8 );

    xend = min( min( x + xsize + 3, cxe + 3 ), 319 );
	yend = y + ysize - 1;

    if ( xend < cxs || yend < cys )
		return false;

    for ( k = 0; k < 4; k++, planeNum <<= 1 )
	{
        outpw( 0x3c4, planeNum | 0x02 );

		buf = buffer + 4;

        for ( j = y; j <= yend; j++, buf += xsize )
		{
            if ( j < cys || j > cye || j < 0 || j > 199 )
				continue;
            for ( i = x & 0xffc; i <= xend; i += 4 )
			{
                offs = ( i + k );
                if ( offs < cxs || offs > cxe )
					continue;
				offs -= x;
                if ( offs < 0 )
					continue;
                if ( offs >= xsize )
					continue;
                if ( invisibleColorEnabled && *( buf + offs ) == invisibleColor )
					continue;
				else
                    *( VRAMADDRESS + vRamAddrTable[activePage][j] + ( i >> 2 ) ) =
                        *( buf + offs );
			}
		}
	}
    outpw( 0x03c4, 0x0f02 );
    return ( word ) ( buf - buffer );
}

/* --- SPRITE MAIN FUNCTIONS ----------- */

int vgaSprInit( void )
{
	int i;

	if ( !isPlaneModeSet )
		return false;

    for ( i = 0; i < MAXSPRITE; i++ )
		sprAttrBuf[i].defined = false;

	return true;
}

int vgaSprSet( int id, byte * buffer )
{
    int xsize, ysize;

    if ( sprAttrBuf[id].defined )
		return false;

	sprAttrBuf[id].displayed = OFF;

    if ( buffer == VRAMSPRITE )
	{
        sprAttrBuf[id].buffer = ( byte * ) VRAMSPRITE;
		sprAttrBuf[id].vram.x = -1;
		sprAttrBuf[id].vram.y = -1;
		xsize = sprAttrBuf[id].vram.xsize = -1;
		ysize = sprAttrBuf[id].vram.ysize = -1;
	}
	else
	{
        xsize = ( ( int * ) buffer )[0];
        ysize = ( ( int * ) buffer )[1];

        if ( !( sprAttrBuf[id].buffer = ( byte * ) malloc( xsize * ysize + 4 ) ) )
		{
			return false;
		}
        memcpy( sprAttrBuf[id].buffer, buffer, ( size_t ) xsize * ysize + 4 );
		sprAttrBuf[id].vram.x =
			sprAttrBuf[id].vram.y =
			sprAttrBuf[id].vram.xsize =
			sprAttrBuf[id].vram.ysize = 0;
	}

	sprAttrBuf[id].cArea.x =
		sprAttrBuf[id].cArea.y = 0;
	sprAttrBuf[id].xsize =
		sprAttrBuf[id].cArea.xsize = xsize;
	sprAttrBuf[id].ysize =
		sprAttrBuf[id].cArea.ysize = ysize;
	sprAttrBuf[id].crushed = false;

	sprAttrBuf[id].defined = true;

	return true;
}

void vgaSprFree( int id )
{
    if ( !sprAttrBuf[id].defined )
		return;

    vgaSprHide( id );

    if ( sprAttrBuf[id].buffer != VRAMSPRITE )
	{
        if ( sprAttrBuf[id].buffer && ( sprAttrBuf[id].vram.x != -1 ) )
            free( sprAttrBuf[id].buffer );
        sprAttrBuf[id].buffer = ( byte * ) NULL;
	}
	sprAttrBuf[id].defined = false;
}

int vgaSprPut( int id, int x, int y )
{
    word( *PutImage ) ( int, int, byte * );
	Box b;

	sprAttrBuf[id].crushed = false;

	PutImage = invisibleColorEnabled ?
		vgaPmPutImageInviCol : vgaPmPutImage;

    if ( !sprAttrBuf[id].defined )
		return false;

    if ( vgaSprIsDisplayed( id, activePage ) )
        vgaSprPageHide( id, activePage );

	sprAttrBuf[id].img_area[activePage].x = x;
	sprAttrBuf[id].img_area[activePage].y = y;
	sprAttrBuf[id].img_area[activePage].xsize = sprAttrBuf[id].xsize;
	sprAttrBuf[id].img_area[activePage].ysize = sprAttrBuf[id].ysize;
    resetBoxEnd( &sprAttrBuf[id].img_area[activePage] );

    if ( sprAttrBuf[id].buffer == VRAMSPRITE )
	{
		b = sprAttrBuf[id].vram;
        vgaPmPageCopy( activePage, x, y,
                      SPRITEPAGE, b.x, b.y, b.xsize, b.ysize );
        sprAttrBuf[id].displayed |= ( 1 << activePage );
        sprAttrBuf[id].crushed = vgaSprRefreshUpper( id );
	}
	else
	{
        ( *PutImage ) ( x, y, sprAttrBuf[id].buffer );
        sprAttrBuf[id].displayed |= ( 1 << activePage );
        sprAttrBuf[id].crushed = vgaSprRefreshUpper( id );
	}

	return sprAttrBuf[id].crushed;
}

int vgaSprReset( int id )
{
    int crPage = activePage, i, temp = false;

    for ( i = 0; i < 4; i++ )
	{
        vgaPmSetActivePage( i );
        if ( vgaSprIsDisplayed( id, activePage ) )
            if ( vgaSprPut( id, vgaSprGetXloc( id ), vgaSprGetYloc( id ) ) )
				temp = true;
	}
    vgaPmSetActivePage( crPage );
	return temp;
}

int vgaSprRefreshUpper( int id )
{
    int i, crushed = false;
    Box b, b1, b2;

    if ( !vgaSprIsDisplayed( id, activePage ) )
		return false;

	b = sprAttrBuf[id].img_area[activePage];

    for ( i = id + 1; i < MAXSPRITE; i++ )
	{
        if ( !vgaSprIsDefined( i ) )
			continue;
        if ( !vgaSprIsDisplayed( i, activePage ) )
			continue;
		b1 = sprAttrBuf[i].img_area[activePage];
        b2 = crossBox( b, b1 );
        if ( nullBox( b2 ) )
			continue;
        if ( sprAttrBuf[i].buffer == VRAMSPRITE )
		{
			b2 = sprAttrBuf[i].vram;
            vgaPmPageCopy( activePage, b1.x, b1.y,
                          SPRITEPAGE, b2.x, b2.y, b2.xsize, b2.ysize );
			b2.x = b1.x;
			b2.y = b1.y;
            resetBoxEnd( &b2 );
            b = unionBox( b, b2 );
		}
		else
            vgaPmPutImageClip( b1.x, b1.y,
                              sprAttrBuf[i].buffer,
                              box2int( b2 ) );
		crushed = true;
	}
	return crushed;
}

int vgaSprRefreshArea( int x, int y, int xe, int ye )
{
    int i, crushed = false;
    Box b, b1, b2;

	b.x = x;
	b.y = y;
	b.xend = xe;
	b.yend = ye;
    resetBox( &b );

    if ( nullBox( b ) )
		return false;
    for ( i = 0; i < MAXSPRITE; i++ )
	{
        if ( !vgaSprIsDefined( i ) )
			continue;
        if ( !vgaSprIsDisplayed( i, activePage ) )
			continue;
		b1 = sprAttrBuf[i].img_area[activePage];
        b2 = crossBox( b, b1 );
        if ( nullBox( b2 ) )
			continue;
        if ( sprAttrBuf[i].buffer == VRAMSPRITE )
		{
			b2 = sprAttrBuf[i].vram;
            vgaPmPageCopy( activePage, b1.x, b1.y,
                          SPRITEPAGE, b2.x, b2.y, b2.xsize, b2.ysize );
			b2.x = b1.x;
			b2.y = b1.y;
            resetBoxEnd( &b2 );
            b = unionBox( b, b2 );
		}
		else
            vgaPmPutImageClip( b1.x, b1.y,
                              sprAttrBuf[i].buffer,
                              box2int( b2 ) );
		crushed = true;
	}
	return crushed;
}

void vgaSprPageHide( int id, int page )
{
	int crPg = activePage;
	Box b;

    vgaPmSetActivePage( page );

    if ( vgaSprIsDisplayed( id, activePage ) )
	{
		b = sprAttrBuf[id].img_area[activePage];
        vgaPmPageCopy( activePage,
                      b.x, b.y,
                      BACKGROUNDPAGE,
                      b.x, b.y, b.xsize, b.ysize );
        sprAttrBuf[id].displayed &= ~( 1 << activePage );
        vgaSprRefreshArea( b.x, b.y, b.xend, b.yend );
	}
    vgaPmSetActivePage( crPg );
}

void vgaSprHide( int id )
{
	int i;

    for ( i = 0; i < 4; i++ )
        vgaSprPageHide( id, i );
}

int vgaSprCheckCrush( int id1, int id2 )
{
    int i, j;
    byte d1, d2;
    Box b1, b2, b0;

    if ( id1 == id2 )
		return false;
    if ( !sprAttrBuf[id1].defined )
		return false;
    if ( !sprAttrBuf[id2].defined )
		return false;
    if ( !sprAttrBuf[id1].displayed )
		return false;
    if ( !sprAttrBuf[id2].displayed )
		return false;

    b1.x = ( sprAttrBuf[id1].cArea.x ) + ( sprAttrBuf[id1].img_area[activePage].x );
    b1.y = ( sprAttrBuf[id1].cArea.y ) + ( sprAttrBuf[id1].img_area[activePage].y );
	b1.xsize = sprAttrBuf[id1].cArea.xsize;
	b1.ysize = sprAttrBuf[id1].cArea.ysize;
    resetBoxEnd( &b1 );
    b2.x = ( sprAttrBuf[id2].cArea.x ) + ( sprAttrBuf[id2].img_area[activePage].x );
    b2.y = ( sprAttrBuf[id2].cArea.y ) + ( sprAttrBuf[id2].img_area[activePage].y );
	b2.xsize = sprAttrBuf[id2].cArea.xsize;
	b2.ysize = sprAttrBuf[id2].cArea.ysize;
    resetBoxEnd( &b2 );
    b0 = crossBox( b1, b2 );
    if ( nullBox( b0 ) )
		return false;
    else if ( !detailCrushCheckEnabled )
		return true;
    else if ( sprAttrBuf[id1].buffer == VRAMSPRITE &&
        sprAttrBuf[id2].buffer == VRAMSPRITE )
            return true;

	b1.x = b0.x - b1.x;
	b1.y = b0.y - b1.y;
	b2.x = b0.x - b2.x;
	b2.y = b0.y - b2.y;

    for ( i = 0; i < b0.xsize; i++ )
        for ( j = 0; j < b0.ysize; j++ )
		{
            if ( sprAttrBuf[id1].buffer == VRAMSPRITE )
				d1 = ~invisibleColor;
			else
                d1 = *( sprAttrBuf[id1].buffer +
                     ( sprAttrBuf[id1].xsize ) * ( j + b1.y ) + ( i + b1.x ) + 4 );
            if ( sprAttrBuf[id2].buffer == VRAMSPRITE )
				d2 = ~invisibleColor;
			else
                d2 = *( sprAttrBuf[id2].buffer +
                     ( sprAttrBuf[id2].xsize ) * ( j + b2.y ) + ( i + b2.x ) + 4 );
            if ( d1 == invisibleColor || d2 == invisibleColor )
				continue;
			else
				return true;
		}
	return false;
}

int vgaSprCheckCrushNum( int id1, int id_start, int id_end )
{
    int i, j, id;
    byte d1, d2;
    Box b1, b2, b0;

    if ( !sprAttrBuf[id1].defined )
		return false;
    if ( !sprAttrBuf[id1].displayed )
		return false;

    b1.x = ( sprAttrBuf[id1].cArea.x ) + ( sprAttrBuf[id1].img_area[activePage].x );
    b1.y = ( sprAttrBuf[id1].cArea.y ) + ( sprAttrBuf[id1].img_area[activePage].y );
	b1.xsize = sprAttrBuf[id1].cArea.xsize;
	b1.ysize = sprAttrBuf[id1].cArea.ysize;
    resetBoxEnd( &b1 );

    for ( id = id_start; id <= id_end; id++ )
	{
        if ( id1 == id )
			continue;
        if ( !sprAttrBuf[id].defined )
			continue;
        if ( !sprAttrBuf[id].displayed )
			continue;
        b2.x = ( sprAttrBuf[id].cArea.x ) + ( sprAttrBuf[id].img_area[activePage].x );
        b2.y = ( sprAttrBuf[id].cArea.y ) + ( sprAttrBuf[id].img_area[activePage].y );
		b2.xsize = sprAttrBuf[id].cArea.xsize;
		b2.ysize = sprAttrBuf[id].cArea.ysize;
        resetBoxEnd( &b2 );
        b0 = crossBox( b1, b2 );
        if ( nullBox( b0 ) )
			continue;
        else if ( !detailCrushCheckEnabled )
			return true;
        else if ( sprAttrBuf[id1].buffer == VRAMSPRITE &&
                 sprAttrBuf[id].buffer == VRAMSPRITE )
			return true;

		b1.x = b0.x - b1.x;
		b1.y = b0.y - b1.y;
		b2.x = b0.x - b2.x;
		b2.y = b0.y - b2.y;

        for ( i = 0; i < b0.xsize; i++ )
            for ( j = 0; j < b0.ysize; j++ )
			{
                if ( sprAttrBuf[id1].buffer == VRAMSPRITE )
					d1 = ~invisibleColor;
				else
                    d1 = *( sprAttrBuf[id1].buffer +
                     ( sprAttrBuf[id1].xsize ) * ( j + b1.y ) + ( i + b1.x ) + 4 );
                if ( sprAttrBuf[id].buffer == VRAMSPRITE )
					d2 = ~invisibleColor;
				else
                    d2 = *( sprAttrBuf[id].buffer +
                      ( sprAttrBuf[id].xsize ) * ( j + b2.y ) + ( i + b2.x ) + 4 );
                if ( d1 == invisibleColor || d2 == invisibleColor )
					continue;
				else
					return true;
			}
        b1.x = ( sprAttrBuf[id1].cArea.x ) + ( sprAttrBuf[id1].img_area[activePage].x );
        b1.y = ( sprAttrBuf[id1].cArea.y ) + ( sprAttrBuf[id1].img_area[activePage].y );
	}
	return false;
}

void vgaSprSetBuf( int id, byte * buf )
{
    if ( sprAttrBuf[id].vram.x >= 0 )
        vgaSprFreeBuf( id );

	sprAttrBuf[id].vram.x = -1;
	sprAttrBuf[id].vram.y = -1;
	sprAttrBuf[id].vram.xsize = -1;
	sprAttrBuf[id].vram.ysize = -1;

	sprAttrBuf[id].buffer = buf;
	sprAttrBuf[id].xsize =
        sprAttrBuf[id].cArea.xsize = ( ( int * ) buf )[0];
	sprAttrBuf[id].ysize =
        sprAttrBuf[id].cArea.ysize = ( ( int * ) buf )[1];
	sprAttrBuf[id].cArea.x = sprAttrBuf[id].cArea.y = 0;
}

int vgaSprFreeBuf( int id )
{
	int k = false;

    if ( sprAttrBuf[id].buffer )
	{
        free( sprAttrBuf[id].buffer );
        sprAttrBuf[id].buffer = ( byte * ) NULL;
		k = true;
	}
	return k;
}

void vgaSprSetVRamBuf( int id, int x, int y, int xe, int ye )
{
    if ( sprAttrBuf[id].vram.x >= 0 )
        vgaSprFreeBuf( id );
    sprAttrBuf[id].buffer = ( byte * ) NULL;

	sprAttrBuf[id].vram.x = x;
	sprAttrBuf[id].vram.y = y;
	sprAttrBuf[id].vram.xend = xe;
	sprAttrBuf[id].vram.yend = ye;
    resetBox( &sprAttrBuf[id].vram );

    if ( nullBox( sprAttrBuf[id].vram ) )
	{
		sprAttrBuf[id].vram.x = -1;
		sprAttrBuf[id].vram.y = -1;
		sprAttrBuf[id].vram.xsize = -1;
		sprAttrBuf[id].vram.ysize = -1;
	}
	else
	{
		sprAttrBuf[id].cArea.x =
			sprAttrBuf[id].cArea.y = 0;
		sprAttrBuf[id].cArea.xsize =
			sprAttrBuf[id].xsize = sprAttrBuf[id].vram.xsize;
		sprAttrBuf[id].cArea.ysize =
			sprAttrBuf[id].ysize = sprAttrBuf[id].vram.ysize;
	}
}

void vgaSprSetCrushArea( int id, int x, int y, int xe, int ye )
{
	sprAttrBuf[id].cArea.x = x;
	sprAttrBuf[id].cArea.y = y;
	sprAttrBuf[id].cArea.xend = xe;
	sprAttrBuf[id].cArea.yend = ye;
    resetBox( &( sprAttrBuf[id].cArea ) );
}

void vgaSprClose( void )
{
	int i;

    for ( i = 0; i < MAXSPRITE; i++ )
        vgaSprFree( i );
}

