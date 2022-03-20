#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>

#include "vgaspr.h"
#include "palette.h"
#include "pmpcx.h"
#include "pmhan.h"
#include "amdmusic.h"
#include "aeffect.h"
#include "key.h"
#include "timer.h"

#include "shgame.h"

/* stage map * pattern * music */

int stageMap[4096];
int bossPtn[1024];
int *enemyPtn['Z' - 'A' + 1];

AMD_Music *stageMusic[2];
AMD_Music *instrument;

int soundTable[10];

/* load image from file */

void loadSprite(int st)
{
    FILE *f;
    char *fname;
    int i, imgMax;

    fname = "stage!.map";
    fname[5] = st + '0';

    if (!(f = fopen(fname, "rt")))
        errExit("stage map file not found.");

    fscanf(f, "%d", &imgMax);
    fclose(f);

    fname = "stage!.img";
    fname[5] = st + '0';

    for (i = 0; i < 255; i++)
        enemyImg[i] = NULL;

    if (!(f = fopen(fname, "rb")))
        errExit("stage image file not found.");

    for (i = 0; i < imgMax; i++)
    {
        imageGet(&enemyImg[i], f);
    }

    fclose(f);
}

/* load music */

void loadMusic(int stage)
{
	static flag = false;
	char *fname = "stage!.amd";

	if (!flag)
	{
		flag = true;
		instrument = amdOpen("effect.amd",OFF);
		soundTable[0] = 7;
	}

	fname[5] = stage + '0';

	stageMusic[0] = amdOpen( fname, ON | (8 << 1) );
	setAdlibEffectVoice( 8 );
	amdSetVolume(70);
	setAdlibEffectVolume( 100 );
}

/* background drawing */

void loadBackGround(int stage)
{
	int i, j, k, step, crPage = activePage;
	Point p;

	char *temp = "!";

	vgaSetOnePalette(STAR_COLOR_START, 63, 63, 63);

	for (i = STAR_COLOR_START; i <= STAR_COLOR_END; i++)
		vgaSetOnePalette(i, 0, 0, 15);

	vgaPmSetActivePage(BACKGROUNDPAGE);
	vgaPmClearScr(BLACK);

	for (i = 0; i < STAR_DISP_MAX; i++)
	{
		p.x = rand() % 320;
		p.y = rand() % 200;
		k = rand() % 3;

		for (j = 0; j < STAR_COLOR_NUM; j++)
		{
			if (k)
			{
				step = p.x - (unsigned) (320 * j) / STAR_COLOR_NUM;
				while (step < 0)
					step += 320;
				vgaPmPutPixel(step, p.y, j + STAR_COLOR_START);
			}
			else
			{
				step = p.x - (unsigned) (640 * j) / STAR_COLOR_NUM;
				while (step < 0)
					step += 320;
				vgaPmPutPixel(step, p.y, j + STAR_COLOR_START);
			}
		}
	}
	/*

	shadowPutsxy(10, 5,
				 "Shooting Demo Made By J.M.Kim ", 15, 13);
	shadowPutsxy(10, 180,
				 "ÑÁ¬iÎa,Ctrl:PowerUp,Alt:Shoot", 15, 12);
	*/
	*temp = stage + '0';
	shadowPutsxy(300, 5, temp, 15, 10);
	vgaPmPutImageInviCol(253, 180, shipImg[CENTER]);
	vgaPmPutImage(260, 190, miniNum[shipNum]);

	putMiniNum(260, 10, score);

	vgaPmSetActivePage(crPage);
}

void loadEnemyPtn(int st, int num)
{
	char *temp;
	FILE *f;
	int i, j;

	temp = "enemy!!.ptn";
	temp[5] = st + '0';
	temp[6] = num + 'A';

    if (!(f = fopen(temp, "rt")))
        errExit("enemy patten file not found.");

    rewind(f);

	if (!(enemyPtn[num] = (int *)malloc(768)))
        errExit(ERR_MEMORY);

    j = 0;
    while (true)
    {
        fscanf(f, "%d", &i);
        enemyPtn[num][j++] = i;
        if (i == END)
            break;
    }

    fclose(f);
}

