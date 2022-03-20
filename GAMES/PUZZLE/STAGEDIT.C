#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "chingari.h"

#define REFRESH_RATE            (BASIC_CLOCK_HZ / 8)

#define BGM_FNAME				"STAGEDIT.AMD"

byte stage       [STAGE_SZX][STAGE_SZY];
byte imgFloor    [IMG_SIZE];
byte imgHero  [8][IMG_SIZE];
byte imgBlock    [IMG_SIZE];
byte imgSlime [2][IMG_SIZE];
byte imgKnight[2][IMG_SIZE];
byte imgGhost [2][IMG_SIZE];
byte imgStick    [IMG_SIZE];
byte imgSword    [IMG_SIZE];
byte imgCross    [IMG_SIZE];
byte imgPrincess [IMG_SIZE];
byte imgKey      [IMG_SIZE];
byte imgExit  [2][IMG_SIZE];
byte imgUp       [IMG_SIZE];
byte imgDown     [IMG_SIZE];
byte imgLeft     [IMG_SIZE];
byte imgRight    [IMG_SIZE];
byte imgKing  [2][IMG_SIZE];

byte *img[16]  = { imgFloor      , imgBlock    , imgHero[2], imgSlime[0],
				   imgKnight[0]  , imgGhost[0] , imgStick  , imgSword   ,
				   imgCross      , imgPrincess , imgKey    , imgExit[0] ,
				   imgUp         , imgDown     , imgLeft   , imgRight };

char stageFname[13];

AMD_Music *bgmMusic;

byte effectPal[768];

void main( int argc, char *argv[] );

void initStageEditor( void );
void initStageFile( void );
void setGamePalette( void );
void readImg( void );
void dispUsage( void );

void mainLoop( void );
void listImg( void );
void drawStage( void );
void putImg( int x, int y, int id );

void loadStage( int stageNum );
void saveStage( int stageNum );

void outlinePutsxy( int x, int y, int brightC, int strC, byte *str );
void shadowPutsxy( int x, int y,
				   int brightC, int strC, int shadowC, byte *str );
void waitForSec( long sec );
int  keyGet( void );

void main( int argc, char *argv[] )
{
	if ( argc == 2 )
		strcpy( stageFname, argv[1] );
	else strcpy( stageFname, STAGE_FNAME );

	initStageEditor();
	mainLoop();

	amdCloseMusic();
	vgaStdCloseHan();
	vgaStdEndGraphMode();
}

void initStageEditor( void )
{
	vgaStdSetGraphMode();
	vgaStdOpenHan( "HANGULG.FNT", "ENGLISH.FNT" );

	amdInitMusic();
    amdSetVolume( 70 );

    bgmMusic = amdOpen( BGM_FNAME, ON );
	amdPlay( bgmMusic );

	setGamePalette();
	readImg();
	dispUsage();

	setGamePalette();
	loadStage( 0 );
	listImg();
	drawStage();
}

void initStageFile( void )
{
	register i, j;
	int size;
	FILE *stageFile;

	stageFile = fopen( stageFname , "wb" );
	if ( stageFile == NULL )
		errExit( "Can't make stage data file!" );

	size = STAGE_SZX*STAGE_SZY * MAX_STAGE;

	for( i = 0 ; i < size ; i++ )
		putc( IMG_FLOOR, stageFile );

	fclose( stageFile );
}

void setGamePalette( void )
{
	FILE *fpal;

	byte pal[768];

	fpal = fopen( PALETTE_FNAME, "rb" );

	fread( pal, 768, 1, fpal );
	vgaSetAllPalette( pal );
}

