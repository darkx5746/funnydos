#include <stdio.h>

#include "vgastd.h"
#pragma inline

byte invisibleColor = 0x80;

void vgaStdGetImage( int xs, int ys, int xe, int ye, byte *buff )
{
	if ( buff == NULL )
	{
		setError("NULL pointer assigned in vgaStdGetImage()");
		return;
	}

	asm {
		push ds;
		push es;
		mov ax, ys;
		mov bx, VGA256XSIZE;
		mul bx;
		add ax, xs;
		mov si, ax;
		les di, buff;
		mov cx, xe;
		sub cx, xs;
		inc cx;
		mov dx, ye;
		sub dx, ys;
		inc dx;
		mov es:[di], cx;
		mov es:[di+2], dx;
		add di, 4;

		mov ax, VRAMSEGMENT;
		mov ds, ax;

		mov ax, cx;
	}
loop2:
	asm {
		push si;

		rep	movsb;
		dec	dx;
		jz 	end2;
		pop	si;
		add si, VGA256XSIZE;
		mov cx, ax;
		jmp	loop2;
	}
end2:
	asm {
		pop	si;
		pop es;
		pop ds;
	}
}

void vgaStdPutImage( int xs, int ys, byte  *buff )
{
	if ( buff == NULL )
	{
		setError("NULL pointer assigned in vgaStdPutImage()");
		return;
	}

	asm {
		push ds;
		push es;
		mov ax, ys;
		mov bx, VGA256XSIZE;
		mul bx;
		add ax, xs;
		mov di, ax;
		lds si, buff;
		mov cx, ds:[si];
		mov dx, ds:[si+2];
		add si, 4;

		mov ax, VRAMSEGMENT;
		mov es, ax;

		mov ax, cx;
	}
loop1:
	asm {
		push di;

		rep	movsb;
		dec	dx;
		jz  end1;
		pop	di;
		add di, VGA256XSIZE;
		mov cx, ax;
		jmp loop1;
	}
end1:
	asm {
		pop	di;
		pop es;
		pop ds;
	}
}

void vgaStdPutImageInviCol( int x, int y, byte *buffer )
{
    byte icol;

	icol = invisibleColor;

	if ( buffer == NULL )
	{
        setError("NULL pointer assigned in vgaStdPutImageInviCol()");
		return;
	}

	asm {
		push ds;
		push es;
		mov ax, y;
		mov bx, VGA256XSIZE;
		mul bx;
		add ax, x;
		mov di, ax;
		lds si, buffer;
		mov cx, ds:[si];
		mov dx, ds:[si+2];
		add si, 4;

		mov ax, VRAMSEGMENT;
		mov es, ax;

		mov ax, cx;
	}
loop1:
	asm {
		push di;
	}
loop2:
	asm {
		mov bh, ds:[si];
        cmp bh, icol;
		je skip;
		mov es:[di], bh;
	}
skip:
	asm {
		inc di;
		inc si;
		loop loop2;
	}
	asm {
		dec	dx;
		jz  end1;
		pop	di;
		add di, VGA256XSIZE;
		mov cx, ax;
		jmp loop1;
	}
end1:
	asm {
		pop	di;
		pop es;
		pop ds;
	}
}
