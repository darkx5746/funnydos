#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "key.h"
#include "pmpcx.h"
#include "pmhan.h"
#include "timer.h"
#include "vgaspr.h"
#include "palette.h"
#include "amdmusic.h"

#define PALETTE_FNAME       "thief.pal"
#define IMAGE_FNAME         "thief.img"
#define GAME_MUSIC          "thief.mus"
#define STAGE_DATA_FNAME	"thief.dat"
#define OPENING_PCX_FNAME	"thief.opn"
#define STAGE_PCX_FNAME		"thief.stg"

#define BASIC_LIFE			3
#define MAX_LIFE            5
#define MAX_STAGE           4

#define STAGE_XSIZE         32
#define STAGE_YSIZE         18
#define STAGE_BLOCK_SIZE    10
#define STAGE_IMG_SIZE      (STAGE_BLOCK_SIZE * STAGE_BLOCK_SIZE + 4)

#define NFONT_XSIZE         9
#define NFONT_YSIZE			13
#define NFONT_SIZE			(NFONT_XSIZE * NFONT_YSIZE + 4)
#define STR_STAGE_IMG_SIZE	((NFONT_XSIZE*5 + 3*4) * NFONT_YSIZE + 4)
#define STR_SCORE_IMG_SIZE	((NFONT_XSIZE*5 + 3*4) * NFONT_YSIZE + 4)

#define PLAYER_SPR_NUM		1
#define ENEMY1_SPR_NUM		4
#define ENEMY2_SPR_NUM		2
#define ITEM1_SPR_NUM		1
#define ITEM2_SPR_NUM		1
#define GAS_SPR_NUM			3
#define FOOD1_SPR_NUM		10
#define FOOD2_SPR_NUM		10
#define MAX_SPR_NUM			(PLAYER_SPR_NUM + ENEMY1_SPR_NUM +	     \
							ENEMY2_SPR_NUM + ITEM1_SPR_NUM +		 \
							ITEM2_SPR_NUM + GAS_SPR_NUM +   		 \
							FOOD1_SPR_NUM + FOOD2_SPR_NUM + 1)

#define JUMP_STEP           12
#define WALK_STEP			4

#define DEAD_FRAME          4

#define PLAYER_IMG_NUM      4
#define ENEMY_IMG_NUM       3
#define ITEM_IMG_NUM        3
#define GAS_IMG_NUM			3
#define FOOD_IMG_NUM        1

#define IMAGE_XSIZE         16
#define IMAGE_YSIZE         20
#define IMG_SIZE            (IMAGE_XSIZE * IMAGE_YSIZE + 4)

#define DIR_CENTER			0x01
#define DIR_UP    			0x02
#define DIR_DOWN  			0x04
#define DIR_LEFT  			0x08
#define DIR_RIGHT			0x10

#define SPEED_UP			20	/* SPEED = dots / second */
#define PLAYER_SPEED		45
#define ENEMY1_SPEED		30
#define ENEMY2_SPEED		30
#define ITEM1_SPEED			30
#define ITEM2_SPEED		    30
#define GAS_SPEED			50

#define SCORE_ITEM			1000
#define SCORE_FOOD1			100
#define SCORE_FOOD2			200

#define GAS_DELAY           (5 * BASIC_CLOCK_HZ)    /* seconds */
#define SUPER_MODE_DELAY	(4 * BASIC_CLOCK_HZ)

#define MIN_REFRESH_CYCLE   12  /* CYCLE / SECONDS */
#define SOUND_SUSTAIN		3

#define CMD_JUMP			0
#define CMD_LEFT            1
#define CMD_RIGHT           2
#define CMD_GAS             3
#define CMD_NEXTSTAGE		4
#define CMD_PAUSE			5
#define CMD_ESC             6
#define CMD_NUM             7

#define setClock(s)			(internalClock = s)

/* Type definitions ----------------------------------------------- */

typedef enum {
	CENTER, UP, DOWN, LEFT, RIGHT
} Direction;

typedef enum {
	FOOD1 = 0, FOOD2, ENEMY1, ENEMY2, ITEM1, ITEM2, PLAYER, GAS,
	OBJECT_TYPE_NUM
} Object_Type;

typedef enum {
	ST_OFF = 0, ST_ON, ST_DEAD, ST_CAUGHT, ST_SUPER
} Object_Status;

typedef struct {
	int  x, y;
	int  frameStep;			/* animation frame step 		*/
	int  jumpStep;			/* jump step 					*/
	int  opId;				/* opponent id 					*/
	Direction     dir;		/* moving direction 			*/
	Direction     frameDir;	/* image direction				*/
	Object_Type   type;
	Object_Status status;
	int  putFlag;
	long frameClock;
	long prevClock;			/* previously processed time 	*/
	long stopWatch;
} Object;


/* Global variables ----------------------------------------------- */

