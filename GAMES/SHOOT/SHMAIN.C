
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>

/* --------------------------------------------------------- */

#include "vgaspr.h"
#include "palette.h"
#include "pmpcx.h"
#include "pmhan.h"
#include "amdmusic.h"

#include "shgame.h"

/* --------------------------------------------------------- */

int stage = 0;
int score = 0;
int shipNum = SHIP_NUM;
int gameState = STOP;

int _super = OFF;

void opening(void);
void ending(int gs);
void initGame(void);
void closeGame(int gs);

/* --------------------------------------------------------- */

void opening(void)
{
    unsigned i = 0, j = 0;

    vgaPmSetActivePage(PAGE1);
    vgaPmPcxCutDisp("opening.pcx", 0, 0, 0, 320, 200);

    for (i = 0; i < 80; i++)
		for (j = 0; j < 200; j++, j++)
		{
			vgaPmVRamCopy(0, 0, j, 1, 79 - i, j, i + 1, 1);
			vgaPmVRamCopy(0, 79 - i, j + 1, 1, 0, j + 1, i + 1, 1);
		}
}

void loadCommonImg(void)
{
    int i;
    FILE *f;

    if (!(f = fopen("common.img", "rb")))
        errExit("common image file not found.");

    imageGet(NULL, f);
    imageGet(&shipImg[BACK], f);
    imageGet(NULL, f);
    imageGet(&shipImg[UP], f);
    imageGet(&shipImg[CENTER], f);
    imageGet(&shipImg[DOWN], f);
    imageGet(NULL, f);
    imageGet(&shipImg[FORWARD], f);
    imageGet(NULL, f);

    vgaSprSet(SHIP_SPR, OUTBUFFER);

    imageGet(&shootImg[NORMAL], f);
    imageGet(&shootImg[LASER], f);
    imageGet(&shootImg[RIPPLE], f);
    imageGet(&shootImg[RIPPLE + 1], f);
    imageGet(&shootImg[RIPPLE + 2], f);

    for (i = SHOOT_SPR; i < SHOOT_SPR_END; i++)
        vgaSprSet(i, OUTBUFFER);

    imageGet(&shieldImg, f);
    vgaSprSet(SHIELD_SPR, OUTBUFFER);

    imageGet(NULL, f);
	imageGet(&optionImg, f);

    for (i = OPTION_SPR; i < OPTION_SPR_END; i++)
        vgaSprSet(i, OUTBUFFER);

    for (i = 0; i < 3; i++)
        imageGet(&boomImg[i], f);

    for (i = 0; i < 3; i++)
        imageGet(&eShootImg[i], f);

    for (i = 0; i < 3; i++)
        imageGet(&itemImg[i], f);

    for (i = 0; i < 12; i++)
        imageGet(&panelImg[i], f);

    for (i = 0; i < 11; i++)
        imageGet(&miniNum[i], f);

    fclose(f);
}

void initGame(void)
{
	vgaPmSetGraphMode();
    vgaPmOpenHan("hangulg.fnt", "english.fnt");
    vgaSprInit();
    amdInitMusic();

    opening();
    getch();

    loadCommonImg();

    ship(INIT);

    detailCrushCheckEnabled = false;

    return;
}

void ending(int gs)
{
    int i, j;

    vgaPmSetDispPage(PAGE0);
    vgaPmSetActivePage(PAGE1);
    vgaPmPcxCutDisp("ending.pcx", 0, 0, 0, 320, 200);

    if (gs == GAMECLEAR)
        shadowPutsxy(110, 80, "GAME CLEAR", 13, 9);
    else if (gs == GAMEOVER)
        shadowPutsxy(110, 80, "GAME OVER", 13, 9);

	for (i = 0; i < 80; i++)
		for (j = 0; j < 200; j++, j++)
		{
			vgaPmVRamCopy(0, 0, j, 1, 79 - i, j, i + 1, 1);
			vgaPmVRamCopy(0, 79 - i, j + 1, 1, 0, j + 1, i + 1, 1);
		}

    vgaPmFullPageCopy(0, 1);
    vgaPmFullPageCopy(BACKGROUNDPAGE, 1);
}

void freeCommonImg(void)
{
    int i;

    free(shipImg[BACK]);
    free(shipImg[UP]);
    free(shipImg[CENTER]);
    free(shipImg[DOWN]);
    free(shipImg[FORWARD]);
    free(shootImg[NORMAL]);
    free(shootImg[LASER]);
    free(shootImg[RIPPLE]);
    free(shootImg[RIPPLE + 1]);
    free(shootImg[RIPPLE + 2]);
    free(shieldImg);
    free(optionImg);
    for (i = 0; i < 3; i++)
    {
        free(boomImg[i]);
        free(eShootImg[i]);
        free(itemImg[i]);
    }
    for (i = 0; i < 12; i++)
        free(panelImg[i]);
    for (i = 0; i < 11; i++)
        free(miniNum[i]);
}

void closeGame(int gs)
{
    freeCommonImg();

    ending(gs);

    while (kbhit())
        getch();
    getch();

    amdCloseMusic();
    vgaSprClose();
    vgaPmCloseHan();
    vgaPmEndGraphMode();

    return;
}

void main(int argc,char *argv[])
{
    initGame();

    stage = 1;
	score = 0;

	if (argc == 2)
	{
		if ( *argv[1] == 's') _super = ON;
	}

stageStart:
	readyStage(stage);

restartStage:
    gameState = PLAY;

playGame:

    playGame();

    switch (gameState)
    {
        case STAGECLEAR:
            stageEnding(stage);
            stage++;
            if (stage > STAGE_NUM)
                gameState = GAMECLEAR;
            else
                goto stageStart;

        case GAMECLEAR:
            break;

        case BOOM:
            shipNum--;
            shipBoom();
            if (!shipNum)
                gameState = GAMEOVER;
            else
                goto restartStage;

        case GAMEOVER:
            stageEnding(stage);

        default:
            break;
    }

    closeGame(gameState);
}