void loadStage(int st)
{
    int i, j, ptnMax, imgMax;
    char *temp;
	FILE *f;

	j = 0;
    temp = "stage!.map";
    temp[5] = st + '0';

    f = fopen(temp, "rt");

	fscanf(f, "%d", &imgMax);
	stageMap[j++] = imgMax;
	fscanf(f, "%d", &ptnMax);
	stageMap[j++] = ptnMax;

	while (true)
	{
		fscanf(f, "%d", &i);
		stageMap[j++] = i;
		if (i == END)
			break;
	}

	fclose(f);

	for (i = 0; i < ptnMax; i++)
		loadEnemyPtn(st, i);

	j = 0;
	temp = "boss!.ptn";
	temp[4] = st + '0';

	f = fopen(temp, "rt");

	while (true)
	{
		fscanf(f, "%d", &i);
		bossPtn[j++] = i;
		if (i == END)
			break;
	}

	fclose(f);
}

/* stage ready function */

void readyStage(int stage)
{
	int x, y;
	FILE *fp;
	byte p[768];
	char *str;

	vgaPmSetActivePage(PAGE0);
	vgaPmPcxCutDisp("eyecatch.pcx", 0, 0, 0, 320, 200);

	vgaPmSetDispPage(PAGE0);
	vgaPmFullPageCopy(1, 0);
	str = "STAGE !";
	*(str + 6) = stage + '0';
	shadowPutsxy(110, 80, str, 13, 9);
	shadowPutsxy(110, 180, "Wait a moment.", 13, 9);
	vgaPmGetPos(&x, &y);

	loadSprite(stage);

	shadowPutsxy(x, y, ".", 13, 9);
	vgaPmGetPos(&x, &y);

	loadMusic(stage);

	shadowPutsxy(x, y, ".", 13, 9);
	vgaPmGetPos(&x, &y);

	loadBackGround(stage);

	shadowPutsxy(x, y, ".", 13, 9);
	vgaPmGetPos(&x, &y);

	loadStage(stage);

	shadowPutsxy(x, y, ".", 13, 9);
	vgaPmGetPos(&x, &y);

	vgaPmPageCopy(0, 0, 120, 1, 0, 120, 320, 80);
	vgaPmSetDispPage(PAGE0);
	shadowPutsxy(110, 180, "Press Any Key.", 13, 9);

	getch();
	
	fp = fopen("palette.pal", "rb");
    fread(p, 768, 1, fp);
    vgaSetAllPalette(p);
    fclose(fp);

	vgaSetOnePalette(0,0,0,15);
	vgaSetOnePalette(invisibleColor,0,0,15);

	/* copy BackGround Page to Active Pages 0 and 1 page */

	vgaPmPageCopy(0, 0, 0, BACKGROUNDPAGE, 0, 0, 320, 200);
	vgaPmPageCopy(1, 0, 0, BACKGROUNDPAGE, 0, 0, 320, 200);
}

int advanceStage(int command)
{
	static int stageTime;
    static int mapPos;
    static int tempTime;

    int time, event, arg[4], i;

    if (command == INIT)
    {
        item(INIT, NULL);
        object(INIT, NULL);
        enemy(INIT, NULL);
        eShoot(INIT, NULL);
        boss(INIT);
        i = shoot(OTHER + 2);
        shoot(INIT);
        if (i)
            shoot(OTHER + i - 1);
        powerUp(OTHER + 1);

		tempTime = (int) getClock();
		mapPos = 2;
        stageTime = 0;
        return true;
    }

    if (command == RUN)
    {
		if ((int) getClock() - tempTime < STAGE_TIME)
            return false;
        else
            stageTime++;
		tempTime = (int) getClock();

		if ( gameState == BOSS )
			boss(RUN);
		else
        while (gameState == PLAY)
        {
            time = *(stageMap + mapPos);
            if (stageTime < time)
                return true;

            mapPos++;
            event = *(stageMap + mapPos++);
            switch (event)
            {
                case SKIP:
                    break;
                case ENEMY:
                    arg[0] = *(stageMap + mapPos++);
                    enemy(SET, arg);
                case OBJECT:
                    break;
				case END:
					gameState = BOSS;
					boss(SET);
					break;
				default:
					errExit("map file damaged\n");
					break;
			}
			break;
		}
	}
	return true;
}