void readImg( void )
{
	register i,j;
	char str[100];
	FILE *fimg;

	fimg = fopen( IMG_FNAME, "rb" );

	if ( fimg == NULL )
	{
		strcpy( str , IMG_FNAME );
		strcat( str , " doesn't exist!");
		errExit( str );
	}

	fread( imgFloor, IMG_SIZE, 1, fimg );
	fread( imgBlock, IMG_SIZE, 1, fimg );
	fread( imgHero[0], IMG_SIZE, 1, fimg );
	fread( imgHero[1], IMG_SIZE, 1, fimg );
	fread( imgHero[2], IMG_SIZE, 1, fimg );
	fread( imgHero[3], IMG_SIZE, 1, fimg );
	fread( imgHero[4], IMG_SIZE, 1, fimg );
	fread( imgHero[5], IMG_SIZE, 1, fimg );
	fread( imgHero[6], IMG_SIZE, 1, fimg );
	fread( imgHero[7], IMG_SIZE, 1, fimg );
	fread( imgSlime[0], IMG_SIZE, 1, fimg );
	fread( imgSlime[1], IMG_SIZE, 1, fimg );
	fread( imgKnight[0], IMG_SIZE, 1, fimg );
	fread( imgKnight[1], IMG_SIZE, 1, fimg );
	fread( imgGhost[0], IMG_SIZE, 1, fimg );
	fread( imgGhost[1], IMG_SIZE, 1, fimg );
	fread( imgStick, IMG_SIZE, 1, fimg );
	fread( imgSword, IMG_SIZE, 1, fimg );
	fread( imgCross, IMG_SIZE, 1, fimg );
	fread( imgPrincess, IMG_SIZE, 1, fimg );
	fread( imgKey, IMG_SIZE, 1, fimg );
	fread( imgExit[0], IMG_SIZE, 1, fimg );
	fread( imgExit[1], IMG_SIZE, 1, fimg );
	fread( imgUp, IMG_SIZE, 1, fimg );
	fread( imgDown, IMG_SIZE, 1, fimg );
	fread( imgLeft, IMG_SIZE, 1, fimg );
	fread( imgRight, IMG_SIZE, 1, fimg );

	fread( imgKing[0], IMG_SIZE, 1, fimg );
	fread( imgKing[1], IMG_SIZE, 1, fimg );


	fclose( fimg );
}

void dispUsage( void )
{
	int color;
    byte *title = "Ã·´é‰·ºŸi Àx´a¬á Ìe•©·¡ v1.3";

	color = vgaStdGetForeColor();

	/* LOGO DISPLAY */
	vgaStdClearScr( BLACK );
	vgaGetAllPalette( effectPal );

	shadowPutsxy( vgaStdGetCenterOfStr( title ), 100,
				  LIGHTMAGENTA, MAGENTA, WHITE, title );

	waitForSec( 5 );
	vgaFadeOutPalette( effectPal, 0 );

	vgaStdClearScr( BLACK );
	vgaStdSetForeColor( LIGHTBLUE );
    vgaStdPutsxyC( 50, "‰A·±¹A¸b   : ‹±¶wº…           " );
	vgaStdPutsxyC( 70, "œa·¡§aœáŸ¡ : VGASTD œa·¡§aœáŸ¡" );
	vgaFadeInPalette( effectPal, 0 );
	waitForSec( 3 );

	vgaFadeOutPalette( effectPal, 0 );
	vgaStdClearScr( BLACK );
	vgaStdSetForeColor( YELLOW );
	vgaStdPutsxyC( 30,  "[ ¹¡¹·¤ó ]" );
	vgaStdPutsxyC( 50,  "KEY-PAD e ¬a¶w" );
	vgaStdPutsxyC( 70,  "¬wÐa¹Á¶ ÑÁ¬iÎaÇ¡ : ¶‘»¢·¥”a   " );
	vgaStdPutsxyC( 90,  "+ Ç¡              : ”a·q Ìe·a¡" );
	vgaStdPutsxyC( 110, "- Ç¡              : ·¡¸å Ìe·a¡" );
	vgaStdPutsxyC( 130, "7 ¤å Ç¡           : ´| ‹aŸ±    " );
	vgaStdPutsxyC( 150, "9 ¤å Ç¡           : –õ ‹aŸ±    " );
	vgaStdPutsxyC( 170, "SPACE Ç¡          : ‹aŸ± ‘½‹¡  " );

	vgaFadeInPalette( effectPal, 0 );
	waitForSec( 10 );
	vgaFadeOutPalette( effectPal, 0 );
	vgaStdClearScr( BLACK );
	vgaStdSetForeColor( YELLOW );
	vgaStdPutsxyC( 20, "[ ‹AÃ¢ ]" );
	vgaStdSetForeColor( 15 );
	vgaStdPutsxy( 0, 50, "Ëb¥iÐe ‹AÃ¢·e ´ô‰¡, " );
	vgaStdPuts( "Ìe·e ”a·qÌe, ·¡¸åÌe —w·a¡ ñ‹©˜ ¸a•·¸á¸w–A¡a, " );
	vgaStdPuts( "ESC µb¯¡ ¸a•··a¡ ¸á¸wÐe–á {…”a." );
	vgaStdPuts( "\nÁ· 30ÌeŒa»¡ ¹A¸b Ði ® ·¶”a.");

	vgaStdPutsxyC( 170, " õ»¥ Ìe·i  e—a¯¡‹©!" );
	vgaFadeInPalette( effectPal, 0 );
	waitForSec( 10 );
	vgaFadeOutPalette( effectPal, 0 );
	vgaStdClearScr( BLUE );
	vgaSetAllPalette( effectPal );

	vgaStdSetForeColor( color );
}

