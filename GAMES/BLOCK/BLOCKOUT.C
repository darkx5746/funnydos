/*
	BlockOut version 0.7 by Tae-Hyeong Kim (Gapacci)

	0.1 - 1994/02/25
	0.2 - 1994/03/12
	0.3 - 1994/03/24
	0.4 - 1994/04/03
	0.5 - 1994/04/24 - 7 stage, support mouse.
	0.6 - 1994/06/14 - change image interface.
	0.7 - 1994/07/03 - fix bugs & rearrange all source.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#include "key.h"
#include "timer.h"
#include "color.h"
#include "mouse.h"
#include "stdpcx.h"
#include "vgastd.h"
#include "palette.h"


#define CLOCK_HZ	    1000

#define PAD_DELAY           20
#define BALL_DELAY          10
#define SOUND_DELAY         10

#define SCORE_BOARD_COLOR   80

#define PAD_COLOR           0       /*   0 -   9 */
#define BALL_COLOR          10      /*  10 -  19 */
#define WALL_COLOR          20      /*  20 -  29 */
#define BLOCK_COLOR         30      /*  30 -  89 */
#define HARD_BLOCK_COLOR    90      /*  90 -  99 */
#define BACKGROUND_COLOR    100     /* 100 - 255 */

#define MAX_STAGE           7
#define MAX_BALL            10
#define MAX_XBLOCK          15
#define MAX_YBLOCK          14

#define BLOCK_XSIZE         16
#define BLOCK_YSIZE         8
#define BALL_SIZE           4
#define WALL_SIZE           8
#define PAD_XSIZE           (BALL_SIZE * 10)
#define PAD_YSIZE           (BALL_SIZE + BALL_SIZE / 4)

#define PAD_IMG_SIZE	    (PAD_XSIZE * PAD_YSIZE + 4)
#define BALL_IMG_SIZE	    (BALL_SIZE * BALL_SIZE + 4)
#define BLOCK_IMG_SIZE	    (BLOCK_XSIZE * BLOCK_YSIZE + 4)

#define PAD_MID_X           (padImgX + PAD_XSIZE / 2)
#define MAX_PAD_SPEED       20
#define MAX_WAIT_HIT_PAD    100
#define HARD_BLOCK_HIT      6


/* Macro functions ----------------------------------------------- */

#define hideBall()                                                  \
	vgaStdPutImage( ballImgX, ballImgY, ballBackImage )
#define hidePad()                                                   \
	vgaStdPutImage( padImgX, 199 - BALL_SIZE, padBackImage )
#define hideBlock(x,y)                                              \
	vgaStdPutImage( (x) * BLOCK_XSIZE + WALL_SIZE,                  \
		((y)+2) * BLOCK_YSIZE + WALL_SIZE, blockBackImage[y][x] )


/* Function prototypes ------------------------------------------- */

void initGame( void );
void closeGame( void );

void generateStage( void );

void waitUser( void );
void shadowPutsxy( int x, int y, char *str, int f, int b );
void opening( void );
void printBallNum( void );
void printScore( void );
void printStageNum( void );
void printStageClear( void );

void drawScoreBoard( void );
void drawBall( void );
void drawBlock( int block_x, int block_y, int color );
void drawPad( void );
void drawStage( int stage_num );

int moveBall( void );
void movePad( void );

void startBall( void );

int checkHitPad( int nextX );
void hitObject( int objectX, int objectY );
int checkUserEvent( void );

int playBall( void );
word userTimerFunc( void );


/* Global variables ---------------------------------------------- */

typedef enum { KEYBOARD, MOUSE } Control_Type;

Control_Type control;

