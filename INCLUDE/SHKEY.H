/* key que definition */

typedef enum
{
    keyLeft,
    keyRight,
    keyUp,
    keyDown,
    keyShoot,
    keyPowerUp,
    keyMax
}   keyDef;

extern int keyQue[keyMax];

void keyEvent(int key);
void keyQueClear(void);
