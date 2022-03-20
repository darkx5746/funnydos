#include <stdio.h>
#include <stdarg.h>
#include "stdhan.h"

void vgaStdPrintfxy( int x, int y, char *fmt, ... )
{
	char s[MAXSTRING];

	va_list argptr;
	va_start( argptr, fmt );
	vsprintf( s, fmt, argptr );
	va_end(argptr);
	vgaStdPutsxy( x, y, s );
}

void vgaStdPrintf( char *fmt, ... )
{
	char s[MAXSTRING];

	va_list argptr;
	va_start( argptr, fmt );
	vsprintf( s, fmt, argptr );
	va_end(argptr);
	vgaStdPuts( s );
}