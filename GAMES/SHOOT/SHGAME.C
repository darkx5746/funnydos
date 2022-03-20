#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>

#include "vgaspr.h"
#include "pmpcx.h"
#include "pmhan.h"
#include "timer.h"
#include "palette.h"
#include "key.h"
#include "aeffect.h"

#include "shgame.h"

/* function prototype */

int shoot(int command);
int shield(int command);
int option(int command);

/* image buffer */

byte *shipImg[9];
byte *miniNum[11];
byte *enemyImg[255];
byte *optionImg;
byte *shieldImg;
byte *shootImg[5];
byte *boomImg[3];
byte *itemImg[3];
byte *panelImg[12];
byte *eShootImg[3];
byte *bossShotImg;

/* ----------------------------------- */

/* palette scrolling */

void scroll(void)
{
    static i = STAR_COLOR_START;
    static unsigned long starTime;

#define STAR_SPEED  20

	if (getClock() - starTime < STAR_SPEED)
        return;
    else
		starTime = getClock();

	vgaSetOnePalette(i, 0, 0, 15);
    i++;

    if (i > STAR_COLOR_END)
        i = STAR_COLOR_START;

    vgaSetOnePalette(i, 63, 63, 63);
}

/* ship move function */

int ship(int command)
{
    static xloc, yloc;
    byte *temp;
    static shipSpeed;

	static unsigned long shipTime;

    int i, dir = CENTER;

#define SHIP_SPEED  10

    if (command == INIT)
    {
        shipSpeed = 1;
        xloc = 10;
        yloc = 100;
		shipTime = getClock();
        vgaSprSetBuf(SHIP_SPR, shipImg[CENTER]);
        vgaSprPut(SHIP_SPR, xloc, yloc);

        option(INIT);
        shield(INIT);
        powerUp(INIT);
        shoot(INIT);
    }

    if (command == OTHER)
        shipSpeed++;
    if (command == OTHER + 1)
        return shipSpeed;

	if (getClock() - shipTime < SHIP_SPEED)
        return false;
    else
		shipTime = getClock();

    if (keyQue[keyLeft])
    {
        xloc -= shipSpeed;
        dir = LEFT;
    }
    if (keyQue[keyRight])
    {
        xloc += shipSpeed;
		dir = RIGHT;
    }
    if (keyQue[keyUp])
    {
        yloc -= shipSpeed;
        dir = UP;
    }
    if (keyQue[keyDown])
    {
        yloc += shipSpeed;
        dir = DOWN;
    }
    if (keyQue[keyShoot])
    {
        for (i = 0; i <= option(OTHER); i++)
            shoot(SET + i);
        keyQue[keyShoot] = false;
    }
    if (keyQue[keyPowerUp])
    {
        keyQue[keyPowerUp] = false;
        powerUp(RUN);
    }
    if (keyQue[keyUp]&&keyQue[keyDown]&&keyQue[keyRight]&&keyQue[keyLeft])
    {
        restoreKbd();
        getch();
        getch();
        setKbd(keyEvent);
        keyQueClear();
    }

    xloc = max(0, xloc);
    yloc = max(0, yloc);
    xloc = min(320 - vgaSprGetXsize(SHIP_SPR), xloc);
    yloc = min(200 - vgaSprGetYsize(SHIP_SPR), yloc);

    switch (dir)
    {
        case CENTER:
            temp = shipImg[CENTER];
            break;
		case UP:
            temp = shipImg[UP];
            break;
        case DOWN:
            temp = shipImg[DOWN];
            break;
        case LEFT:
            temp = shipImg[LEFT];
            break;
        case RIGHT:
            temp = shipImg[RIGHT];
            break;
    }


    if (vgaSprGetXloc(SHIP_SPR) == xloc &&
        vgaSprGetYloc(SHIP_SPR) == yloc &&
        vgaSprGetBuf(SHIP_SPR) == temp)
        return true;

    vgaSprSetBuf(SHIP_SPR, temp);
    vgaSprSetCrushArea(SHIP_SPR, 1, 4, 19, 6);
    vgaSprPut(SHIP_SPR, xloc, yloc);

    for (i = 0; i < ITEM_SPR_MAX; i++)
    {
        if (vgaSprCheckCrush(SHIP_SPR, i + ITEM_SPR))
        {
            item(UNSET, &i);
            powerUp(SET);
        }
    }

    shield(RUN);
	option(RUN);

    return true;
}

