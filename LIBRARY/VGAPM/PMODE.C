#include <dos.h>
#include <conio.h>
#include <dos.h>

#include "vgapm.h"

byte activePage;
word vRamStartAddr;
byte isPlaneModeSet = false;
word vRamAddrTable[4][200];

void vgaPmSetPlaneMode( void )
{
    int i, j, k;

    outpw( 0x3c4, 0x0604 );
    outpw( 0x3d4, 0x0014 );
	outpw( 0x3d4, 0xe317 );

	vRamStartAddr = 0x0;

	for ( i = 0, k = 0; i < 4; i++ )
		for ( j = 0; j < 200; j++, k += 80 )
			vRamAddrTable[i][j] = k;
    isPlaneModeSet = true;
}

void vgaPmSetDispPage( byte pageNum )
{
	vRamStartAddr = vRamAddrTable[pageNum][0];

	vgaPmSetStartAddr( vRamStartAddr );

	activePage = pageNum;
}

byte vgaPmGetDispPage( void )
{
	int page;

	for( page = 0; page < 4; page++ )
		if ( vRamStartAddr == vRamAddrTable[page][0] )
			return page;

	return 255;
}

void vgaPmSetWriteMode( byte mode )
{
   byte mask;

   outp( 0x3ce, 0x5 );
   mask = inp( 0x3cf );
   outp( 0x3cf, ( mask & 0xfc ) | mode );
}

void vgaPmSetStartAddr( int _offset )
{
	asm {
		mov bx, _offset;
		mov dx, 0x03d4;
		mov al, 0x0d;
		mov ah, bl;
		out dx, ax;

		mov dx, 0x03d4;
		mov al, 0x0c;
		mov ah, bh;
		out dx, ax;
	}
	vgaCheckVSync();
}

void vgaPmClearScr( int color )
{
	word i;

	for ( i = 0; i < 16000; i++ )
		*(VRAMADDRESS + vRamAddrTable[activePage][0] + i) = color;
}

void vgaPmSetGraphMode( void )
{
	vgaSetMode( VGA256COL );
	vgaPmSetPlaneMode();
	vgaPmSetDispPage( 0 );
	vgaPmClearScr( 0 );
	vgaPmFullPageCopy( 1, 0 );
	vgaPmFullPageCopy( 2, 0 );
	vgaPmFullPageCopy( 3, 0 );
}

void vgaPmEndGraphMode( void )
{
	vgaSetMode( 0x03 );
}

