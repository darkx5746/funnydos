#ifndef __VGAPMHAN__
#define __VGAPMHAN__

#include "vgapm.h"

#define FONTXSIZE             8
#define FONTYSIZE             16
#define MAXSTRING             255

void vgaPmOpenHan(char *hanFontName,char *engFontName);
void vgaPmCloseHan(void);

void vgaPmPutEng(int x,int y,int color,byte *buffer);
void vgaPmPutHan(int x,int y,int color,byte *buffer);

void vgaPmPuts(char *string);
void vgaPmPutsxy(int x,int y,char *string);
void vgaPmPutsxyC(int y, char *string);

int  vgaPmGetCenterOfStr( char *string );

void vgaPmSetPos(int x, int y);
void vgaPmGetPos(int *x, int *y);

void vgaPmSetForeColor(int color);
int  vgaPmGetForeColor(void);

void vgaPmPrintfxy(int x, int y, char *format , ... );
void vgaPmPrintf(char *format , ... );

#endif