/* dealing shoot function */

int shoot(int command)
{
    static shootMax[shootTypeMax] = {9, 4, 4};
    static shootTime[shootTypeMax] = {20, 20, 25};
    static shootStep[shootTypeMax] = {10, 20, 12};
    static shootState[shootTypeMax] = {1, 4, 1};
    static rippleTime[2] = {100, 200};

    static type;
    static originSpr;

    typedef struct
    {
        int time, startTime, state, type, xloc, yloc;
    }   shootType;

    static shootType s[SHOOT_SPR_MAX];

    int i, j;

    if (command == INIT)
    {
        type = NORMAL;

        for (i = 0; i < SHOOT_SPR_MAX; i++)
        {
            vgaSprHide(i + SHOOT_SPR);
			s[i].time = OFF;
        }

        return true;
    }

    if (SET <= command && command < UNSET)
    {
        switch (command - SET)
        {
            case 0:
                originSpr = SHIP_SPR;
                break;
            case 1:
                originSpr = OPTION_SPR;
                break;
            case 2:
                originSpr = OPTION_SPR + 1;
                break;
        }
        for (i = 0; i < shootMax[type]; i++)
        {
            if (s[i].time == OFF)
            {
                vgaSprSetBuf(SHOOT_SPR + i, shootImg[type]);
				s[i].time = s[i].startTime = (int) getClock();
                s[i].type = type;
                s[i].state = shootState[type];
                s[i].xloc = vgaSprGetXloc(originSpr) +
                    vgaSprGetXsize(originSpr);
                s[i].yloc = vgaSprGetYloc(originSpr) +
                    vgaSprGetYsize(originSpr) / 2 -
                    vgaSprGetYsize(SHOOT_SPR + i) / 2;
                break;
			}
        }
        return true;
    }

    if (command == RUN)
    {
        for (i = 0; i < SHOOT_SPR_MAX; i++)
        {
            if (s[i].time == OFF)
                continue;

			if ((int) getClock() - s[i].time > shootTime[type])
            {
				s[i].time = (int) getClock();
                s[i].xloc += shootStep[type];

                switch (s[i].type)
                {
                    case NORMAL:
                        vgaSprPut(SHOOT_SPR + i, s[i].xloc, s[i].yloc);
                        break;
                    case LASER:
                        vgaSprPut(SHOOT_SPR + i, s[i].xloc, s[i].yloc);
						break;
					case RIPPLE:
						if ((int) getClock() - s[i].startTime > rippleTime[0] &&
//							(int) getClock() - s[i].startTime <= rippleTime[1] &&
						vgaSprGetBuf(i + SHOOT_SPR) != shootImg[RIPPLE + 1])
						{
							s[i].yloc = vgaSprGetYloc(SHOOT_SPR + i) +
								vgaSprGetYsize(SHOOT_SPR + i) / 2;
							vgaSprSetBuf(i + SHOOT_SPR, shootImg[RIPPLE + 1]);
							s[i].yloc -= vgaSprGetYsize(SHOOT_SPR + i) / 2;
						}
/*						else if ((int) getClock() - s[i].startTime > rippleTime[1] &&
						vgaSprGetBuf(i + SHOOT_SPR) != shootImg[RIPPLE + 2])
						{
							s[i].yloc = vgaSprGetYloc(SHOOT_SPR + i) +
								vgaSprGetYsize(SHOOT_SPR + i) / 2;
							vgaSprSetBuf(i + SHOOT_SPR, shootImg[RIPPLE + 2]);
							s[i].yloc -= vgaSprGetYsize(SHOOT_SPR + i) / 2;
						}
*/
                        vgaSprPut(SHOOT_SPR + i, s[i].xloc, s[i].yloc);
                        break;
                    default:
                        break;
                }
                if (s[i].xloc > VGA256XSIZE)
                    shoot(UNSET + i);
                else
                {
                    for (j = 0; j < ENEMY_SPR_MAX; j++)
                    {
                        if (vgaSprCheckCrush(SHOOT_SPR + i, j + ENEMY_SPR))
                        {
                            shoot(UNSET + i);
                            enemy(UNSET, &j);
                        }
                    }
                    if (vgaSprCheckCrush(SHOOT_SPR + i, BOSS_ATTACK))
                    {
                        boss(UNSET);
                        shoot(UNSET + i);
                    }
                    else if (vgaSprCheckCrush(SHOOT_SPR + i, BOSS_NORMAL))
                    {
						shoot(UNSET + i);
                    }
                }
            }
        }

    }
    if (command >= UNSET && command < OTHER)
    {
        i = command - UNSET;
        if (s[i].time != OFF)
        {
            s[i].state--;
            if (!s[i].state)
            {
                vgaSprHide(SHOOT_SPR + i);
                s[i].time = OFF;
            }
        }
        return true;
    }


    if (command >= OTHER)
    {
        switch (command - OTHER)
        {
            case 0:
                type = LASER;
                break;
            case 1:
                type = RIPPLE;
                break;
            case 2:
				return type;
        }
    }
    return true;
}

