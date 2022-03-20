#ifndef __VGAPM__
#define __VGAPM__

#include "vgastd.h"

#define VGAPM_MAXPAGE	4
#define PAGE0           0
#define PAGE1           1
#define PAGE2           2
#define PAGE3           3

#define vgaPmGetActivePage() (activePage)
#define vgaPmSetActivePage(pagenum) (activePage = (pagenum))

extern byte near activePage;
extern word near vRamStartAddr ;
extern word near vRamAddrTable[4][200];
extern byte near planeModeSetted;
extern byte near isPlaneModeSet;

void vgaPmSetPlaneMode(void);
void vgaPmSetGraphMode(void);
void vgaPmEndGraphMode(void);

void vgaPmSetDispPage(byte pageNum);
byte vgaPmGetDispPage(void);
void vgaPmSetWriteMode(byte mode);
void vgaPmSetStartAddr(int _offset);
void vgaPmClearScr(int color);

void vgaPmPutPixel(int x, int y, byte color);
byte vgaPmGetPixel(int x, int y);

word vgaPmGetImage(int x, int y, int xe, int ye, byte *buffer);
word vgaPmPutImage(int x, int y, byte *buffer);
word vgaPmPutImageInviCol(int x, int y, byte *buffer);
void vgaPmFullPageCopy(byte targetPage,byte sourcePage);
void vgaPmVRamCopy(byte targetPage,int targetX,int targetY,
				   byte sourcePage,int sourceX,int sourceY,int xsize,int ysize);
void vgaPmPageCopy(byte targetPage,int targetX,int targetY,
				   byte sourcePage,int sourceX,int sourceY,int xsize,int ysize);
void vgaPmBoxFill(int xs , int ys , int xe , int ye , int color);
void vgaPmBox(int xs , int ys , int xe , int ye , int color);
void vgaPmVLine(int xs,int ys,int ye,int color);
void vgaPmHLine(int x,int y,int xsize,int color);
void vgaPmCircle(int xx,int yy,int radius,int color);
void vgaPmPaint(int x, int y, int col);

#endif