int sprAbleDir[OBJECT_TYPE_NUM] = {
	0,                                                    /* FOOD1  */
	0,                                                    /* FOOD2  */
	DIR_LEFT | DIR_RIGHT,                                 /* ENEMY1 */
	DIR_LEFT | DIR_RIGHT | DIR_UP | DIR_DOWN | DIR_CENTER,/* ENEMY2 */
	DIR_LEFT | DIR_RIGHT | DIR_CENTER,                    /* ITEM1  */
	DIR_LEFT | DIR_RIGHT | DIR_DOWN | DIR_CENTER,         /* ITEM2  */
	DIR_LEFT | DIR_RIGHT | DIR_UP | DIR_DOWN | DIR_CENTER,/* PLAYER */
	DIR_LEFT | DIR_RIGHT | DIR_CENTER 					  /* GAS    */
};

int sprNumTable[OBJECT_TYPE_NUM] = {
	FOOD1_SPR_NUM, FOOD2_SPR_NUM, ENEMY1_SPR_NUM, ENEMY2_SPR_NUM,
	ITEM1_SPR_NUM, ITEM2_SPR_NUM, PLAYER_SPR_NUM, GAS_SPR_NUM,
};

Object_Type objTypeTable[MAX_SPR_NUM];
Object 		sprObj[MAX_SPR_NUM];

int	cmdBuf[CMD_NUM] = { OFF, OFF, OFF, OFF, OFF, OFF, OFF };
int speedTable[OBJECT_TYPE_NUM] = {
	0, 0, ENEMY1_SPEED, ENEMY2_SPEED,
	ITEM1_SPEED, ITEM2_SPEED, PLAYER_SPEED, GAS_SPEED
};

byte playerImg[2][PLAYER_IMG_NUM][IMG_SIZE];	/* image buffer */
byte enemy1Img[2][ENEMY_IMG_NUM][IMG_SIZE];
byte enemy2Img[2][ENEMY_IMG_NUM][IMG_SIZE];
byte item1Img[ITEM_IMG_NUM][IMG_SIZE];
byte item2Img[ITEM_IMG_NUM][IMG_SIZE];
byte gasImg[GAS_IMG_NUM][IMG_SIZE];
byte food1Img[FOOD_IMG_NUM][IMG_SIZE];
byte food2Img[FOOD_IMG_NUM][IMG_SIZE];

byte stageImg[MAX_STAGE][STAGE_IMG_SIZE];
byte stageBoard[MAX_STAGE][STAGE_YSIZE][STAGE_XSIZE];

byte strStageImg[STR_STAGE_IMG_SIZE];
byte strScoreImg[STR_SCORE_IMG_SIZE];
byte numFont[10][NFONT_SIZE];
byte blankFont[NFONT_SIZE];

byte effectPal[768];

int foodNum, gasNum, playerId, life, score, stage, startX, startY;


/* Function prototypes -------------------------------------------- */

void initGame( void );
void closeGame( void );

void flipImage( byte *dest, byte *src );
void loadSprImage( void );
void loadStages( void );
void initObjTypeTable( void );

void keyEvent( int scancode );
void waitHitKey( void );

void initStage( void );
void closeStage( int endGame );

void drawStage( void );
void drawSprite( void );

int  openSprite( Object_Type type, int x, int y );
int  closeSprite( int id );
int  setSprite( int id );
void initSprObject( void );
void resetSpriteClock( void );

int  findId( Object_Type type, int startId );
int  getEmptyId( Object_Type type );
int  getImgNum( int frameStep );

void shadowPutsxy( int x, int y, char *str, int fore, int back );
void printNumXys( int x, int y, int size, int value );
void printLife( void );
void printScore( void );
void printStageNum( void );
void printGameOver( void );

int  play( int endGame );

int  moveObject( int id, int step );
void movePlayer( int id );
void moveEnemy1( int id );
void moveEnemy2( int id );
void moveItem1( int id );
void moveItem2( int id );
void moveGas( int id );

void changeDir( int id );
int  isWall( int id, Direction dir );
int  getStep( int id );
void incStep( int id, int step );

void processDead( int id );


/* Function definitions ------------------------------------------- */

void main( void )
{
	int endGame = FALSE;

	initGame();

	while ( !endGame )
	{
		initStage();

		endGame = play( endGame );

		if ( ++stage >= MAX_STAGE || life < 1 ) endGame = TRUE;

		closeStage( endGame );
	}

	closeGame();
}

void initGame( void )
{
	stage = 0;
	score = 0;
	life = BASIC_LIFE;

	amdInitMusic();
	amdSetVolume( 80 );
	if ( !(amdPlay( amdOpen( GAME_MUSIC, ON ) )) )
		errExit( "" );

	vgaPmSetGraphMode();
	vgaPmOpenHan( "HANGULG.FNT", "ENGLISH.FNT" );

	loadSprImage();
	loadStages();
	initObjTypeTable();
	randomize();
	setKbd( keyEvent );

	vgaPmSetDispPage( 0 );
	vgaPmSetActivePage( 1 );
	vgaPmPcxCutDisp( OPENING_PCX_FNAME, 0, 0, 0, 319, 199 );
	vgaGetAllPalette( effectPal );
	vgaFadeOutPalette( effectPal, 0 );
	vgaPmFullPageCopy( 0, 1 );
	vgaFadeInPalette( effectPal, 0 );
	waitHitKey();
	vgaFadeOutPalette( effectPal, 0 );
	vgaPmSetActivePage( 0 );
	vgaPmClearScr( 0 );
	vgaSetAllPalette( effectPal );
}

