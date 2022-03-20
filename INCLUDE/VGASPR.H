#ifndef __SPRITE__
#define __SPRITE__

#include "vgapm.h"

#define VRAMSPRITE          NULL
#define OUTBUFFER           VRAMSPRITE
#define MAXSPRITE           0x200
#define BACKGROUNDPAGE      0x02
#define SPRITEPAGE          0x03
#undef  PAGE2
#undef  PAGE3

#include "boxlib.h"

extern byte near invisibleColorEnabled;
extern byte near detailCrushCheckEnabled;

typedef struct {
	byte defined;
	byte displayed;
					/* if sprite is displayed set page(0-3) bit */
	byte *buffer;   /* sprite's buffer pointer
					   include sprite's size infomation */
	int xsize,ysize;
	Box  vram ;     /* if video ram sprite use this */
	Box  cArea ;    /* crush check area */
	byte crushed;   /* last displayed crush check bit */
					/* sprite's displayed position is saving this variable */
	Box img_area[VGAPM_MAXPAGE];
}   Spr_Attr;

extern Spr_Attr sprAttrBuf[MAXSPRITE];

word vgaPmPutImageClip(int x, int y,byte *buffer,
				   int cxs,int cys,int cxe,int cye);

int  vgaSprInit(void);
int  vgaSprSet(int id,byte *buffer);
void vgaSprFree(int id);
int  vgaSprPut(int id,int x,int y);
int  vgaSprRePutAllPage(int id);
int  vgaSprRefreshArea(int x,int y,int xe, int ye);
void vgaSprPageHide(int id,int page);
void vgaSprHide(int id);
void vgaSprSetCrushArea(int id,int x,int y,int xe,int ye);
int  vgaSprCheckCrushNum(int id1,int id_start,int id_end);
int  vgaSprCheckCrush(int id1,int id2);
void vgaSprSetBuf(int id,byte *buf);
void vgaSprSetVRamBuf(int id,int x,int y,int xe,int ye);
void vgaSprClose(void);

/* sprite library macro function */

#define vgaSprIsDefined(id)	(sprAttrBuf[id].defined)
#define vgaSprIsDisplayed(id,page) (sprAttrBuf[id].displayed & (1 << (page)))
#define vgaSprGetXloc(id)   (sprAttrBuf[id].img_area[activePage].x)
#define vgaSprGetYloc(id)   (sprAttrBuf[id].img_area[activePage].y)
#define vgaSprIsCrushed(id) (sprAttrBuf[id].crushed)
#define vgaSprIsVRSprite(id) (!sprAttrBuf[id].buffer)
#define vgaSprGetBuf(id)     (sprAttrBuf[id].buffer)
#define vgaSprGetXsize(id)   (sprAttrBuf[id].xsize)
#define vgaSprGetYsize(id)   (sprAttrBuf[id].ysize)

#endif
