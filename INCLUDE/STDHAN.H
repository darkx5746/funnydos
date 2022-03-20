#ifndef __VGASTDHAN__
#define __VGASTDHAN__

#include "vgastd.h"

#define FONTXSIZE             8
#define FONTYSIZE             16
#define MAXSTRING             255

void vgaStdOpenHan(char *hanFontName,char *engFontName);
void vgaStdCloseHan(void);
void vgaStdSetFont( char *hanFontFile , char *engFontFile );

void vgaStdPuts(char *string);
void vgaStdPutsxy(int x,int y,char *string);
void vgaStdPutsxyC(int y, char *string);

int  vgaStdGetCenterOfStr( char *string );

void vgaStdSetPos(int x, int y);
void vgaStdGetPos(int *x, int *y);

void vgaStdSetForeColor(int color);
int  vgaStdGetForeColor( void );

void vgaStdPrintfxy(int x, int y, char *format , ... );
void vgaStdPrintf(char *format , ... );

#endif