void closeGame( void )
{
	restoreKbd();
	amdCloseMusic();
	vgaSprClose();
	vgaPmCloseHan();
	vgaPmEndGraphMode();
}

void flipImage( byte *dest, byte *src )
{
	register int i, j;
	int xsize, ysize;

	xsize = *((int *)src);
	ysize = *((int *)src + 1);
	memcpy( dest, src, 4 );
	for ( j = 0; j < ysize; j++ )
		for ( i = 0; i < xsize; i++ )
			dest[ j * IMAGE_XSIZE + i + 4 ] =
				src[ j * IMAGE_XSIZE + IMAGE_XSIZE - i - 1 + 4 ];
}

void loadSprImage( void )
{
	int i;
	byte pal[768];
	FILE *fp;

	vgaPmSetDispPage( 0 );
	vgaPmSetActivePage( SPRITEPAGE );

	if ( !(fp = fopen( PALETTE_FNAME, "rb" )) )
		errExit( "Palette file open error." );

	fread( pal, 768, 1, fp );
	vgaSetAllPalette( pal );

	fclose( fp );
	if ( !(fp = fopen( IMAGE_FNAME, "rb" )) )
		errExit( "Image file open error." );

	fread( playerImg[0][0], IMG_SIZE, PLAYER_IMG_NUM, fp );
	fread( enemy1Img[0][0], IMG_SIZE, ENEMY_IMG_NUM, fp );
	fread( enemy2Img[0][0], IMG_SIZE, ENEMY_IMG_NUM, fp );
	fread( item1Img[0], IMG_SIZE, ITEM_IMG_NUM, fp );
	fread( item2Img[0], IMG_SIZE, ITEM_IMG_NUM, fp );
	fread( gasImg[0], IMG_SIZE, GAS_IMG_NUM, fp );
	fread( food1Img[0], IMG_SIZE, FOOD_IMG_NUM, fp );
	fread( food2Img[0], IMG_SIZE, FOOD_IMG_NUM, fp );

	fread( stageImg[0], STAGE_IMG_SIZE, MAX_STAGE, fp );

	fread( numFont[0], NFONT_SIZE, 10, fp );
	fread( blankFont, NFONT_SIZE, 1, fp );

	fread( strStageImg, STR_STAGE_IMG_SIZE, 1, fp );
	fread( strScoreImg, STR_SCORE_IMG_SIZE, 1, fp );

	for ( i = 0; i < PLAYER_IMG_NUM; i++ )
		flipImage( playerImg[1][i], playerImg[0][i] );

	for ( i = 0; i < ENEMY_IMG_NUM; i++ )
	{
		flipImage( enemy1Img[1][i], enemy1Img[0][i] );
		flipImage( enemy2Img[1][i], enemy2Img[0][i] );
	}

	fclose( fp );
}

void loadStages( void )
{
	int j, k;
	char buf[81];
	FILE *fp;

	if ( !(fp = fopen( STAGE_DATA_FNAME, "rt" )) )
	{
		setError( "Stage data file open error. " );
		return;
	}

	for ( k = 0; k < MAX_STAGE; k++ )
		for ( j = 0; j < STAGE_YSIZE; j++ )
		{
			fgets( buf, 80, fp );
			memcpy( &stageBoard[k][j][0], buf, STAGE_XSIZE );
		}
}

void initObjTypeTable( void )
{
	int i;
	Object_Type *otype, j;

	otype = objTypeTable;

	/* initialize objTypeTable value */

	for ( i = 1, j = FOOD1; j < OBJECT_TYPE_NUM; i++ )
	{
		*otype++ = j;
		if ( i >= sprNumTable[j] )
		{
			i = 0;
			j++;
		}
	}
}

void keyEvent( int key )
{
	int scanKey;

	scanKey = ( key & 0x7f ) + 0x100;

	if ( key & 0x80 )
	{
		switch ( scanKey )
		{
			case _KEY_Space:
				cmdBuf[CMD_GAS] = OFF;
				break;
			case _KEY_Left:
				cmdBuf[CMD_LEFT] = OFF;
				break;
			case _KEY_Right:
				cmdBuf[CMD_RIGHT] = OFF;
				break;
			case _KEY_Up:
				cmdBuf[CMD_JUMP] = OFF;
				break;
			case _KEY_PadPlus:
				cmdBuf[CMD_NEXTSTAGE] = OFF;
				break;
			case _KEY_Esc:
				cmdBuf[CMD_ESC] = OFF;
				break;
		}
	}
	else
	{
		cmdBuf[CMD_PAUSE] = OFF;

		switch ( scanKey )
		{
			case _KEY_Space:
				cmdBuf[CMD_GAS] = ON;
				break;
			case _KEY_Left:
				cmdBuf[CMD_LEFT] = ON;
				cmdBuf[CMD_RIGHT] = OFF;
				break;
			case _KEY_Right:
				cmdBuf[CMD_RIGHT] = ON;
				cmdBuf[CMD_LEFT] = OFF;
				break;
			case _KEY_Up:
				cmdBuf[CMD_JUMP] = ON;
				break;
			case _KEY_PadPlus:
				cmdBuf[CMD_NEXTSTAGE] = ON;
				break;
			case _KEY_P:
				cmdBuf[CMD_PAUSE] = ON;
				break;
			case _KEY_Esc:
				cmdBuf[CMD_ESC] = ON;
				break;
		}
	}
}