int option(int command)
{
    static Point optionLocQue[0x40];
    static int quePos;
    static int temp;
    static optionNum;

    int i;
    Point loc;

#define OPTION_DELAY 27

    switch (command)
    {
        case INIT:
            quePos = 0;
            optionNum = 0;
            temp = 0;
            for (i = 0; i < OPTION_SPR_MAX; i++)
                if (vgaSprIsDisplayed(OPTION_SPR + i, activePage))
                    vgaSprHide(OPTION_SPR + i);
            break;
        case SET:
            if (optionNum >= OPTION_SPR_MAX - 1)
                return false;
            else
                vgaSprSetBuf(OPTION_SPR + optionNum, optionImg);
            vgaSprPut(OPTION_SPR + optionNum,
					  vgaSprGetXloc(SHIP_SPR),
                      vgaSprGetYloc(SHIP_SPR));
            optionNum++;
        case RUN:
            optionLocQue[quePos].x = vgaSprGetXloc(SHIP_SPR);
            optionLocQue[quePos].y = vgaSprGetYloc(SHIP_SPR);
            quePos++;
            quePos &= 0x3f;
            if (!optionNum || temp < OPTION_DELAY * 2)
            {
                temp++;
                return false;
            }
            for (i = 0; i < optionNum; i++)
            {
                loc = optionLocQue
                    [(quePos - (OPTION_DELAY / ship(OTHER + 1)) * (i + 1)) & 0x3f];
                vgaSprPut(OPTION_SPR + i, loc.x, loc.y);
            }
            break;
        case OTHER:
            return optionNum;
    defalut:
            return false;
    }
    return true;
}

int shield(int command)
{
    static xloc, yloc, set;

#define SHIP_GAP 3

	switch (command)
    {
        case INIT:
            set = OFF;
            vgaSprHide(SHIELD_SPR);
            return true;
        case SET:
            if (set)
                return false;
            set = 10;
            vgaSprSetBuf(SHIELD_SPR, shieldImg);
        case RUN:
            if (!set)
                return false;
            xloc = vgaSprGetXloc(SHIP_SPR) +
                vgaSprGetXsize(SHIP_SPR) + SHIP_GAP;
            yloc = vgaSprGetYloc(SHIP_SPR) +
                vgaSprGetYsize(SHIP_SPR) / 2 -
                vgaSprGetYsize(SHIELD_SPR) / 2;
            vgaSprPut(SHIELD_SPR, xloc, yloc);
            break;
        case UNSET:
            set--;
            if (!set)
            {
                vgaSprHide(SHIELD_SPR);
                powerUp(OTHER);
            }
        default:
            return true;
    }
    return true;
}