void mainLoop( void )
{
	static long prevClock = 0;

	int whichImg = 0;
	int stageNum = 0;
	int x = STAGE_SZX / 2, y = STAGE_SZY / 2;
	int cursFlag = TRUE;
	int k;

	while( TRUE )
	{
		if ( getClock() - prevClock > REFRESH_RATE )
		{
			if ( cursFlag == TRUE )
			{
					vgaStdBox( STAGE_START_PTX+x*IMG_SZX,
							   STAGE_START_PTY+y*IMG_SZY,
							   STAGE_START_PTX+(x+1)*IMG_SZX-1,
							   STAGE_START_PTY+(y+1)*IMG_SZY-1,
							   BRIGHTWHITE );
				cursFlag = FALSE;
			} else if ( cursFlag == FALSE )
			{
				putImg( x, y, whichImg );
				cursFlag = TRUE;
			}
			prevClock = getClock();
		}

		k = keyGet();

		switch( k ) {
			case KEY_Up :
			case '8' :
				if ( y-1 == -1 ) break;
				putImg( x, y, stage[x][y] );
				--y;
				break;

			case KEY_Down :
			case '2' :
				if ( y+1 == STAGE_SZY ) break;
				putImg( x, y, stage[x][y] );
				++y;
				break;

			case KEY_Left :
			case '4' :
				if ( x-1 == -1 ) break;
				putImg( x, y, stage[x][y] );
				--x;
				break;

			case KEY_Right :
			case '6' :
				if ( x+1 == STAGE_SZX ) break;
				putImg( x, y, stage[x][y] );
				++x;
				break;

			case KEY_PgUp : /* NEXT IMG KEY */
			case '9' :
				if ( whichImg == IMG_RIGHT ) whichImg = 0;
				else ++whichImg;
					break;

			case KEY_Home : /* PREV IMG KEY */
			case '7' :
				if ( whichImg == 0 ) whichImg = IMG_RIGHT;
				else --whichImg;
				break;

			case KEY_Space : /* PUT IMG KEY */
			case KEY_Enter :
				stage[x][y] = whichImg;
				break;

			case KEY_PadMinus : /* PREV STAGE KEY */
				saveStage( stageNum );
				if ( stageNum != 0 )
				{
					stageNum--;
                    vgaStdPrintfxy( 100, 100, "NEXT STAGE %d", stageNum+1 );
                    waitForSec( 1 );
                    loadStage( stageNum );
					drawStage();
				}
				break;

			case KEY_PadPlus: /* NEXT STAGE KEY */
				saveStage( stageNum );
				if ( stageNum != ( MAX_STAGE-1 ) )
				{
					stageNum++;
                    vgaStdPrintfxy( 100, 100, "NEXT STAGE %d", stageNum+1 );
                    waitForSec( 1 );
					loadStage( stageNum );
					drawStage();
				}
				break;

			case KEY_Esc:
				saveStage( stageNum );
				return;
		}
	}
}


