#include "vgastd.h"

void vgaStdHLine( int xs, int ys, int xe, int color )
{
	if ( xs < 0 ) xs = 0;

	if ( ys < 0 ) ys = 0;

	if ( xe >= VGA256XSIZE ) xe = VGA256XSIZE - 1;

	if ( xs >= VGA256XSIZE ) return;

	if ( ys >= VGA256YSIZE ) return;

	asm {
		push es;
		mov  ax, xs;
		mov  bx, xe;
		cmp  ax, bx;
		jle  go;
		mov  xs, bx;
		mov  xe, ax;
	}
go:
    asm {
        mov  ax, ys;
		mov  bx, VGA256XSIZE;
		mul  bx;
        add  ax, xs;
        mov  di, ax;

        mov  cx, xe;
        sub  cx, xs;
        inc  cx;
        mov  dx, color;
        mov  ax, VRAMSEGMENT;
        mov  es, ax;
	}
loop1:
    asm {
        mov  es:[di], dl
		inc  di;
		loop loop1;
		pop  es;
	}
}

void vgaStdVLine( int xs, int ys, int ye, int color )
{
    if ( xs < 0 ) xs = 0;

    if ( ys < 0 ) ys = 0;

	if ( ye >= VGA256YSIZE ) ye = VGA256YSIZE - 1;

	if ( xs >= VGA256XSIZE ) return;

	if ( ys >= VGA256YSIZE ) return;

    asm {
		push es;
        mov  ax, ys;
        mov  bx, ye;
        cmp  ax, bx;
		jle  go;
		mov  ys, bx;
        mov  ye, ax;
	}
go:
   asm {
        mov  ax, ys;
		mov  bx, VGA256XSIZE;
		mul  bx;
        add  ax, xs;
        mov  di, ax;

        mov  cx, ye;
		sub  cx, ys;
		inc  cx;
        mov  dx, color;
        mov  ax, VRAMSEGMENT;
        mov  es, ax;
	}
loop1:
    asm {
        mov  es:[di], dl;
		add  di, VGA256XSIZE;
		loop loop1;
		pop  es;
	}
}

void vgaStdBox(int xs, int ys, int xe, int ye, int color)
{
	vgaStdHLine( xs, ys, xe, color );
	vgaStdHLine( xs, ye, xe, color );
	vgaStdVLine( xs, ys, ye, color );
	vgaStdVLine( xe, ys, ye, color );
}

void vgaStdBoxFill(int xs, int ys, int xe, int ye, int color)
{
	register i;

	if ( xs > xe )
	{
		i = xs;
		xe = xs;
		xs = i ;
	}

	if ( ys > ye )
	{
		i = ys;
		ye = ys;
		ys = i;
	}

	for( i = ys ; i <= ye ; i++ )
		vgaStdHLine( xs, i, xe, color );
}

void vgaStdClearScr( int color )
{
	vgaStdBoxFill( 0, 0, VGA256XSIZE-1, VGA256YSIZE-1, color );
}

