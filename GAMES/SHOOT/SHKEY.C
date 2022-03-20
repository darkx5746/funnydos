#include <stdio.h>
#include "common.h"

#include "key.h"

#include "shgame.h"

/* key interrupt */

int keyQue[keyMax];

void keyQueClear(void)
{
    keyQue[keyUp] = keyQue[keyDown] =
    keyQue[keyLeft] = keyQue[keyRight] =
    keyQue[keyShoot] = keyQue[keyPowerUp] = false;
}

void keyEvent(int key)
{
    static near scankey;
    static shootFlag = true;

	scankey = (byte) (key & 0x7f) + 256;

    if (scankey == _KEY_Esc)
        gameState = GAMEOVER;

    if (key & 0x80)
    {
        switch (scankey)
        {
            case _KEY_Up:
                keyQue[keyUp] = false;
                break;
            case _KEY_Down:
                keyQue[keyDown] = false;
                break;
            case _KEY_Left:
                keyQue[keyLeft] = false;
                break;
            case _KEY_Right:
                keyQue[keyRight] = false;
                break;
            case _KEY_Alt:
                shootFlag = true;
                keyQue[keyShoot] = false;
                break;
            case _KEY_Ctrl:
                keyQue[keyPowerUp] = false;
                break;
        }
    }
    else
    {
        switch (scankey)
        {
            case _KEY_Up:
                keyQue[keyUp] = true;
                break;
            case _KEY_Down:
                keyQue[keyDown] = true;
                break;
            case _KEY_Left:
                keyQue[keyLeft] = true;
                break;
            case _KEY_Right:
                keyQue[keyRight] = true;
                break;
            case _KEY_Alt:
                if (shootFlag)
                {
                    keyQue[keyShoot] = true;
                    shootFlag = false;
                }
                break;
            case _KEY_Ctrl:
                keyQue[keyPowerUp] = true;
                break;
        }
    }
}