/* demonstrate how to make a user stage */
byte specialStage[MAX_YBLOCK][MAX_XBLOCK] = {
   30, 90, 90,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   90,  0,  0,  0, 90, 90, 50,  0, 90, 90, 40,  0, 90, 90, 50,
   90,  0,  0,  0,  0,  0, 90,  0, 90,  0, 90,  0,  0,  0, 90,
   90,  0, 90,  0, 50, 90, 90,  0, 90,  0, 90,  0, 50, 90, 90,
   90,  0, 90,  0, 90,  0, 90,  0, 90,  0, 90,  0, 90,  0, 90,
   30, 90, 90,  0, 50, 90, 90,  0, 90, 90, 40,  0, 50, 90, 90,
	0,  0,  0,  0,  0,  0,  0,  0, 90,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0, 60, 90, 70,  0, 60, 90, 70,  0, 90,  0,  0,  0,
	0,  0,  0, 90,  0, 90,  0, 90,  0, 90,  0, 80,  0,  0,  0,
	0,  0,  0, 90,  0,  0,  0, 90,  0,  0,  0, 90,  0,  0,  0,
	0,  0,  0, 90,  0,  0,  0, 90,  0,  0,  0, 90,  0,  0,  0,
	0,  0,  0, 90,  0, 70,  0, 90,  0, 70,  0, 90,  0,  0,  0,
	0,  0,  0, 60, 90, 90,  0, 60, 90, 90,  0, 90,  0,  0,  0,
};

char *stagePicture[MAX_STAGE] = {
	"stage1.pcx", "stage2.pcx", "stage3.pcx", "stage4.pcx",
	"stage5.pcx", "stage6.pcx", "stage7.pcx"
};


int isSoundOn;	    	/* sound on/off flag */
int soundCount;     	/* sound sustain count ( SOUND_DELAY ~ 0 )*/

long prevClock;

int ballDelayCount, ballBasicDelay, padDelayCount, padBasicDelay;

int startStage, stage =1, score =0;
int ballNum =0, blockNum;  /* ball/block numbers */

int hitCount;           /* for ball speed increment */
int afterHitPadCount;   /* prevent infinate loop of ball */

/*  these variables are used when useing keyboard to control pad */
int prevPadMoveDir;		/*	1  : same direction,
							-1 : opposite direction,
							0  : no moves */
int padMoveCount;   	/* count when moves to the same direction */
int padSpeedTable[MAX_PAD_SPEED] =
	{ 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10 };

int ballX, ballY;   		/* 16 times accurate coordinate        */
int ballImgX, ballImgY;     /* ball image on the screen            */
int ballStepX, ballStepY;   /* x/y increment of ballX/Y ( 7 ~ 13 ) */
int ballDirX, ballDirY;     /* direction of ball ( -1 / 1 )        */
int padImgX, padImgY;       /* padImage on the screen              */

/* restore background image */
byte padImage[PAD_IMG_SIZE], ballImage[BALL_IMG_SIZE];
byte padBackImage[PAD_IMG_SIZE], ballBackImage[BALL_IMG_SIZE];
byte blockBackImage[MAX_YBLOCK][MAX_XBLOCK][BLOCK_IMG_SIZE];

/* stores the first color of each object */
byte stageBlock[MAX_STAGE][MAX_YBLOCK][MAX_XBLOCK];

/* stores the count left to crush hard block */
byte hardBlock[MAX_YBLOCK][MAX_XBLOCK];


/* Main function ------------------------------------------------- */

void main( int argc, char *argv[] )
{
	int endStage, endGame = FALSE;

	startStage = 1;

	if ( argc == 2 )
	{
		startStage = atoi( argv[1] );
		if ( (startStage < 1) || (startStage > MAX_STAGE) )
			startStage = 1;
	}

	initGame();

	while ( !endGame )
	{
		drawStage( stage );
		startBall();
		endStage = FALSE;

		while ( !endStage )
		{
			if ( playBall()==FAIL || checkUserEvent()==FAIL )
				endGame = endStage = TRUE;

			if ( blockNum <= 0 )
			{
				endStage = TRUE;
				printStageClear();
				if ( ++stage > MAX_STAGE ) endGame = TRUE;
			}
		}
		nosound();
	}

	closeGame();
}


/* Functions ... ------------------------------------------------- */

