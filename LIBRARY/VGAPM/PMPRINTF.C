#include <stdarg.h>
#include <stdio.h>
#include "pmhan.h"

void vgaPmPrintfxy( int x, int y, char *fmt, ... )
{
	char s[255];

	va_list argptr;
	va_start( argptr, fmt );
	vsprintf( s, fmt, argptr );
	va_end(argptr);
	vgaPmPutsxy( x, y, s );
}

void vgaPmPrintf( char *fmt, ... )
{
	char s[255];

	va_list argptr;
	va_start( argptr, fmt );
	vsprintf( s, fmt, argptr );
	va_end(argptr);
	vgaPmPuts( s );
}