/* game main function */

void playGame(void)
{
	vgaPmSetDispPage(PAGE0);

	//vgaPmSetActivePage(PAGE1);

	advanceStage(INIT);
	amdSetVolume(70);
	amdPlay(stageMusic[0]);

	restoreKbd();
	keyQue[keyRight] = true;
	ship(RUN);
	keyQueClear();
	setKbd(keyEvent);

	while (gameState == PLAY || gameState == BOSS)
	{
		scroll();

		ship(RUN);
		shoot(RUN);

		if (advanceStage(RUN))
		{
			enemy(RUN, NULL);
			item(RUN, NULL);
		}
		eShoot(RUN, NULL);
		object(RUN, NULL);

		//vgaPmFullPageCopy(PAGE0, PAGE1);
	}
}

/* ----------- */

void shipBoom(void)
{
	int i, arg[2];

	arg[0] = vgaSprGetXloc(SHIP_SPR);
	arg[1] = vgaSprGetYloc(SHIP_SPR);
	object(SET, arg);
	vgaSprHide(SHIP_SPR);
	for (i = 0; i < 10;)
	{
		amdSetVolume(70 - i * 7);
		scroll();
        shoot(RUN);
		if (advanceStage(RUN))
		{
			if ( vgaSprIsDisplayed(BOSS_NORMAL,activePage) )
				boss(RUN);
			i++;
			enemy(RUN, NULL);
			item(RUN, NULL);
		}
		eShoot(RUN, NULL);
		object(RUN, NULL);
	}
	ship(INIT);
	advanceStage(INIT);

	vgaPmPutImage(260, 190, miniNum[shipNum]);
	vgaPmPageCopy(BACKGROUNDPAGE, 260, 190, activePage, 260, 190, 6, 6);

	restoreKbd();
	amdRewind(stageMusic[0]);
}

/* stage Ending */

void stageEnding(int st)
{
    FILE *f;
    char *fname;
    int i, j, imgMax, ptnMax, jk[9];

    if (gameState == STAGECLEAR)
    {
        jk[5] = vgaSprGetXloc(BOSS_NORMAL);
        jk[6] = vgaSprGetYloc(BOSS_NORMAL);
        jk[7] = vgaSprGetXsize(BOSS_NORMAL);
        jk[8] = vgaSprGetYsize(BOSS_NORMAL);

        boss(INIT);
        for (i = 0; i < ENEMY_SPR_MAX; i++)
            enemy(UNSET, &i);
        for (i = 0; i < SHOOT_SPR_MAX; i++)
            vgaSprHide(SHOOT_SPR + i);
        enemy(INIT, NULL);
        eShoot(INIT, NULL);

		for (i = 0, j = (int) getClock(); i < 30;)
		{
			scroll();
			ship(RUN);
            shoot(RUN);
			object(RUN, NULL);
			if ((int) getClock() - j < STAGE_TIME)
				continue;
			else
			{
				i++;
				j = (int) getClock();
				jk[0] = jk[5] + (rand() % (jk[7] / 3)) * 3;
				jk[1] = jk[6] + (rand() % (jk[8] / 3)) * 3;
				object(SET, jk);
				item(RUN, NULL);
			}
        }
    }

    object(INIT, NULL);
    item(INIT, NULL);

    fname = "stage!.map";
    fname[5] = st + '0';

    if (!(f = fopen(fname, "rt")))
        errExit("stage map file not found.");

    fscanf(f, "%d", &imgMax);
    fscanf(f, "%d", &ptnMax);
    fclose(f);

    for (i = 0; i < imgMax; i++)
        free(enemyImg[i]);

    for (i = 0; i < ptnMax; i++)
        free(enemyPtn[i]);

    for (i = 99; i > 0; i--)
	{
		amdSetVolume(i*7/10);
        vgaPmHLine(0, 100 - i, 319, 255);
		vgaPmHLine(0, 99 + i, 319, 255);
		delay(100);
	}

	restoreKbd();
	amdClose(stageMusic[0]);
	amdSetVolume(70);
}