void initGame( void )
{
	FILE *imgFp, *palFp;
	byte pal[768];

	/* set variables */

	stage = startStage;
	score = 0;
	ballNum = MAX_BALL;
	isSoundOn = FALSE;
	soundCount = 0;
	padImgX = 100;
	padImgY = 199 - BALL_SIZE;
	invisibleColor = 0x80;

	/* deside control */

	if ( !initMouse() )
	{
		control = KEYBOARD;
		printf( "Mouse driver not found. Use keyboard.\n"
				"Press any key..." );
		keyRead();
	}
	else
		control = MOUSE;

	/* load images and palette */

	if ( !(imgFp = fopen( "BLOCKOUT.IMG", "rb" )) )	errExit( "BLOCKOUT.IMG open error" );
	if ( !(palFp = fopen( "BLOCKOUT.PAL", "rb" )) )	errExit( "BLOCKOUT.PAL open error" );

	fread( pal, 768, 1, palFp );

	fread( ballImage, BALL_IMG_SIZE, 1, imgFp );
	fread( padImage, PAD_IMG_SIZE, 1, imgFp );

	fclose( imgFp );
	fclose( palFp );

	vgaSetMode( VGA256COL );
	vgaStdOpenHan( "HANGULG.FNT", "ENGLISH.FNT" );
	opening();

	vgaSetAllPalette( pal );
	generateStage();
	drawScoreBoard();

	startNewTimer( 0 );

	setClockRate( CLOCK_HZ );
}

void closeGame( void )
{
	int i, j;

	stopNewTimer();

	shadowPutsxy( 50, 110, "G A M E   O V E R !", 55, 0 );

	keyRead();

	vgaStdCloseHan();
	vgaSetMode( 3 );
}

void generateStage( void )
{
	int i, j, k, l, m, color;

	/* stage 1 */

	for ( j = 0; j < 12; j++ )
		for ( i = 0; i < MAX_XBLOCK; i++ )
			stageBlock[0][j][i] = BLOCK_COLOR + (12-j-1)/2 * 10;

	stageBlock[0][1][2] = HARD_BLOCK_COLOR;
	stageBlock[0][1][7] = HARD_BLOCK_COLOR;
	stageBlock[0][1][12] = HARD_BLOCK_COLOR;

	/* stage 2 */

	for ( j = 0; j < MAX_YBLOCK; j++ )
		for ( i = 0; i < MAX_XBLOCK; i ++ )
			if ( (i != 0) && (i != 7) && (i != 14) )
				stageBlock[1][j][i] = BLOCK_COLOR + (rand() % 6) * 10;

	/* stage 3 */

	for ( j = 0; j < 8; j++ )
		for ( i = 0; i < 6; i++ )
		{
			color = ( (j==0) || (j==7) ? HARD_BLOCK_COLOR :
				BLOCK_COLOR + (j-1) * 10 );

			stageBlock[2][j+i][i] = color;
			stageBlock[2][j+i][MAX_XBLOCK-i-1] = color;
		}

	for ( j = 7; j < 13; j++ )
		for ( i = 6; i < MAX_XBLOCK - 6; i++ )
			stageBlock[2][j][i] = BLOCK_COLOR + (j-7) * 10;

	/* stage 4 */

	for ( j = 0; j < MAX_YBLOCK; j++ )
		for ( i = 0; i < MAX_XBLOCK; i++ )
			stageBlock[3][j][i] = BLOCK_COLOR + ((j / 2) % 6) * 10;

	for ( j = 0; j < MAX_YBLOCK; j += 2 )
		for ( i = 1; i < MAX_XBLOCK; i += 2 )
			stageBlock[3][((i-1)%4?j:j+1)][i] = HARD_BLOCK_COLOR;

	/* stage 5 */

	for ( i = 1; i < MAX_XBLOCK - 1; i++ )
		for ( j = 0; j < MAX_YBLOCK; j++ )
			stageBlock[4][j][i] =
				( (i==1) || (i==MAX_XBLOCK-2) || (j>=MAX_YBLOCK-1) ) ?
				HARD_BLOCK_COLOR :
				( BLOCK_COLOR + (i % 6) * 10 );

	/* stage 6 */

	for ( l = 0; l < MAX_YBLOCK; l += 5 )
		for ( k = 0; k < MAX_XBLOCK; k += 5 )
			for ( j = 0; j < 4; j++ )
				for ( i = 0; i < 4; i++ )
				{
					m = ( l == 5 ? 1 : 0 );
					stageBlock[5][j+l][i+k+m] =
						( (i==0) || (i==3) || (j==0) || (j==3) ) ?
						HARD_BLOCK_COLOR :
						( BLOCK_COLOR + ((l * 3 + k) % 6) * 10 );
				}
	/* stage 7 */
	memcpy( stageBlock[6][0], specialStage[0], MAX_YBLOCK * MAX_XBLOCK );
}

