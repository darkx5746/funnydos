#ifndef __Box_LIB__
#define __Box_LIB__

#ifndef max(a,b)
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef swap(a,b)
#define swap(a,b) 	( (a)^=(b)^=(a)^=(b) )
#endif

typedef struct {
	int x,		y;
}       Point;

typedef struct {
	int x,		y,
		xsize,	ysize,
		xend, 	yend;
}       Box;

Box sizeBox(int x,int y,int xsize,int ysize);
Box endBox(int x,int y,int xend,int yend);
void resetBoxSize(Box*);
void resetBoxEnd(Box*);
Box unionBox ( Box b1 , Box b2 ) ;
Box crossBox ( Box b1 , Box b2 ) ;

#define box             endBox
#define resetBox        resetBoxSize
#define nullBox(box)    (!((box).xsize * (box).ysize))
#define box2int(b)		(b).x,(b).y,(b).xend,(b).yend

#endif