void enemy(int command, int *arg)
{
    typedef struct
    {
        int ptn, state, pos;
    }   enemyType;

    static enemyType e[ENEMY_SPR_MAX];

    int i, j, k, jj, kk, jk[4];

    switch (command)
    {
        case INIT:
            for (i = ENEMY_SPR, j = 0; i < ENEMY_SPR_END; i++, j++)
            {
                if (vgaSprIsDefined(i))
                    vgaSprFree(i);
                vgaSprSet(i, OUTBUFFER);
                e[j].state = OFF;
                e[j].pos = 0;
            }
            return;
        case SET:
            for (i = 0; i < ENEMY_SPR_MAX; i++)
            {
                if (e[i].state)
                    continue;

                e[i].ptn = arg[0] - 1;
                e[i].state = enemyPtn[e[i].ptn][e[i].pos++];
                j = enemyPtn[e[i].ptn][e[i].pos++];
                vgaSprSetBuf(i + ENEMY_SPR, enemyImg[j]);
                j = enemyPtn[e[i].ptn][e[i].pos++];
				k = enemyPtn[e[i].ptn][e[i].pos++];
                vgaSprPut(i + ENEMY_SPR, j, k);
                return;
            }
            return;
        case UNSET:
            if (e[*arg].state == INVINCIBLE)
                return;
            else if (e[*arg].state == END)
            {
                vgaSprHide(arg[0] + ENEMY_SPR);
				e[*arg].state = OFF;
				e[*arg].pos = 0;
				return;
			}
			else
				e[*arg].state--;
			if (!(e[*arg].state & 0x1f))
			{
				jk[0] = vgaSprGetXloc(arg[0] + ENEMY_SPR);
				jk[1] = vgaSprGetYloc(arg[0] + ENEMY_SPR);
				object(SET, jk);
				if (e[*arg].state & 0x80)
					item(SET, jk);
				vgaSprHide(arg[0] + ENEMY_SPR);
				score += 10;
				adlibSound(0);
				putMiniNum(260, 10, score);
				e[*arg].state = OFF;
				e[*arg].pos = 0;
            }
            return;
        case RUN:
			for (i = ENEMY_SPR, j = 0; i < ENEMY_SPR_END; i++, j++)
			{
				if (!e[j].state)
					continue;

				switch (jj = enemyPtn[e[j].ptn][e[j].pos++])
				{
					case MOVE:
						jj = enemyPtn[e[j].ptn][e[j].pos++];
						vgaSprSetBuf(i, enemyImg[jj]);
						jj = enemyPtn[e[j].ptn][e[j].pos++];
						kk = enemyPtn[e[j].ptn][e[j].pos++];
						jj += vgaSprGetXloc(i);
						kk += vgaSprGetYloc(i);
						vgaSprPut(i, jj, kk);
					case SKIP:
						break;
					case SHOOT:
                        jj = enemyPtn[e[j].ptn][e[j].pos++];
                        vgaSprSetBuf(i, enemyImg[jj]);
                        jk[0] = enemyPtn[e[j].ptn][e[j].pos++];
                        jk[1] = enemyPtn[e[j].ptn][e[j].pos++];
                        jk[2] = vgaSprGetXloc(i);
                        jk[3] = vgaSprGetYloc(i);
						jk[4] = NORMAL;
						vgaSprPut(i, jk[2], jk[3]);
                        eShoot(SET, jk);
                        i--;
                        j--;
						continue;
                    case JUMP:
                        if (rand() % 100 < enemyPtn[e[j].ptn][e[j].pos++])
							e[j].pos = enemyPtn[e[j].ptn][e[j].pos++];
						else e[j].pos++;
						i--;
						j--;
						continue;
					case END:
						e[j].state = END;
						enemy(UNSET, &j);
						break;
				}
				if (vgaSprCheckCrush(SHIELD_SPR, i))
				{
					shield(UNSET);
					enemy(UNSET, &j);
				}
				else if (vgaSprCheckCrush(SHIP_SPR, i))
				{
					if (_super)
						enemy(UNSET, &j);
					else
						gameState = BOOM;
				}
            }
            break;
        case OTHER:

        default:
            errExit("enemy pattern file damaged .. \n");
            break;
    }
}