void opening( void )
{
	vgaStdClearScr( BLACK );
	shadowPutsxy( 64, 40, "B  L  O  C  K    O  U  T", CYAN, BLUE );
	vgaStdSetForeColor( LIGHTBLUE );
	vgaStdPutsxyC( 70, "ver 0.7  1994.7.3" );
	vgaStdPutsxyC( 90, "by Gapacci");
	vgaStdSetForeColor( GREEN );

	if ( control == MOUSE )
		vgaStdPutsxyC( 120, "Ì—a¶‘»¢·± -  a¶¯a" );
	else
		vgaStdPutsxyC( 120, "Ì—a¶‘»¢·± - ¶E½¢/µ¡Ÿe½¢ Shift" );

	vgaStdPutsxyC( 140, "¸q¯¡ ñÂ‘ - P" );
	vgaStdPutsxyC( 160, "‰A·± ÈiÂ‰ - Esc" );

	waitUser();

	vgaStdClearScr(BLACK);
}

void waitUser( void )
{
	if ( control == MOUSE )
	{
		while ( !getMouseLeftButton() )
			if ( keyPressed() )
			{
				keyRead();
				break;
			}
	}
	else
	{
		while ( !keyPressed() );
		keyRead();
	}
}

void shadowPutsxy( int x, int y, char *str, int fore, int back )
{
	vgaStdSetForeColor( back ); vgaStdPutsxy( x+1, y+1, str );
	vgaStdSetForeColor( fore ); vgaStdPutsxy( x, y, str );
}

void printStageNum( void )
{
	vgaStdBoxFill( 264, 45, 310, 65, SCORE_BOARD_COLOR );
	vgaStdPrintfxy( 264, 45, "%5d", stage );
}

void printScore( void )
{
	vgaStdBoxFill( 264, 105, 310, 125, SCORE_BOARD_COLOR );
	vgaStdPrintfxy( 264, 105, "%5d", score );
}

void printBallNum( void )
{
	vgaStdBoxFill( 264, 165, 310, 185, SCORE_BOARD_COLOR );
	vgaStdPrintfxy( 264, 165, "%5d", ballNum );
}

void printStageClear( void )
{
	shadowPutsxy( 20, 90, "S T A G E   C L E A R E D !", 55, 0 );
	sound( 1728 );
	delay(200);
	nosound();

	waitUser();
}

void drawScoreBoard( void )
{
	vgaStdBoxFill( 256, 0, 319, 199, SCORE_BOARD_COLOR );

	shadowPutsxy( 264, 20, "STAGE", 99, 0 );
	shadowPutsxy( 264, 80, "SCORE", 99, 0 );
	shadowPutsxy( 264, 140, " BALL", 99, 0 );
}

