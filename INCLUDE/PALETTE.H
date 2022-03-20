#ifndef __PALETTE__
#define __PALETTE__

#include "common.h"

void vgaCheckVSync(void);

void vgaSetOnePalette(byte i, byte r, byte g, byte b);
void vgaGetOnePalette(byte i, byte *r,byte *g, byte *b);

int  vgaSetAllPalette(byte *palette);
int  vgaGetAllPalette(byte *palette);

int  vgaSetNPalette( int start, int num, byte *palette);

void vgaDecPalette(byte *palette, int start, int step);
void vgaIncPalette(byte *palette, int start, int step);
int  vgaFadeInPalette(byte *palette, int start);
int  vgaFadeOutPalette(byte *palette, int start);
int  vgaLightInPalette(byte *palette, int start);
int  vgaLightOutPalette(byte *palette, int start);
int  vgaRotatePalette( byte *palette, int begin, int end, int count );

#endif