void waitHitKey()
{
	cmdBuf[CMD_PAUSE] = ON;
	while( cmdBuf[CMD_PAUSE] );
}

void initStage( void )
{
	vgaSprInit();
	vgaPmSetDispPage( 0 );
	vgaPmSetActivePage( BACKGROUNDPAGE );
	initSprObject();
	drawStage();
	vgaGetAllPalette( effectPal );
	vgaFadeOutPalette( effectPal, 0 );
	vgaPmSetActivePage( 1 );
	vgaPmFullPageCopy( 1, BACKGROUNDPAGE );
	drawSprite();
	vgaPmFullPageCopy( 0, 1 );
	vgaFadeInPalette( effectPal, 0 );
	resetSpriteClock();
}

void closeStage( int endGame )
{
	if ( endGame )
	{
		printGameOver();
		waitHitKey();
	}

	vgaGetAllPalette( effectPal );
	vgaFadeOutPalette( effectPal, 0 );
	vgaPmSetActivePage( 0 );
	vgaPmClearScr( 0 );
	vgaSetAllPalette( effectPal );
	vgaSprClose();
}

void drawStage( void )
{
	int i, j, x, y, id;
	int food1Num, food2Num, enemy1Num, enemy2Num, item1Num, item2Num,
		playerNum;
	static char *stagePic[MAX_STAGE] = {
		STAGE_PCX_FNAME, STAGE_PCX_FNAME, STAGE_PCX_FNAME,
		STAGE_PCX_FNAME,
	};

	if ( stage >= MAX_STAGE )
		errExit( "Cannot draw stage." );

	food1Num = food2Num =enemy1Num = enemy2Num = item1Num = item2Num =
		playerNum = 0;

	vgaPmClearScr( 0 );

	/* draw background picture */

	vgaPmPcxCutDisp( stagePic[stage], 96, 0, 30, 319, 199 );

	/* print score board */

	for ( i = 0; i < 22; i++ )
		vgaPmPutImage( i * (NFONT_XSIZE+1) + 2, 4, blankFont );

	vgaPmPutImage( 2, 4, strStageImg );
	vgaPmPutImage( 100, 4, strScoreImg );

	printLife();
	printScore();
	printStageNum();

	/* draw board */

	for ( j = 0; j < STAGE_YSIZE; j++ )
		for ( i = 0; i < STAGE_XSIZE; i++ )
		{
			x = i * STAGE_BLOCK_SIZE;
			y = j * STAGE_BLOCK_SIZE + 20;
			switch ( stageBoard[stage][j][i] )
			{
				case 'X' :
					vgaPmPutImage( x, y, stageImg[stage] );
					break;
				case 'A' :
					if ( ++food1Num <= FOOD1_SPR_NUM )
					{
						id = openSprite( FOOD1, x - 3, y - 10 );
						vgaSprSetCrushArea( id, 6, 6, 9, 13 );
					}
					break;
				case 'B' :
					if ( ++food2Num <= FOOD2_SPR_NUM )
					{
						id = openSprite( FOOD2, x - 3, y - 10 );
						vgaSprSetCrushArea( id, 6, 6, 9, 13 );
					}
					break;
				case 'C' :
					if ( ++enemy1Num <= ENEMY1_SPR_NUM )
						id = openSprite( ENEMY1, x - 3, y - 10 );
					break;
				case 'D' :
					if ( ++enemy2Num <= ENEMY2_SPR_NUM )
						id = openSprite( ENEMY2, x - 3, y - 10 );
					break;
				case 'E' :
					if ( ++item1Num <= ITEM1_SPR_NUM )
					{
						id = openSprite( ITEM1, x - 3, y - 10 );
						vgaSprSetCrushArea( id, 6, 6, 9, 13 );
					}
					break;
				case 'F' :
					if ( ++item2Num <= ITEM2_SPR_NUM )
					{
						id = openSprite( ITEM2, x - 3, y - 10 );
						vgaSprSetCrushArea( id, 6, 6, 9, 13 );
					}
					break;
				case 'G' :
					if ( ++playerNum <= PLAYER_SPR_NUM )
					{
						startX = x - 3;
						startY = y - 10;
						id = openSprite( PLAYER, startX, startY );
					}
					break;
				case '.' :
				case ' ' :
					break;
				default :
					errExit( "Invalid stage data encountered." );
					break;
			}
		}

	foodNum = food1Num + food2Num;

	sprObj[playerId].status = ST_SUPER;
}

void drawSprite( void )
{
	int i;

	for ( i = 0; i < MAX_SPR_NUM; i++ )
		if ( sprObj[i].status != ST_OFF )
			vgaSprPut( i, sprObj[i].x, sprObj[i].y );
}

int openSprite( Object_Type type, int x, int y )
{
	int id;

	if ( (id = getEmptyId( type )) == MAX_SPR_NUM )
		return id;

	sprObj[id].x = x;
	sprObj[id].y = y;
	sprObj[id].frameStep = 0;
	sprObj[id].jumpStep = 0;
	sprObj[id].opId = MAX_SPR_NUM;
	sprObj[id].dir = ( rand() % 2 ? LEFT : RIGHT );
	sprObj[id].frameDir = sprObj[id].dir;
	sprObj[id].type = type;
	sprObj[id].status = ST_ON;
	sprObj[id].putFlag = OFF;
	sprObj[id].frameClock = getClock();
	sprObj[id].prevClock = getClock();
	sprObj[id].stopWatch = getClock();

	setSprite( id );

	return id;
}

