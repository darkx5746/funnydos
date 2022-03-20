#include <stdio.h>
#include <dos.h>
#include <process.h>

#include "timer.h"
#include "vgastd.h"

extern char *ERR_FATAL = "Fatal error !";
extern char *ERR_FILE = "File manipulation error.";
extern char *ERR_MEMORY = "Insufficient memory.";

static char *errMessage = NULL;

void setError( char *str )
{
    errMessage = str;
}

void errExit( char *str )
{
	nosound();
	stopNewTimer();
	vgaStdEndGraphMode();

	if ( errMessage )
		puts( errMessage );
	if ( str )
		puts( str );

	exit( 1 );
}