void object(int command, int *arg)
{
    static int objState[ENEMY_SPR_MAX];
    static int objectTime;

    int i, j;

#define OBJECT_SPEED 100

    switch (command)
    {
        case INIT:
			objectTime = (int) getClock();
            for (i = OBJECT_SPR, j = 0; i < OBJECT_SPR_END; i++, j++)
            {
                if (vgaSprIsDefined(i))
                    vgaSprFree(i);
                vgaSprSet(i, OUTBUFFER);
                objState[j] = OFF;
            }
            return;
        case SET:
            for (i = 0; i < OBJECT_SPR_MAX; i++)
            {
                if (objState[i])
                    continue;
                objState[i] = 3;
                vgaSprSetBuf(i + OBJECT_SPR, boomImg[0]);
                vgaSprPut(i + OBJECT_SPR, arg[0], arg[1]);
                return;
            }
            return;
        case RUN:
			if ((int) getClock() - objectTime < OBJECT_SPEED)
				return;
            else
				objectTime = (int) getClock();

            for (i = 0; i < OBJECT_SPR_MAX; i++)
            {
                if (!objState[i])
                    continue;
                objState[i]--;
                if (!objState[i])
                {
                    vgaSprHide(i + OBJECT_SPR);
                    continue;
                }

                if (vgaSprGetBuf(i + OBJECT_SPR) == boomImg[2 - objState[i]])
                    continue;
                else
                    vgaSprSetBuf(i + OBJECT_SPR, boomImg[2 - objState[i]]);
                vgaSprPut(i + OBJECT_SPR,
                          vgaSprGetXloc(i + OBJECT_SPR),
                          vgaSprGetYloc(i + OBJECT_SPR));
            }
            return;
        case OTHER:
        default:
            break;
    }
}

void eShoot(int command, int *arg)
{
    Point eShootAngle[16] = {
		{-12, 0}, {-11, -5}, {-9, -9}, {-5, -11},
		{0, -12}, {5, -11}, {9, -9}, {11, -5},
		{12, 0}, {11, 5}, {9, 9}, {5, 11},
		{0, 12}, {-5, 11}, {-9, 9}, {-11, 5}
    };
	static eShootTime[3] = {50, 30, 40};

	typedef struct
	{
		int time, startTime, angle, xloc, yloc, type;
	}   eShootType;

	static eShootType es[ESHOOT_SPR_MAX];

	int i, j;

	switch (command)
	{
		case INIT:
			for (i = ESHOOT_SPR, j = 0; i < ESHOOT_SPR_END; i++, j++)
			{
				if (vgaSprIsDefined(i))
					vgaSprFree(i);
				vgaSprSet(i, OUTBUFFER);
				es[j].time = OFF;
            }
            return;
        case SET:
            if (rand() % 100 < arg[0])
                for (i = 0; i < ESHOOT_SPR_MAX; i++)
                {
                    if (es[i].time != OFF)
                        continue;
                    es[i].angle = arg[1];
					vgaSprSetBuf(ESHOOT_SPR + i, eShootImg[arg[4]]);
					es[i].time = es[i].startTime = (int) getClock();
					es[i].xloc = arg[2];
					es[i].yloc = arg[3];
					es[i].type = arg[4];
					break;
				}
			return;
		case RUN:
			for (i = 0; i < ESHOOT_SPR_MAX; i++)
			{
				if (es[i].time == OFF)
					continue;

				if ((int) getClock() - es[i].time > eShootTime[es[i].type])
				{
					es[i].time = (int) getClock();
					switch (es[i].type)
					{
						case NORMAL :
							es[i].xloc += eShootAngle[es[i].angle].x;
							es[i].yloc += eShootAngle[es[i].angle].y;
							break;
						case LASER :
							es[i].xloc -= 25;
							break;
					}

					vgaSprPut(ESHOOT_SPR + i, es[i].xloc, es[i].yloc);
				}
				if (es[i].xloc < -vgaSprGetXsize(i+ESHOOT_SPR) || es[i].xloc > 320 ||
					es[i].yloc < -vgaSprGetYsize(i+ESHOOT_SPR) || es[i].yloc > 200)
					//((int) getClock() - es[i].startTime > 2000)
					{
						es[i].type = NORMAL;
						eShoot(UNSET, &i);
					}
				else if (vgaSprCheckCrush(SHIELD_SPR, i + ESHOOT_SPR))
                {
                    shield(UNSET);
                    eShoot(UNSET, &i);
                }
				else if (vgaSprCheckCrush(SHIP_SPR, i + ESHOOT_SPR))
				{
					if (_super)
						eShoot(UNSET, &i);
					else
						gameState = BOOM;
				}
			}
            return;
        case UNSET:
            i = ESHOOT_SPR + arg[0];
			if (es[arg[0]].time != OFF && es[arg[0]].type != LASER)
			{
                if (vgaSprIsDisplayed(i, activePage))
                    vgaSprHide(i);
                es[arg[0]].time = OFF;
            }
            return;
    }
}

