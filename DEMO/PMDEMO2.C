#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "vgapm.h"

#define swap(a,b) (a)^=(b)^=(a)^=(b)

void vgaPmLine(int x1,int y1,int x2,int y2,byte col)
{
	int i;

	if (x1==x2)
	{
		vgaPmVLine(x1,y1,y2,col);
		return;
	}
	if (y1==y2)
	{
		vgaPmHLine(x1,y1,x2,col);
		return;
	}

    if (x1>x2) swap(x1,x2);
	if (y1>y2) swap(y1,y2);

	if (x2-x1 > y2-y1)
	{
		for (i=x1;i<=x2;i++)
			vgaPmPutPixel(i,(y2-y1)*i/(x2-x1)+y1,col);
	}
	else
	{
		for (i=y1;i<=y2;i++)
			vgaPmPutPixel((x2-x1)*i/(y2-y1)+x1,i,col);
	}
}

void putPixelDemo(void)
{
	unsigned int i,x,y,col;

	vgaPmSetGraphMode();

//	for(i=0;i!=65535;i++)
	while(!kbhit())
	{
		x=rand() % 320;
		y=rand() % 200;
		col=rand() % 256;
		vgaPmPutPixel(x,y,col);
	}

	getch();
}

void BoxDemo(void)
{
	unsigned int i,x1,y1,x2,y2,col;

	vgaPmSetGraphMode();

//	for(i=0;i<4192;i++)
	while(!kbhit())
	{
		x1=rand() % 320;
		y1=rand() % 200;
		x2=rand() % 320;
		y2=rand() % 200;
		if (x1>x2) swap(x1,x2);
		if (y1>y2) swap(y1,y2);
		col=rand() % 256;
		vgaPmBox(x1,y1,x2,y2,col);
	}

	getch();
}

void BoxFillDemo(void)
{
	unsigned int i,x1,y1,x2,y2,col;

	vgaPmSetGraphMode();

//	for(i=0;i<2048;i++)
	while(!kbhit())
	{
		x1=rand() % 320;
		y1=rand() % 200;
		x2=rand() % 320;
		y2=rand() % 200;
		if (x1>x2) swap(x1,x2);
		if (y1>y2) swap(y1,y2);
		col=rand() % 256;
		vgaPmBoxFill(x1,y1,x2,y2,col);
	}

	getch();
}

void CircleDemo(void)
{
	unsigned int i,x,y,r,col;

	vgaPmSetGraphMode();

//	for(i=0;i<2048;i++)
	while(!kbhit())
	{
		x=rand() % 320;
		y=rand() % 200;
		r=rand() % 60;
		col=rand() % 256;
		vgaPmCircle(x,y,r,col);
	}

	getch();
}

void PaintDemo(void)
{
	unsigned int i,x,y,r,col,x1,x2,y1,y2;

	vgaPmSetGraphMode();

//	for(i=0;i<16;i++)
#if 0
	while(!kbhit())
	{
		x1=rand() % 80;
		y1=rand() % 50;
		x2=rand() % 80;
		y2=rand() % 50;
		x1*=4;		y1*=4;
		x2*=4;		y2*=4;
		if (x1>x2) swap(x1,x2);
		if (y1>y2) swap(y1,y2);
		col=rand() % 256;
		for (x1=0;x1<100; x1++, col=rand() % 256)
			vgaPmLine(x1,x1,x1+10,x1+20,col);
			vgaPmLine(x1,200,0,y2,col);
			vgaPmLine(x1,0,320,y2,col);
			vgaPmLine(320,y1,x1,200,col);
			col=rand() % 256;
			vgaPmPaint((x1+x2)/2,(y1+y2)/2,col);
	}
#endif

//	for(i=0;i<20;i++)
	while(!kbhit())
	{
		x=rand() % 320;
		y=rand() % 190+5;
		col=rand() % 256;
		vgaPmPaint(x,y,col);
	}

	getch();
}

void main(void)
{
	randomize();
//	vgaPmSetGraphMode();
	putPixelDemo();
	BoxDemo();
	BoxFillDemo();
	CircleDemo();
	PaintDemo();
	vgaPmEndGraphMode();
}