void drawStage( int stage )
{
	int i, j, color;

	if ( (stage > MAX_STAGE) || (stage < 1) ) return;

	printStageNum();
	printScore();
	printBallNum();

	vgaStdBoxFill( 0, 0, 255, 199, 0 );

	/* draw wallpaper */
	vgaStdPcxCutDisp( stagePicture[stage-1], BACKGROUND_COLOR,
		0, 0, 256 - WALL_SIZE, 200 );

	/* draw wall */
	for ( i = 0; i < WALL_SIZE; i++ )
	{
        vgaStdHLine( i, i, 255-i, i + WALL_COLOR );
        vgaStdVLine( i, i, 199, i + WALL_COLOR );
        vgaStdVLine( 255-i, i, 199, i + WALL_COLOR );
	}

	/* draw blocks */
	blockNum = 0;
	for ( j = 0; j < MAX_YBLOCK; j++ )
		for ( i = 0; i < MAX_XBLOCK; i++ )
		{
			color = stageBlock[stage-1][j][i];
			if ( (color >= WALL_COLOR) && (color < BACKGROUND_COLOR) )
			{
				if ( color >= BLOCK_COLOR )	blockNum++;
				drawBlock( i, j, color );
				hardBlock[j][i] = ( color >= HARD_BLOCK_COLOR ) ?
					HARD_BLOCK_HIT : 0;
			}
		}
}

void drawBlock( int x, int y, int color )
{
	int sx, sy, ex, ey;

	sx = x * BLOCK_XSIZE + WALL_SIZE;
	sy = (y+2) * BLOCK_YSIZE + WALL_SIZE;
	ex = (x+1) * BLOCK_XSIZE + WALL_SIZE - 1;
	ey = (y+3) * BLOCK_YSIZE + WALL_SIZE - 1;

	vgaStdGetImage( sx, sy, ex, ey, blockBackImage[y][x] );

	vgaStdPutPixel( sx, sy, color + 9 );
	vgaStdPutPixel( sx, ey, color + 5 );
	vgaStdPutPixel( ex, sy, color + 5 );
	vgaStdPutPixel( ex, ey, color );

	vgaStdHLine( sx+1, sy, ex-1, color + 7 );
	vgaStdHLine( sx+1, ey, ex-1, color + 2 );
	vgaStdVLine( sx, sy+1, ey-1, color + 7 );
	vgaStdVLine( ex, sy+1, ey-1, color + 2 );

	vgaStdBoxFill( sx+1, sy+1, ex-1, ey-1, color + 4 );
}

void drawBall( void )
{
	int sx, sy, ex, ey;

	sx = ballImgX;
	sy = ballImgY;
	ex = sx + BALL_SIZE - 1;
	ey = sy + BALL_SIZE - 1;

	vgaStdGetImage( sx, sy, ex, ey, ballBackImage );

	vgaStdPutImageInviCol( sx, sy, ballImage );
}

void drawPad( void )
{
	int sx, sy, ex, ey;

	sx = padImgX;
	sy = padImgY;
	ex = sx + PAD_XSIZE - 1;
	ey = sy + PAD_YSIZE - 1;

	vgaStdGetImage( sx, sy, ex, ey, padBackImage );

	vgaStdPutImageInviCol( sx, sy, padImage );
}

void hitObject( int x, int y )
{
	word bx, by, freq, prevScore;
	byte color;

	color = vgaStdGetPixel( x, y );

	if ( color >= BACKGROUND_COLOR ) return;

	if ( color >= BLOCK_COLOR )
	{
		bx = (x - WALL_SIZE) / BLOCK_XSIZE;
		by = (y - WALL_SIZE) / BLOCK_YSIZE - 2;

		if ( bx >= MAX_XBLOCK || by >= MAX_YBLOCK )
			errExit( ERR_FATAL );

		prevScore = score;

		if ( color >= HARD_BLOCK_COLOR )
			if ( --hardBlock[by][bx] != 0 )
			{
				score += 4;
				printScore();
				freq = 1728;
			}

		if ( hardBlock[by][bx] == 0 )
		{
			blockNum--;
			hideBlock( bx, by );
			score += MAX_YBLOCK - by + 4;
			printScore();
			if ( (score/1000 - prevScore/1000) )
			{
				ballNum ++;
				printBallNum();
				freq = 3456;
			}
			else
				freq = 1500 - color;
		}
	}
	else if ( color >= WALL_COLOR )	freq = 864;
	else if ( color >= BALL_COLOR )	freq = 2000;
	else freq = 432;	/* color == PAD_COLOR */

	/* check & escape from infinite loop */

	if ( ++afterHitPadCount > MAX_WAIT_HIT_PAD )
	{
		afterHitPadCount = MAX_WAIT_HIT_PAD / 3;
		ballStepX = (rand() % 4) * 2 + 7;
		ballStepY = 20 - ballStepX;
	}

	/* speed up */

	if ( ++hitCount >= (blockNum / 10 + 10) )
	{
		hitCount = 0;
		if ( ballBasicDelay > 3 )
			ballBasicDelay--;
	}

	sound( freq );
	soundCount = 0;
	isSoundOn = TRUE;
}

