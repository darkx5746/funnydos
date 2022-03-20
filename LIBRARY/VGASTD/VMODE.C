#include <dos.h>
#include "common.h"

#ifndef VGABIOSNUM
#define VGABIOSNUM 0x10
#endif

int vgaGetMode( void )
{
	union REGS regs;

	regs.h.ah = 0x0f;

	int86( VGABIOSNUM, &regs, &regs );

	return ( int )( regs.h.al );
}

int vgaSetMode( int modenum )
{
	union REGS regs;

	regs.h.ah = 0;
	regs.h.al = ( byte )modenum;

	int86( VGABIOSNUM, &regs, &regs );

	if ( vgaGetMode() != modenum )
		return false;
	else
		return true;
}

void vgaCheckVSync( void )
{
	asm mov dx, 0x03da;
L1:
	asm {
		in  al, dx;
		test al, 0x08;
		jz L1;
	}
}