void item(int command, int *arg)
{
    static it[ITEM_SPR_MAX];

    int i, j;

    switch (command)
    {
        case INIT:
            for (i = ITEM_SPR, j = 0; i < ITEM_SPR_END; i++, j++)
            {
                if (vgaSprIsDefined(i))
                    vgaSprFree(i);
                vgaSprSet(i, OUTBUFFER);
                it[j] = OFF;
            }
            return;
        case SET:
            for (i = ITEM_SPR, j = 0; i < ITEM_SPR_END; i++, j++)
			{
                if (it[j] != OFF)
                    continue;
                else
                    it[j] = 1;
                vgaSprSet(i, itemImg[0]);
				vgaSprPut(i, arg[0], arg[1]);
				break;
			}
			return;
		case RUN:
			for (i = ITEM_SPR, j = 0; i < ITEM_SPR_END; i++, j++)
			{
				if (it[j] == OFF)
					continue;
				else
					it[j] = ((it[j] + 1) % 3) + 1;
				vgaSprSetBuf(i, itemImg[it[j] - 1]);
				vgaSprPut(i, vgaSprGetXloc(i) - 4, vgaSprGetYloc(i));
				if (vgaSprGetXloc(i) < 0)
					item(UNSET, &j);
				else if (vgaSprCheckCrush(SHIP_SPR, i))
				{
					item(UNSET, &j);
					powerUp(SET);
				}
			}
			return;
		case UNSET:
			i = arg[0];
			it[i] = OFF;
			vgaSprFree(i + ITEM_SPR);
			return;
	}
}

int powerUp(int command)
{
    static state;
    static panel[5];

#define L 155

    static xloc = 280, yloc[5] = {4 + L, 12 + L, 19 + L, 26 + L, 33 + L};

    int i;

    switch (command)
    {
        case INIT:
            state = -1;
            panel[0] = 3;
            panel[1] = 1;
            panel[2] = 1;
            panel[3] = 2;
            panel[4] = 1;
            break;
        case RUN:
            if (state >= 0)
            {
                if (panel[state])
                {
                    panel[state]--;
                    switch (state)
                    {
                        case 0:
                            ship(OTHER);
                            break;
						case 1:
                            panel[2] = 1;
                            shoot(OTHER);
                            break;
                        case 2:
                            panel[1] = 1;
                            shoot(OTHER + 1);
                            break;
                        case 3:
                            option(SET);
                            break;
                        case 4:
                            shield(SET);
                            break;
                    }
                    state = -1;
                }
            }
            break;
        case SET:
            state++;
            state %= 5;
            break;
        case OTHER:
            panel[4] = 1;
        default:
            break;
    }

    for (i = 0; i < 5; i++)
    {
        vgaPmPutImage(xloc, yloc[i], panelImg[i]);
        if (!panel[i])
            vgaPmPutImage(xloc, yloc[i] + (!i), panelImg[10]);
	}

    i = state;
    if (panel[i])
        vgaPmPutImage(xloc, yloc[i], panelImg[i + 5]);
    else
        vgaPmPutImage(xloc, yloc[i] + (!i), panelImg[11]);

    vgaPmPageCopy(BACKGROUNDPAGE, xloc, yloc[0],
                  activePage, xloc, yloc[0], 24, 37);
    return state;
}

