#include <dos.h>

#include "key.h"

static byte far *keyBufferHead = (byte far *) ( 0x0040001aL );
static byte far *keyBufferTail = (byte far *) ( 0x0040001cL );
static byte far *ShiftStatus   = (byte far *) ( 0x00400017L );

static int isNewKbdInstalled = FALSE;

static void (interrupt far *oldKBD)( void );
static void (*keyEvent)(int);

static void interrupt far keyInt( void );

void setKbd( void (*event)(int) )
{
	if ( isNewKbdInstalled ) return;

	oldKBD = getvect( 0x09 );
	setvect( 0x09, keyInt );
	keyEvent = event;
	isNewKbdInstalled = TRUE;
}

void restoreKbd( void )
{
	if ( !isNewKbdInstalled ) return;

	setvect( 0x09, oldKBD );
	isNewKbdInstalled = FALSE;
}

word keyRead( void )
{
	union REGS regs;

	if ( isNewKbdInstalled ) return 0;

	*keyBufferHead = *keyBufferTail-2;
	regs.h.ah = 0x00;
	int86( 0x16, &regs, &regs );

	if( regs.h.al )
		return( regs.h.al );
	else
		return( regs.h.ah + 0x100 );
}

int keyPressed( void )
{
	if ( isNewKbdInstalled ) return false;

	return( *keyBufferHead != *keyBufferTail );
}

byte keyShiftStatus( void )
{
	return( *ShiftStatus );
}

void interrupt far keyInt( void )
{
	byte key;

	key=inportb( 0x60 );

	(*keyEvent)( key );

	outportb( 0x20, 0x20 );
}