int closeSprite( int id )
{
	if ( id < 0 || id >= MAX_SPR_NUM )
		return FAIL;

	sprObj[id].status = ST_OFF;
	vgaSprHide( id );

	return SUCCESS;
}

int setSprite( int id )
{
	int dir, imgNum;

	if ( id < 0 || id >= MAX_SPR_NUM ) return FAIL;

	dir = sprObj[id].frameDir;

	if ( dir == LEFT )
		dir = 1;
	else if ( dir == RIGHT )
		dir = 0;
	else
		return SUCCESS;

	imgNum = getImgNum( sprObj[id].frameStep );

	switch ( sprObj[id].type )
	{
		case PLAYER:
			vgaSprSetBuf( id, playerImg[dir][imgNum] );
			break;
		case ENEMY1:
			vgaSprSetBuf( id, enemy1Img[dir][imgNum] );
			break;
		case ENEMY2:
			vgaSprSetBuf( id, enemy2Img[dir][imgNum] );
			break;
		case ITEM1:
			vgaSprSetBuf( id, item1Img[imgNum] );
			break;
		case ITEM2:
			vgaSprSetBuf( id, item2Img[imgNum] );
			break;
		case GAS:
			vgaSprSetBuf( id, gasImg[imgNum] );
			break;
		case FOOD1:
			vgaSprSetBuf( id, food1Img[0] );
			break;
		case FOOD2:
			vgaSprSetBuf( id, food2Img[0] );
			break;
		default:
			errExit( "Internal error!" );
	}
	return SUCCESS;
}

void initSprObject( void )
{
	int i;

	/* initialize sprites */

	gasNum = 0;

	for ( i = 0; i < MAX_SPR_NUM; i++ )
	{
		sprObj[i].status = ST_OFF;
		if ( (sprObj[i].type = objTypeTable[i]) == PLAYER )
			playerId = i;
	}

	for ( i = 0; i < MAX_SPR_NUM; i++ )
		vgaSprSet( i, OUTBUFFER );

	speedTable[PLAYER] = PLAYER_SPEED;
}

void resetSpriteClock( void )
{
	int i;

	for ( i = 0; i < MAX_SPR_NUM; i++ )
		if ( sprObj[i].status != ST_OFF )
			sprObj[i].prevClock = getClock();
}

int findId( Object_Type type, int startId )
{
	int i;

	for ( i = startId; i < MAX_SPR_NUM; i++ )
		if ( sprObj[i].type == type && sprObj[i].status != ST_OFF )
			return i;

	return i;
}

int getEmptyId( Object_Type type )
{
	int i;

	for ( i = 0; i < MAX_SPR_NUM; i++ )
		if ( sprObj[i].type == type && sprObj[i].status == ST_OFF )
			return i;

	return MAX_SPR_NUM;
}

int getImgNum( int frameStep )
{
	if ( frameStep < 3 ) return frameStep;
	if ( frameStep == 3 ) return 1;
	if ( frameStep == DEAD_FRAME ) return 3;
	return 0;
}

void shadowPutsxy( int x, int y, char *str, int fore, int back )
{
	vgaPmSetForeColor( back );
	vgaPmPutsxy( x+1, y+1, str );
	vgaPmSetForeColor( fore );
	vgaPmPutsxy( x, y, str );
}

void printNumXys( int x, int y, int size, int value )
{
	char str[7];
	int len, blankNum, i;

	itoa( value, str, 10 );

	if ( (len = strlen( str )) > size ) return;

	blankNum = size - len;

	for ( i = 0; i < blankNum; i++, x += (NFONT_XSIZE+3) )
		vgaPmPutImage( x, y, blankFont );

	for ( i = 0; i < size - blankNum; i++, x += (NFONT_XSIZE+3) )
		vgaPmPutImage( x, y, numFont[str[i]-'0'] );
}

void printLife( void )
{
	int i;

	for ( i = 0; i < MAX_LIFE; i++ )
	{
		if ( i < life )
			vgaPmPutImageInviCol( i * 16 + 238, 0, item2Img[1] );
		else
			vgaPmBoxFill( i * 16 + 238, 0, (i+1) * 16 + 235, 19, 0 );
	}
}

void printScore( void )
{
	printNumXys( 162, 4, 5, score );
}

void printStageNum( void )
{
	printNumXys( 62, 4, 2, stage+1 );
}

void printGameOver( void )
{
	vgaPmSetActivePage( 0 );
	shadowPutsxy( 90, 97, "G A M E   O V E R", 51, 55 );
	vgaPmSetActivePage( 1 );
}