void listImg( void )
{
	int i;

	for( i = IMG_FLOOR; i <= IMG_RIGHT ; i++ )
	{
		vgaStdBox( i * IMG_SZX, 0, (i + 1) * IMG_SZX, IMG_SZY, WHITE );
		vgaStdPutImageInviCol( i * IMG_SZX, 0, img[i] );
	}
}

void drawStage( void )
{
	register i, j;

	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ; j < STAGE_SZX ; j++ )
			putImg( j, i, stage[j][i] );
}

void putImg( int x, int y, int id )
{
	vgaStdPutImageInviCol( STAGE_START_PTX+x*IMG_SZX,
						   STAGE_START_PTY+y*IMG_SZY,
						   img[IMG_FLOOR] );
	vgaStdPutImageInviCol( STAGE_START_PTX+x*IMG_SZX,
						   STAGE_START_PTY+y*IMG_SZY,
						   img[id] );
}

void loadStage( int stageNum )
{
	register i, j;
	FILE *stageFile;

	stageFile = fopen( stageFname, "rb" );
	if ( stageFile == NULL )
	{
		initStageFile();
		stageFile = fopen( stageFname, "rb" );
	}

	fseek( stageFile, STAGE_SZX*STAGE_SZY*stageNum, SEEK_SET );

	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ; j < STAGE_SZX ; j++ )
			stage[j][i] = getc( stageFile );

	fclose( stageFile );
}

void saveStage( int stageNum )
{
	register i, j;
	FILE *stageFile;

	stageFile = fopen( stageFname, "r+b" );

	fseek( stageFile, STAGE_SZX*STAGE_SZY*stageNum, SEEK_SET );

	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ;j < STAGE_SZX ; j++ )
			putc( stage[j][i], stageFile );

	fclose( stageFile );
}

void outlinePutsxy( int x, int y, int brightC, int strC, byte *str )
{
	int color;

	color = vgaStdGetForeColor();

	vgaStdSetForeColor( brightC );
	vgaStdPutsxy( x, y-1, str );
	vgaStdSetForeColor( brightC );
    vgaStdPutsxy( x, y+1, str );
    vgaStdSetForeColor( brightC );
	vgaStdPutsxy( x-1, y, str );
	vgaStdSetForeColor( brightC );
    vgaStdPutsxy( x+1, y, str );
    vgaStdSetForeColor( brightC );
    vgaStdPutsxy( x-1, y-1, str );
	vgaStdSetForeColor( brightC );
    vgaStdPutsxy( x+1, y-1, str );
	vgaStdSetForeColor( brightC );
	vgaStdPutsxy( x-1, y+1, str );
	vgaStdSetForeColor( brightC );
	vgaStdPutsxy( x+1, y+1, str );

	vgaStdSetForeColor( strC );
	vgaStdPutsxy( x, y, str );

	vgaStdSetForeColor( color );
}

void shadowPutsxy( int x, int y,
				   int brightC, int strC, int shadowC, byte *str )
{
	int color;

	color = vgaStdGetForeColor();

	vgaStdSetForeColor( brightC );
	vgaStdPutsxy( x-1, y-1, str );
	vgaStdSetForeColor( shadowC );
	vgaStdPutsxy( x+1, y+1, str );
	vgaStdSetForeColor( strC );
	vgaStdPutsxy( x, y, str );

	vgaStdSetForeColor( color );
}

int keyGet( void )
{
	if ( keyPressed() ) return keyRead();
	return FALSE;
}

void waitForSec( long sec )
{
	long start, end;

	start = getClock();
	end   = getClock();


	while ( ( end - start ) < sec * BASIC_CLOCK_HZ )
	{
		if ( keyGet() ) return;
		end = getClock();
	}
}
