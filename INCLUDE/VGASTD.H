#ifndef __VGASTD__
#define __VGASTD__

#include "common.h"

#define VGABIOSNUM		0x10
#define VGA256COL		0x13
#define VRAMSEGMENT		0xa000
#define VRAMADDRESS     ((byte far *)(0xa0000000L))
#define VGA256XSIZE     320
#define VGA256YSIZE     200

#define vgaStdSetGraphMode()    vgaSetMode(VGA256COL);
#define vgaStdEndGraphMode()    vgaSetMode(0x03);
#define vga256ImageSize(x1,y1,x2,y2)  (((x2)-(x1)+1)*((y2)-(y1)+1)+4)

extern byte invisibleColor;

int  vgaSetMode(int mode);
int  vgaGetMode(void);
void vgaCheckVSync( void );

void vgaStdClearScr(int color);
byte vgaStdGetPixel(int x,int y);
void vgaStdPutPixel(int x,int y,int color);
void vgaStdBox(int x1,int y1,int x2,int y2,int color);
void vgaStdBoxFill(int x1,int y1,int x2,int y2,int color);
void vgaStdCircle(int x,int y,int r,int color);
void vgaStdHLine(int x1,int y1,int x2,int);
void vgaStdVLine(int x1,int y1,int y2,int);
void vgaStdPaint(int x,int y,int color);

void vgaStdGetImage(int x1,int y1,int x2,int y2,byte *buffer);
void vgaStdPutImage(int x,int y,byte *buffer);
void vgaStdPutImageInviCol(int x,int y,byte *buffer);

/* STDHAN.C */
void vgaStdSetFont ( char *hanFontFile , char *engFontFile );
void vgaStdOpenHan ( char *hanFont , char *engFont );
void vgaStdPutsxy ( int x , int y , char *string );
int  vgaStdGetCenterOfStr ( char *string );
void vgaStdPutsxyC ( int y , char *string );
void vgaStdPuts ( char *string );
void vgaStdSetPos ( int x , int y );
void vgaStdGetPos ( int *x , int *y );
void vgaStdSetForeColor ( int color );
int  vgaStdGetForeColor ( void );
void vgaStdCloseHan ( void );

void vgaStdPrintfxy( int x, int y, char *fmt, ... );
void vgaStdPrintf( char *fmt, ... );

#endif
