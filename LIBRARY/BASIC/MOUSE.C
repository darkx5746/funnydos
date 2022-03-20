#include "mouse.h"

int initMouse( void )
{
    int i;

    asm {
        mov ax, 0
        int 0x33
        mov i, ax
    }
    return i;
}

void mouseOn( void )
{
    asm {
        mov ax, 1
        int 0x33
    }
}

void mouseOff( void )
{
    asm {
        mov ax, 2
        int 0x33
    }
}

void getMousePos( int *x, int *y )
{
    int tx, ty;

    asm {
        mov ax, 3
        int 0x33
        mov tx, cx
        mov ty, dx
    }
    *x = tx; *y = ty;
}

void setMousePos( int x, int y )
{
    asm {
        mov ax, 4
        mov cx, x
        mov dx, y
        int 0x33
    }
}

int getMouseLeftButton( void )
{
    int status;

    asm {
        mov ax, 5
        mov bx, 0
        int 0x33
        mov status, ax
    }
    return status & 1;
}

int getMouseRightButton( void )
{
    int status;

    asm {
        mov ax, 5
        mov bx, 1
        int 0x33
        mov status, ax
    }
    return status & 2;
}