int checkHitPad( int nextX )
{
	if ( (nextX <= padImgX - BALL_SIZE) ||
		(nextX >= padImgX + PAD_XSIZE) )
			return FALSE;

	if ( (nextX < padImgX + BALL_SIZE * 2) ||
		(nextX >= padImgX + PAD_XSIZE - BALL_SIZE * 2) )
	{
		if ( ((PAD_MID_X - nextX)) * ballDirX > 0 ) /* from outside */
		{
			ballDirX = -ballDirX;

			if ( ballStepY > 9 )
			{
				ballStepY -= 4;
				ballStepX += 4;
			}
		}
		else /* from inside */
		{
			if ( ballStepY < 11 )
			{
				ballStepX = 13;
				ballStepY = 7;
			}
			else
			{
				ballStepX = 11;
				ballStepY = 9;
			}
		}
	}
	else if ( (nextX < padImgX + BALL_SIZE * 4) ||
		(nextX >= padImgX + PAD_XSIZE - BALL_SIZE * 4) )
	{
		if ( ((PAD_MID_X - nextX)) * ballDirX > 0 ) /* from outside */
		{
			ballDirX = -ballDirX;

			if ( ballStepY < 11 )
			{
				ballStepY += 4;
				ballStepX -= 4;
			}
		}
	}

	return TRUE;
}

int moveBall( void )
{
	int nextX, nextY, checkX, checkY;
	int i, tmpStep, color;

	ballImgX = (ballX>>4);
	ballImgY = (ballY>>4);

	nextX = (( ballX + ballStepX*ballDirX )>>4);
	nextY = (( ballY + ballStepY*ballDirY )>>4);

	if ( nextX == ballImgX ) nextX += ballDirX;
	if ( nextY == ballImgY ) nextY += ballDirY;

	checkX = ( ballDirX > 0 ? nextX + BALL_SIZE - 1 : nextX );
	checkY = ( ballDirY > 0 ? nextY + BALL_SIZE - 1 : nextY );

	/* check side */

	if ( nextX != ballImgX )
		for ( i = 0; i < BALL_SIZE; i++ )
		{
			color = vgaStdGetPixel( checkX, ballImgY + i );
			if ( color < BACKGROUND_COLOR )
			{
				ballDirX = -ballDirX;
				hitObject( checkX, ballImgY + i );
				return SUCCESS;
			}
		}

	/* check hit/miss ball */

	if ( (nextY >= padImgY - BALL_SIZE) && (ballDirY > 0) )
	{
		if ( checkHitPad( nextX ) )
		{
			ballDirY = -ballDirY;
			afterHitPadCount = 0;
			hitObject( PAD_MID_X, 199 - BALL_SIZE );
		}
		else	/* process miss ball */
		{
			hideBall();
			hidePad();
			if ( --ballNum < 0 ) return FAIL;
			printBallNum();
			startBall();
		}
		return SUCCESS;
	}

	/* check up/down */

	if ( nextY != ballImgY )
		for ( i = 0; i < BALL_SIZE; i++ )
		{
			color = vgaStdGetPixel( ballImgX + i, checkY );
			if ( color < BACKGROUND_COLOR )
			{
				ballDirY = -ballDirY;
				hitObject( ballImgX + i, checkY );
				return SUCCESS;
			}
		}

	/* check corner */

	if ( vgaStdGetPixel( checkX, checkY ) < BACKGROUND_COLOR )
	{
		ballDirX = -ballDirX;
		ballDirY = -ballDirY;
		tmpStep = ballStepX;
		ballStepX = ballStepY;
		ballStepY = tmpStep;
		hitObject( checkX, checkY );
		return SUCCESS;
	}

	hideBall();

	ballX += ballStepX * ballDirX;
	ballY += ballStepY * ballDirY;
	ballImgX = (ballX>>4);
	ballImgY = (ballY>>4);

	drawBall();

	return SUCCESS;
}

