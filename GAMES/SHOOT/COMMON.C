#include <stdio.h>
#include <dos.h>
#include <process.h>

#include "common.h"

extern char *ERR_FATAL = "Fatal error !";
extern char *ERR_FILE = "File manipulation error.";
extern char *ERR_MEMORY = "Insufficient memory.";

static char *errMessage = NULL;

void setError( char *str )
{
    errMessage = str;
}

void amdCloseMusic(void);
void vgaSprClose(void);
void restoreKbd(void);
byte vgaSetMode(int);

void errExit( char *str )
{
	amdCloseMusic();
	vgaSprClose();
	restoreKbd();

	vgaSetMode(0x03);

	if ( str )
		puts( str );
	else
	if ( errMessage )
		puts( errMessage );

	exit( 1 );
}
