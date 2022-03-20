#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <mem.h>

/* --------------------------------------------------------- */

#include "vgaspr.h"
#include "palette.h"
#include "pmpcx.h"
#include "pmhan.h"
#include "amdmusic.h"
#include "aeffect.h"

#include "shgame.h"

void shadowPutsxy(int x, int y, byte * str, int fore, int back)
{
    vgaPmSetForeColor(BLACK);
    vgaPmPutsxy(x - 1, y - 1, str);
    vgaPmPutsxy(x - 1, y, str);
    vgaPmPutsxy(x, y - 1, str);
    vgaPmPutsxy(x + 2, y + 2, str);
    vgaPmPutsxy(x + 2, y + 1, str);
    vgaPmPutsxy(x + 1, y + 2, str);

    vgaPmSetForeColor(back);
    vgaPmPutsxy(x + 1, y + 1, str);
    vgaPmSetForeColor(fore);
    vgaPmPrintfxy(x, y, str);
}

int imageGet(byte ** image, FILE * f)
{
    int sx, sy;
    unsigned int size;
    fpos_t fp;
    byte *temp;

    fgetpos(f, &fp);

    fread(&sx, sizeof(int), 1, f);
    fread(&sy, sizeof(int), 1, f);

    size = vga256ImageSize(1, 1, sx, sy);

    fsetpos(f, &fp);

    temp = (byte *) malloc(size);
    if (!(temp))
        errExit(ERR_MEMORY);

    if (!fread(temp, size, 1, f))
        errExit("file damaged .. ");

    if (image)
    {
        *image = (byte *) malloc(size);
        if (!(*image))
            errExit(ERR_MEMORY);

        memcpy(*image, temp, size);
    }
    free(temp);

    return true;
}

void putMiniNum(int x, int y, int score)
{
    vgaPmPutImage(x, y, miniNum[score / 10000]);
    x += 5;
    score %= 10000;
    vgaPmPutImage(x, y, miniNum[score / 1000]);
    x += 5;
    score %= 1000;
    vgaPmPutImage(x, y, miniNum[score / 100]);
    x += 5;
    score %= 100;
    vgaPmPutImage(x, y, miniNum[score / 10]);
    x += 5;
    score %= 10;
    vgaPmPutImage(x, y, miniNum[score]);
    vgaPmPageCopy(BACKGROUNDPAGE, x - 20, y,
                  activePage, x - 20, y, 25, 5);
}

void adlibSound( int soundNum )
{
	adlibEffectOff();
	adlibEffectOn( 50, &instrument->instrTable[soundTable[soundNum]] );
}