int play( int endGame )
{
	int id = 0, tmpId, endStage = FALSE;
	long soundClock, saveClock;

	soundClock = getClock();

	while ( !endGame && !endStage )
	{
		if ( cmdBuf[CMD_ESC] )
			endGame = TRUE;
		else if ( cmdBuf[CMD_NEXTSTAGE] )
			endStage = TRUE;
		else if ( cmdBuf[CMD_PAUSE] )
		{
			saveClock = getClock();
			while ( cmdBuf[CMD_PAUSE] );
			setClock( saveClock );
		}

		for ( id=0; id < MAX_SPR_NUM && !endGame && !endStage; id++ )
		{
			if ( getClock() - soundClock > SOUND_SUSTAIN )
				nosound();

			if ( sprObj[id].status == ST_OFF ) continue;

			/* to flash at super mode */
			sprObj[id].putFlag = !sprObj[id].putFlag;

			switch ( sprObj[id].type )
			{
				case PLAYER:
					movePlayer( id );
					if ( life < 1 )
						endGame = TRUE;

					break;

				case ENEMY1:
				case ENEMY2:
					if ( sprObj[id].type == ENEMY1 )
						moveEnemy1( id );
					else
						moveEnemy2( id );

					if ( sprObj[playerId].status != ST_DEAD &&
						sprObj[playerId].status != ST_SUPER )
					{
						if ( vgaSprCheckCrush( id, playerId ) )
						{
							sound( 200 );
							soundClock = getClock();

							processDead( playerId );
						}
					}

					if ( sprObj[id].status == ST_CAUGHT )
					{
					}
					else if ( gasNum > 0 )
					{
						tmpId = 0;
						while ( (tmpId = findId( GAS, tmpId )) !=
							MAX_SPR_NUM )
						{
							vgaSprSetCrushArea( tmpId, 7, 9, 7, 9 );
							if ( vgaSprCheckCrush( id, tmpId ) &&
								sprObj[tmpId].status != ST_CAUGHT )
							{
								sprObj[id].opId = tmpId;
								sprObj[id].status = ST_CAUGHT;
								sprObj[tmpId].opId = id;
								sprObj[tmpId].status = ST_CAUGHT;
							}

							tmpId++;
						}
					}

					if ( sprObj[(sprObj[id].opId)].status == ST_OFF )
						sprObj[id].status = ST_ON;

					break;

				case ITEM1:
				case ITEM2:
					if ( sprObj[id].type == ITEM1 )
						moveItem1( id );
					else
						moveItem2( id );

					if ( sprObj[playerId].status != ST_DEAD )
					{
						if ( vgaSprCheckCrush( id, playerId ) )
						{
							sound( 1000 );
							soundClock = getClock();

							if ( sprObj[id].type == ITEM1 )
								speedTable[PLAYER] += SPEED_UP;
							else
							{
								life++;
								printLife();
							}
							score += SCORE_ITEM;
							printScore();
							closeSprite( id );
						}
					}

					break;

				case GAS:
					moveGas( id );

					if ( getClock()-sprObj[id].stopWatch > GAS_DELAY )
					{
						gasNum--;
						closeSprite( id );
					}

					break;

				case FOOD1:
				case FOOD2:
					if ( vgaSprCheckCrush( id, playerId ) )
					{
						sound( 500 );
						soundClock = getClock();

						foodNum--;
						score += ( sprObj[id].type == FOOD1 ?
							SCORE_FOOD1 : SCORE_FOOD2 );
						closeSprite( id );
						printScore();
						if ( foodNum <= 0 )
							endStage = TRUE;
					}

					break;

				default:
					break;
			}
		}

		/* update screen */

		vgaPmFullPageCopy( 0, 1 );
	}

	nosound();

	return endGame;
}
int moveObject( int id, int step )
{
	incStep( id, step );

	if ( sprObj[id].status == ST_SUPER && sprObj[id].putFlag )
		vgaSprHide( id );
	else
		vgaSprPut( id, sprObj[id].x, sprObj[id].y );

	return step;
}

void movePlayer( int id )
{
	int gasId, saveDir, step;

	if ( sprObj[id].status == ST_SUPER )
		if ( getClock() - sprObj[id].stopWatch > SUPER_MODE_DELAY )
			sprObj[id].status = ST_ON;

	step = getStep( id );
	if ( step )
		sprObj[id].prevClock = getClock();

	if ( sprObj[id].status == ST_DEAD )
	{
		if ( getClock() - sprObj[id].stopWatch > 1000 )
		{
			life--;
			printLife();
			if ( life < 1 )	return;

			closeSprite( id );
			openSprite( PLAYER, startX, startY );
			sprObj[id].status = ST_SUPER;

			return;
		}
		moveObject( id, step );
	}
	else
	{
		switch ( sprObj[id].dir )
		{
			case LEFT :
			case RIGHT :
			case CENTER :
				sprObj[id].dir = CENTER;

				if ( cmdBuf[CMD_JUMP] )
				{
					cmdBuf[CMD_JUMP] = FALSE;
					sprObj[id].dir = UP;
					sprObj[id].jumpStep = 0;
					moveObject( id, step );
				}
				else if ( cmdBuf[CMD_RIGHT] )
				{
					sprObj[id].frameDir = sprObj[id].dir = RIGHT;
					moveObject( id, step );
				}
				else if ( cmdBuf[CMD_LEFT] )
				{
					sprObj[id].frameDir = sprObj[id].dir = LEFT;
					moveObject( id, step );
				}
				else
					moveObject( id, step );

				break;

			case UP :
			case DOWN :
				moveObject( id, step );

				saveDir = sprObj[id].dir;

				if ( cmdBuf[CMD_RIGHT] )
				{
					sprObj[id].frameDir = sprObj[id].dir = RIGHT;
					moveObject( id, step );
				}
				else if ( cmdBuf[CMD_LEFT] )
				{
					sprObj[id].frameDir = sprObj[id].dir = LEFT;
					moveObject( id, step );
				}

				sprObj[id].dir = saveDir;

				break;
		}

		if ( cmdBuf[CMD_GAS] )
		{
			cmdBuf[CMD_GAS] = FALSE;
			if ( gasNum < GAS_SPR_NUM )
			{
				gasNum++;
				gasId = openSprite( GAS, sprObj[id].x, sprObj[id].y );
				sprObj[gasId].frameDir = sprObj[gasId].dir =
					( sprObj[id].frameDir == LEFT ? RIGHT : LEFT );
			}
		}
	}

	return;
}