void boss(int command)
{
    static int pos;
    static int state;

    int jj, kk, jk[10];

#define CORE_PAL (STAR_COLOR_END+1)

    switch (command)
    {

        case INIT:
            if (vgaSprIsDefined(BOSS_NORMAL))
                vgaSprFree(BOSS_NORMAL);
            if (vgaSprIsDefined(BOSS_ATTACK))
                vgaSprFree(BOSS_ATTACK);
            pos = 0;
            return;

        case SET:
			pos = 0;
            state = bossPtn[pos++];
            vgaSprSet(BOSS_ATTACK, enemyImg[bossPtn[pos++]]);
            vgaSprSet(BOSS_NORMAL, OUTBUFFER);
            vgaSprSetBuf(BOSS_NORMAL, enemyImg[bossPtn[pos++]]);
            vgaSetOnePalette(CORE_PAL, 0, 0, 63);
            jj = bossPtn[pos++];
            kk = bossPtn[pos++];
            vgaSprPut(BOSS_NORMAL, jj, kk);
            vgaSprPut(BOSS_ATTACK, jj, kk + bossPtn[pos++]);
        case RUN:
            for (;;)
            {
                switch (bossPtn[pos++])
                {
                    case SKIP:
                        return;
                    case MOVE:
                        jj = bossPtn[pos++];
                        vgaSprSetBuf(BOSS_NORMAL, enemyImg[jj]);
                        jj = bossPtn[pos++];
                        kk = bossPtn[pos++];
                        vgaSprPut(BOSS_NORMAL,
                                  jj + vgaSprGetXloc(BOSS_NORMAL),
                                  kk + vgaSprGetYloc(BOSS_NORMAL));
                        vgaSprPut(BOSS_ATTACK,
                                  jj + vgaSprGetXloc(BOSS_ATTACK),
                                  kk + vgaSprGetYloc(BOSS_ATTACK));
                        break;
                    case SHOOT:
                        jj = bossPtn[pos++];
						jk[0] = bossPtn[pos++];
						jk[1] = bossPtn[pos++];
						jk[2] = vgaSprGetXloc(BOSS_NORMAL);
						jk[3] = vgaSprGetYloc(BOSS_NORMAL);
						if (enemyImg[jj] != vgaSprGetBuf(BOSS_NORMAL) )
						{
							vgaSprSetBuf(BOSS_NORMAL, enemyImg[jj]);
							vgaSprPut(BOSS_NORMAL, jk[2], jk[3]);
						}
						jk[3] += vgaSprGetYsize(BOSS_NORMAL) / 2;
						jk[4] = 0;
						eShoot(SET, jk);
						continue;
					case BOSSSHOOT :
						jj = bossPtn[pos++];
						jk[0] = 100;
						jk[1] = 0;
						jk[2] = vgaSprGetXloc(BOSS_NORMAL);
						jk[3] = vgaSprGetYloc(BOSS_NORMAL);
						if (enemyImg[jj] != vgaSprGetBuf(BOSS_NORMAL) )
						{
							vgaSprSetBuf(BOSS_NORMAL, enemyImg[jj]);
							vgaSprPut(BOSS_NORMAL, jk[2], jk[3]);
						}
						jk[2]+=bossPtn[pos++];
						jk[3]+=bossPtn[pos++];
						jk[4] = LASER;
						eShoot(SET, jk);
						continue;
					case ENEMY:
						jk[0] = bossPtn[pos++];
						enemy(SET, jk);
						continue;
					case JUMP:
						if (rand() % 100 < bossPtn[pos++])
							pos = bossPtn[pos++];
						else pos++;
                        continue;
                    case END:
                        pos = 6;
                        continue;
                    case OTHER:
                    default:
                        errExit("Boss pattern file damaged .. \n");
                        break;
                }
				if (vgaSprCheckCrush(SHIP_SPR, BOSS_NORMAL))
				{
					if (!_super)
						gameState = BOOM;
				}
                else
                    break;
            }
            return;

        case UNSET:
            state--;
			score += 10;
			adlibSound(0);
			if (state)
            {
                score += 150;
                if (state < bossPtn[0] / 3)
					vgaSetOnePalette(CORE_PAL, 63, 0, 0);
				else if (state < bossPtn[0] * 2 / 3)
					vgaSetOnePalette(CORE_PAL, 32, 0, 32);
			}
            else
            {
                gameState = STAGECLEAR;
            }
            return;
    }
}
