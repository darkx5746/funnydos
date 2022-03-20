#include <stdio.h>
#include <dos.h>

#include "key.h"

int flag;

void keyevent(const int scancode)
{
   int scankey = (scancode & 0x7f) + 256;
   printf("0x%2x : %3d \n", scancode, scancode);

//   if (scankey==_KEY_Esc || scankey== _Esc ) flag=0;
   if (scankey==_KEY_Esc) flag=0;
}

void main(void)
{
#define NOP_OP
    flag=1;

    setKbd(keyevent);

    while(flag)
	NOP_OP;

    restoreKbd();
}