void moveEnemy1( int id )
{
	int step;

	if ( (rand() % 100) == 0 )
		changeDir( id );

	step = getStep( id );
	if ( step )
		sprObj[id].prevClock = getClock();

	moveObject( id, step );

	return;
}

void moveEnemy2( int id )
{
	int i, gapX, gapY, saveY, move, step;

	gapX = sprObj[playerId].x - sprObj[id].x;
	gapY = sprObj[playerId].y - sprObj[id].y;

	if ( sprObj[id].dir != DOWN && sprObj[id].dir != UP )
	{
		move = rand() % 100;

		if ( move < 40 )
		{
			if ( gapY <= 0 && gapY > -40 )
				sprObj[id].frameDir = sprObj[id].dir =
					( gapX > 0 ? RIGHT : LEFT );
		}
		else if ( move < 45 )
		{
			if ( gapY < 0 )
			{
				saveY = sprObj[id].y;
				for ( i = 10; i < 20; i++ )
				{
					sprObj[id].y = saveY - i;
					if ( isWall( id, UP ) )
					{
						sprObj[id].dir = UP;
						sprObj[id].jumpStep = 0;
						break;
					}
				}
				sprObj[id].y = saveY;
			}
		}
		else if ( move < 55 )
			sprObj[id].frameDir = sprObj[id].dir =
				( move % 2 ? LEFT : RIGHT );
	}

	step = getStep( id );
	if ( step )
		sprObj[id].prevClock = getClock();

	moveObject( id, step );

	return;
}

void moveItem1( int id )
{
	int step;

	step = getStep( id );
	if ( step )
		sprObj[id].prevClock = getClock();

	moveObject( id, step );

	return;
}

void moveItem2( int id )
{
	int step;

	if ( sprObj[id].dir == CENTER )
		changeDir( id );

	step = getStep( id );
	if ( step )
		sprObj[id].prevClock = getClock();

	moveObject( id, step );

	return;
}

void moveGas( int id )
{
	int step;

	step = getStep( id );
	if ( step )
		sprObj[id].prevClock = getClock();

	moveObject( id, step );

	return;
}

int isWall( int id, Direction dir )
{
	int i, x, y, retValue;
	byte color;

	vgaPmSetActivePage( BACKGROUNDPAGE );

	x = sprObj[id].x;
	y = sprObj[id].y;

	switch ( dir )
	{
		case CENTER :
			retValue = FALSE;
			for ( i = 0; i < IMAGE_XSIZE; i++ )
			{
				color = vgaPmGetPixel( x + i, y + IMAGE_YSIZE - 1 );
				if ( (color >= 16) && (color < 32) )
				{
					retValue = TRUE;
					break;
				}
			}
			break;

		case UP :
			retValue = FALSE;
			for ( i = 0; i < IMAGE_XSIZE; i++ )
			{
				color = vgaPmGetPixel( x + i, y - 1 );
				if ( (color >= 16) && (color < 32) )
				{
					retValue = TRUE;
					break;
				}
			}
			break;

		case DOWN :
			retValue = TRUE;
			for ( i = 0; i < IMAGE_XSIZE; i++ )
			{
				color = vgaPmGetPixel( x + i, y + IMAGE_YSIZE - 1 );
				if ( (color >= 16) && (color < 32) )
				{
					retValue = FALSE;
					break;
				}
			}
			if ( retValue != FALSE )
			{
				retValue = FALSE;
				for ( i = 0; i < IMAGE_XSIZE; i++ )
				{
					color = vgaPmGetPixel( x + i, y + IMAGE_YSIZE );
					if ( (color >= 16) && (color < 32) )
					{
						retValue = TRUE;
						break;
					}
				}
			}
			break;

		case LEFT :
			retValue = TRUE;
			for ( i = 0; i < IMAGE_YSIZE; i++ )
			{
				color = vgaPmGetPixel( x, y + i );
				if ( (color >= 16) && (color < 32) )
				{
					retValue = FALSE;
					break;
				}
			}
			if ( retValue != FALSE )
			{
				retValue = FALSE;
				for ( i = IMAGE_YSIZE/3*2; i < IMAGE_YSIZE; i++ )
				{
					color = vgaPmGetPixel( x - 1, y + i );
					if ( (color >= 16) && (color < 32) )
					{
						retValue = TRUE;
						break;
					}
				}
			}
			break;

		case RIGHT :
			retValue = TRUE;
			for ( i = 0; i < IMAGE_YSIZE; i++ )
			{
				color = vgaPmGetPixel( x + IMAGE_XSIZE - 1, y + i );
				if ( (color >= 16) && (color < 32) )
				{
					retValue = FALSE;
					break;
				}
			}
			if ( retValue != FALSE )
			{
				retValue = FALSE;
				for ( i = IMAGE_YSIZE/3*2; i < IMAGE_YSIZE; i++ )
				{
					color = vgaPmGetPixel( x + IMAGE_XSIZE, y + i );
					if ( (color >= 16) && (color < 32) )
					{
						retValue = TRUE;
						break;
					}
				}
			}
			break;

		default :
			retValue = FALSE;
	}

	vgaPmSetActivePage( 1 );

	return retValue;
}

