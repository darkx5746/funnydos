#include <alloc.h>

#include "vgapm.h"

void vgaPmFullPageCopy( byte targetPage, byte sourcePage )
{
    word s, t;

    s = vRamAddrTable[sourcePage][0];
    t = vRamAddrTable[targetPage][0];

    vgaPmSetWriteMode( 1 );

    asm {
        mov si,  s;
        mov di,  t;

        push ds;
        push es;

        mov ax,  VRAMSEGMENT;
        mov ds,  ax;
        mov es,  ax;

        mov cx,  16000;
        rep movsb;

        pop es;
        pop ds;
    }
    vgaPmSetWriteMode( 0 );
}

void vgaPmVRamCopy( byte targetPage, int targetX, int targetY,
    byte sourcePage, int sourceX, int sourceY, int xsize, int ysize )
{
    int i, j, tx, ty, xend, yend;
    word srOffset, tgOffset;

    xend = sourceX+xsize;
    yend = sourceY+ysize;

    vgaPmSetWriteMode( 1 );

    for ( j = sourceY, ty = targetY; j < yend; j++, ty++ )
	{
        if ( j<0 || j>199 || ty<0 || ty>199 ) continue;
        for ( i = sourceX, tx = targetX; i < xend; i++, tx++ )
		{
            if ( i<0 || i>79 || tx<0 || tx>79 ) continue;
            srOffset = vRamAddrTable[sourcePage][j]+i;
			tgOffset = vRamAddrTable[targetPage][ty]+tx;
            *(VRAMADDRESS + tgOffset) = *(VRAMADDRESS + srOffset);
		}
	}
    vgaPmSetWriteMode( 0 );
}

void vgaPmPageCopy( byte targetPage, int targetX, int targetY,
    byte sourcePage, int sourceX, int sourceY, int xsize, int ysize )
{
    byte currentPage = vgaPmGetActivePage(  );
	byte *buffer;
    int sxend = sourceX + xsize - 1, temp;

    if ( xsize<1 || ysize<1 ) return;

    if ( (sourceX & 3) == (targetX & 3) )
	{
        if ( sourceX & 3 )
		{
			sxend = sourceX | 0x03;
			temp = sxend - sourceX + 1;
            buffer = (byte*) malloc( (word)temp * ysize + 4 );
            if ( !buffer )
                errExit( ERR_MEMORY );
            vgaPmSetActivePage( sourcePage );
            vgaPmGetImage( sourceX, sourceY, sxend, sourceY+ysize-1, buffer );
            vgaPmSetActivePage( targetPage );
            vgaPmPutImage( targetX,  targetY,  buffer );
            vgaPmSetActivePage( currentPage );
            if ( buffer ) free( buffer );
            sourceX += temp;
            targetX += temp;
            xsize   -= temp;
            if ( xsize<1 ) return;
			sxend = sourceX + xsize - 1;
		}

        if ( (sxend & 0xffc) != (sourceX & 0xffc) )
		{
			temp = (((sxend+1) - sourceX) >> 2);
            vgaPmVRamCopy( targetPage, targetX>>2, targetY,
                           sourcePage, sourceX>>2, sourceY, temp, ysize );
            sourceX += ( temp<<2 );
            targetX += ( temp<<2 );
            xsize   -= ( temp<<2 );
			if ( xsize<1 ) return;
		}

		{
            buffer = (byte*) malloc( (word)((sxend & 3) + 1) * ysize + 4 );
                if ( !buffer )
					errExit( ERR_MEMORY );
            vgaPmSetActivePage( sourcePage );
            vgaPmGetImage( sourceX, sourceY, sourceX+xsize-1, sourceY+ysize-1,  buffer );
            vgaPmSetActivePage( targetPage );
            vgaPmPutImage( targetX,  targetY,  buffer );
            vgaPmSetActivePage( currentPage );
            if ( buffer ) free( buffer );
		}
	}

	else

	{
        buffer = ( byte* )malloc( (word) xsize * ysize + 4 );
        if ( !buffer )
            errExit( ERR_MEMORY );
        vgaPmSetActivePage( sourcePage );
        vgaPmGetImage( sourceX,  sourceY, sourceX+xsize-1, sourceY+ysize-1,  buffer );
        vgaPmSetActivePage( targetPage );
        vgaPmPutImage( targetX,  targetY,  buffer );
        vgaPmSetActivePage( currentPage );
        if ( buffer ) free( buffer );
	}
}