void startBall( void )
{
	padBasicDelay = PAD_DELAY;
	ballBasicDelay = BALL_DELAY;
	ballDelayCount = padDelayCount = 0;
	afterHitPadCount = padMoveCount = hitCount = 0;
	prevPadMoveDir = 0;
	padImgX = 100;

	drawPad();

	waitUser();

	ballX = (WALL_SIZE+2)<<4;
	ballY = 160<<4;

	ballStepX = (rand() % 4) * 2 + 7;
	ballStepY = 20 - ballStepX;
	ballDirX = 1;
	ballDirY = -1;

	ballImgX = ballX>>4;
	ballImgY = ballY>>4;

	drawBall();
}

int checkUserEvent( void )
{
	if ( keyPressed() )
		switch ( keyRead() )
		{
			case KEY_Esc:
				return FAIL;
			case KEY_P:
				waitUser();
		}

	return SUCCESS;
}

void movePad( void )
{
	if ( control == MOUSE )
	{
		int x, y;

		getMousePos( &x, &y );

		if ( padImgX == x )	return;

		hidePad();

		/* boundary check */

		if ( x < WALL_SIZE )
			x = WALL_SIZE;
		else if ( x > (256 - WALL_SIZE - PAD_XSIZE) )
			x = 256 - WALL_SIZE - PAD_XSIZE;

		setMousePos( x, y );
		padImgX = x;
	}
	else /* control == KEYBOARD */
	{
		byte sft;
		int moveDir;

		sft = keyShiftStatus();

		if ( (sft & KEY_LeftShift) && !(sft & KEY_RightShift) )
			moveDir = -1;
		else if ( (sft & KEY_RightShift) && !(sft & KEY_LeftShift) )
			moveDir = 1;
		else    /* no shift key pressed or both shift key pressed */
		{
			prevPadMoveDir = 0;
			return;
		}

		/* check direction to previous move */

		if ( (moveDir * prevPadMoveDir) <= 0 )
			padMoveCount = 0;
		else if ( ++padMoveCount > MAX_PAD_SPEED )
			padMoveCount = MAX_PAD_SPEED;

		prevPadMoveDir = moveDir;

		hidePad();

		padImgX += padSpeedTable[padMoveCount] * moveDir;

		/* boundary check */

		if ( padImgX < WALL_SIZE )
			padImgX = WALL_SIZE;
		else if ( padImgX > (256 - WALL_SIZE - PAD_XSIZE) )
			padImgX = 256 - WALL_SIZE - PAD_XSIZE;
	}

	drawPad();
}

int playBall( void )
{
	if ( getClock() == prevClock ) return SUCCESS;
	prevClock = getClock();

	if ( isSoundOn )
		if ( soundCount++ > SOUND_DELAY )
		{
			soundCount = 0;
			isSoundOn = FALSE;
			nosound();
		}

	if ( ++padDelayCount > padBasicDelay )
	{
		padDelayCount = 0;
		movePad();
	}

	if ( ++ballDelayCount > ballBasicDelay )
	{
		ballDelayCount = 0;
		return moveBall();	/* return FAIL if ball missed */
	}
	else
		return SUCCESS;
}

word userTimerFunc( void )
{
	return 10000;
}