void changeDir( int id )
{
	switch ( sprObj[id].dir )
	{
		case LEFT :
			sprObj[id].dir 		= RIGHT;
			sprObj[id].frameDir = RIGHT;
			break;
		case RIGHT :
			sprObj[id].dir 		= LEFT;
			sprObj[id].frameDir = LEFT;
			break;
		case UP :
			sprObj[id].dir = DOWN;
			break;
		case DOWN :
			sprObj[id].dir = CENTER;
			break;
		case CENTER :
			sprObj[id].frameDir = sprObj[id].dir =
				( rand() % 2 ? LEFT : RIGHT );
			break;
		default:
			break;
	}
}

void incStep( int id, int step )
{
	int i, ableDir;
	static int jumpUpTable[JUMP_STEP] =	{
		6, 6, 6, 5, 5, 4, 4, 3, 3, 2, 1, 1
	};

	if ( id < 0 || id >= MAX_SPR_NUM )
		return;

	ableDir = sprAbleDir[sprObj[id].type];

	if ( sprObj[id].status == ST_CAUGHT )
	{
	}
	else if ( sprObj[id].dir == UP )
	{
		for ( i = 0; i < step; i++ )
		{
			if ( ableDir & DIR_UP )
				sprObj[id].y -= jumpUpTable[sprObj[id].jumpStep];

			if ( sprObj[id].y < 30 )
			{
				sprObj[id].y = 30;
				sprObj[id].dir = DOWN;
				break;
			}

			if ( ++sprObj[id].jumpStep >= JUMP_STEP )
			{
				sprObj[id].jumpStep = 0;
				sprObj[id].dir = DOWN;
				break;
			}
		}
	}
	else /* move Left, Right, Down */
	{
		for ( i = 0; i < step; i++ )
		{
			if ( isWall( id, sprObj[id].dir ) )
			{
				if ( sprObj[id].type == PLAYER )
					sprObj[id].dir = CENTER;
				else
					changeDir( id );
				break;
			}

			switch ( sprObj[id].dir )
			{
				case LEFT :
					if ( ableDir & DIR_LEFT ) sprObj[id].x--;

					if ( sprObj[id].x < 10 )
					{
						sprObj[id].x = 10;
						if ( sprObj[id].type == PLAYER )
							sprObj[id].dir = CENTER;
						else
							changeDir( id );
					}
					break;

				case RIGHT :
					if ( ableDir & DIR_RIGHT ) sprObj[id].x++;

					if ( sprObj[id].x > 310 - IMAGE_XSIZE )
					{
						sprObj[id].x = 310 - IMAGE_XSIZE;
						if ( sprObj[id].type == PLAYER )
							sprObj[id].dir = CENTER;
						else
							changeDir( id );
					}
					break;

				case DOWN :
					if ( ableDir & DIR_DOWN ) sprObj[id].y++;
					if ( isWall( id, DOWN ) )
						sprObj[id].dir = CENTER;
					break;

				default : break;
			}
		}

		if ( sprObj[id].dir != DOWN )
			if ( ableDir & DIR_DOWN )
				if ( !isWall( id, DOWN ) )
				{
					sprObj[id].dir = DOWN;
				}
	}

	/* increase animation frameStep */

	if ( (getClock() - sprObj[id].frameClock) * MIN_REFRESH_CYCLE >=
		BASIC_CLOCK_HZ )
	{
		if ( sprObj[id].status == ST_DEAD )
			sprObj[id].frameDir =
				(sprObj[id].frameDir == LEFT ? RIGHT : LEFT);
		else if ( sprObj[id].dir == LEFT || sprObj[id].dir == RIGHT ||
			(sprObj[id].dir == CENTER && sprObj[id].type == GAS) )
		{
			if ( ++sprObj[id].frameStep >= WALK_STEP )
				sprObj[id].frameStep = 0;
		}
		sprObj[id].frameClock = getClock();
	}

	setSprite( id );
}

int getStep( int id )
{
	return (int)( (getClock() - sprObj[id].prevClock) *
		speedTable[sprObj[id].type] / BASIC_CLOCK_HZ );
}

void processDead( int id )
{
	sprObj[id].frameStep = DEAD_FRAME;
	sprObj[id].dir = DOWN;
	sprObj[id].status = ST_DEAD;
	sprObj[id].stopWatch = getClock();
}
