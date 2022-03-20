#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chingari.h"

#define REFRESH_RATE            3

#define OPENING_MUSIC_FNAME     "OPENING.AMD"
#define STAGE_MUSIC_FNAME       "STAGE.AMD"
#define STAGECLEAR_MUSIC_FNAME  "CLEAR.AMD"
#define ENDING_MUSIC_FNAME      "ENDING.AMD"
#define STORY_OPEN_MUSIC_FNAME  "STRYOPEN.AMD"
#define STORY_END_MUSIC_FNAME   "STRYEND.AMD"

#define DIALOGUE_SX				( 1 + IMG_SZX + 2 )
#define DIALOGUE_SY             2
#define DIALOGUE_EX				( VGA256XSIZE - 2 )
#define DIALOGUE_EY				( STAGE_START_PTY - 3 )

#define ITEM_BOX_PTX            (VGA256XSIZE-IMG_SZX*2)/2
#define ITEM_BOX_PTY			5
#define STAGE_NUM_PTX			5
#define STAGE_NUM_PTY			5
#define SCORE_NUM_PTX           210
#define SCORE_NUM_PTY			5

#define R                       0   /* don't change this value */
#define G                       1   /* and this */
#define B                       2   /* and this too */

#define MOVE					TRUE
#define DONTMOVE				FALSE

#define MOVE_SOUND				0
#define CRASH_SOUND				1
#define OPENEXIT_SOUND          2

#define NO_MUSIC				0
#define OPENING_MUSIC			1
#define STAGE_MUSIC				2
#define STAGECLEAR_MUSIC    	3
#define ENDING_MUSIC			4
#define STORY_OPEN_MUSIC        5
#define STORY_END_MUSIC         6

#define MAX_MUSIC_VOLUME        70

#define UP_SIDE                 0
#define DOWN_SIDE               2
#define LEFT_SIDE               4
#define RIGHT_SIDE              6

#define AFTER_STAGE_KEY			'+'
#define BEFORE_STAGE_KEY		'-'
#define CLEAR_GAME_KEY          '*'
#define EXIT_OPENING_KEY		KEY_Esc

/*----------------- ¸åµe ¥e®(global variable) ¬å´å -------------*/

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

byte *img[16]  = { imgFloor      , imgBlock    , imgHero[0], imgSlime[0],
				   imgKnight[0]  , imgGhost[0] , imgStick  , imgSword   ,
				   imgCross      , imgPrincess , imgKey    , imgExit[0] ,
				   imgUp         , imgDown     , imgLeft   , imgRight };

int heroPosX = 0,
	heroPosY = 0,
	heroSide = DOWN_SIDE,
	heroItem = FALSE,
	heroKey  = FALSE,
	stageCleared = FALSE;

int stageNo = 0;
int skipOpening = FALSE;

AMD_Music *music[MAX_MUSIC];
AMD_Music *currentMusic;

byte effectPal[768];


/*---------------------- Ðq® ¬å´å ----------------------*/

void main( int argc, char *argv[] );

void initMusic( void );
void initChingari( void );
void mainOpening( void );
void mainEnding( void );
void storyOpening( void );
void storyEnding( void );
void setGamePalette( void );
void readStage( int stageNum );
void readImg( void );

void mainLoop( void );
void refreshMonsters( void );
void moveHero( int oldX, int oldY, int heroPosX, int heroPosY );
int  checkEvent( int x, int y );
int  checkEventMain( int x, int y );
int  match( int item, int enemy );

void dispScoreBoard( int value );
void dispStageBoard( void );
void dispItemBox( void );
void drawStage( void );
void putImg( int x, int y, byte *id );

void gameSound( int choice );
void gameMusic( int choice );
int  replayStage( void );

void outlinePutsxy( int x, int y, int brightC, int strC, byte *str );
void shadowPutsxy( int x, int y,
				   int brightC, int strC, int shadowC, byte *str );
void putSpeaker( byte *img );
void dialogue( int who, char *msg );
int  waitForSec( long sec );
void delayMiliSec( long miliSec );
int  fadeInMusicAndPal( byte *pal, int start );
int  fadeOutMusicAndPal( byte *pal, int start );
void incRGBPalette( int rgb, int value );
void decRGBPalette( int rgb, int value );
int  keyGet( void );

/*---------------------- ¯©¹A Ðq® ŠÑe ----------------------*/

void main( int argc, char *argv[] )
{
	byte *pal, *savePal;

	pal = ( byte * ) malloc( 768 );
	savePal = ( byte * ) malloc( 768 );
	vgaGetAllPalette( savePal );

	if ( argc == 2 && *argv[1] == ':' ) skipOpening = TRUE;
	/* µ³­e·a¡ : Ÿi º´ö·i ‰w¶ µ¡Ïa“· ¬—œ¢ */

	initChingari();
	vgaGetAllPalette( pal );
	vgaFadeOutPalette( pal, 0 );

	while ( TRUE )
	{
		heroItem = FALSE;
		heroKey = FALSE;
		stageCleared = FALSE;

		readStage( stageNo );
		drawStage();

		dispStageBoard();
		dispScoreBoard( 0 );
		dispItemBox();
		gameMusic( STAGE_MUSIC );
		fadeInMusicAndPal( pal, 0 );
		mainLoop();
		if( stageCleared == TRUE )
		{
			vgaStdPutsxyC( 110, "STAGE CLEARED!" );
			gameMusic( STAGECLEAR_MUSIC );
			waitForSec( 3 );
			fadeOutMusicAndPal( pal, 0 );

			if( stageNo + 1 == MAX_STAGE )
			/* ‰A·±·i 30ÌeŒa»¡ µÅ‰iÐ–·i ‰w¶ÀáŸ¡ */
			{
				storyEnding();
				break;
			}

			++stageNo;
		}
		else
		{
			if ( replayStage() == FALSE ) break;
			fadeOutMusicAndPal( pal, 0 );
		}
	}

	mainEnding();
	vgaSetAllPalette( savePal );

	free( pal );
	free( savePal);
}

void initMusic( void )
{
	amdInitMusic();
    amdSetVolume( MAX_MUSIC_VOLUME );

	if ( !(music[OPENING_MUSIC]    = amdOpen( OPENING_MUSIC_FNAME, ON )) ||
		 !(music[STAGE_MUSIC]      = amdOpen( STAGE_MUSIC_FNAME, ON )) ||
		 !(music[STAGECLEAR_MUSIC] = amdOpen( STAGECLEAR_MUSIC_FNAME, ON )) ||
		 !(music[ENDING_MUSIC]     = amdOpen( ENDING_MUSIC_FNAME, ON )) ||
         !(music[STORY_OPEN_MUSIC] = amdOpen( STORY_OPEN_MUSIC_FNAME, ON )) ||
		 !(music[STORY_END_MUSIC]  = amdOpen( STORY_END_MUSIC_FNAME, ON )) )

		 errExit("");
}

void initChingari( void )
{
	initMusic();

	vgaStdSetGraphMode();
	vgaStdOpenHan( "HANGULG.FNT", "ENGLISH.FNT" );

	setGamePalette();
	readImg();

	if ( !skipOpening )
	{
		gameMusic( OPENING_MUSIC );
		mainOpening();
		storyOpening();
	}
	setGamePalette();
}

void mainOpening( void )
{
	int color;
    byte *title = "Ã·´é‰·ºŸi Àx´a¬á v1.5";

	color = vgaStdGetForeColor();

	/* ¡‰¡ÑÁ¡e Â‰b */
	vgaStdClearScr( BLACK );
	vgaGetAllPalette( effectPal );

	shadowPutsxy( vgaStdGetCenterOfStr( title ), 100,
				  LIGHTMAGENTA, MAGENTA, WHITE, title );

	if ( waitForSec( 5 ) == EXIT_OPENING_KEY ) return;
	vgaFadeOutPalette( effectPal, 0 );

	/* ¹A¸b¸a & ¹A¸bÊ‰ Â‰b */
	vgaStdClearScr( BLACK );
	vgaStdSetForeColor( LIGHTBLUE );
    vgaStdPutsxyC( 10 , "‰A·±¹A¸b   : ‹±¶wº…             " );
    vgaStdPutsxyC( 30 , "œa·¡§aœáŸ¡ : VGASTD œa·¡§aœáŸ¡  " );
    vgaStdPutsxyC( 50 , "·q´b : ‰·ˆ¸aža¯©(HITEL,ÀåŸ¡´e) " );
    vgaStdPutsxyC( 70 , "¹½·e ·q´b·i ´ó¡—a Ðº¯¥ ¦…—iA" );
    vgaStdPutsxyC( 90 , "»¥¯±·a¡ ˆq¬a—aŸ³“¡”a.          " );
    vgaStdPutsxyC(130 , "¥¥ ‰A·±·e <ÇaŸ¡µAž· ¬÷¤>œa“e ‰A·±·i" );
	vgaStdPutsxyC(150 , "IBM¶w·a¡ ¸¹A¸b Ðe ˆõ·±·i ¤jÑa–““¡”a." );

	vgaFadeInPalette( effectPal, 0 );
    if ( waitForSec( 5 ) == EXIT_OPENING_KEY ) return;

	/* ¹¡¹·¤ó Â‰b */
	vgaFadeOutPalette( effectPal, 0 );
	vgaStdClearScr( BLACK );
	vgaStdSetForeColor( YELLOW );
	vgaStdPutsxyC( 50 , "[¹¡¹·¤ó]" );
	vgaStdPutsxyC( 70 , "¬wÐa¹Á¶ Ç¡ : º·¥‰··i ¶‘»¢·¥”a        " );
	vgaStdPutsxyC( 90 , "+ Ç¡        : ”a·q Ìe·a¡              " );
	vgaStdPutsxyC( 110, "- Ç¡        : ·¡¸å Ìe·a¡              " );
	vgaStdPutsxyC( 130, "ESC Ç¡      : ‹a Ìe·i ”a¯¡Ðaˆáa {…”a" );
	vgaFadeInPalette( effectPal, 0 );
	if ( waitForSec( 10 ) == EXIT_OPENING_KEY ) return;

    /* ‰A·± ‹AÃ¢ Â‰b */
	vgaFadeOutPalette( effectPal, 0 );
	vgaStdClearScr( BLACK );
	vgaStdSetForeColor( YELLOW );
	vgaStdPutsxyC( 0 , "[‰A·±‹AÃ¢]" );
	vgaStdSetForeColor( BRIGHTWHITE );
	vgaStdPutsxy( 0, 20 , " â·i® ·¶“e ˆõ·e Äi, »¡Ìw·¡, ¯³¸aˆa· ­Aˆa»¡·¡”a. " );
	vgaStdPuts( "Äi·e Ð‰©·i, »¡Ìw·¡“e ¯iœa·±·i, ¯³¸aˆa“e ·Aw·i ·¡‹©® ·¶”a." );
	vgaStdPuts( "‰A·±· ¡¢Îa“e ·¡—i ŠA¢‰—i·i ÌaŠAÐ¬á µi®AŸi  â‰¡, ÈiÂ‰Š¡ " );
    vgaStdPuts( "ÈiÂ‰Ða“e ˆõ·¡”a.\n" );
	vgaStdPuts( "ÂA¹·¸â·a¡“e Ã·´é‰·ºŸi £¡Š—µA¬á ŠÐa“e ˆõ·¡ ¡¢¸â·¡”a.\n" );

    vgaStdPuts( "Ñe¸ Á· 30Ìe·a¡ Š¬÷–A´á ·¶”a." );
	vgaStdSetForeColor( LIGHTCYAN );
	vgaStdPutsxyC( 170, "”w¯¥· ˆåÊŸi §¥”a!" );
	vgaFadeInPalette( effectPal, 0 );
	if ( waitForSec( 10 ) == EXIT_OPENING_KEY ) return;

	fadeOutMusicAndPal( effectPal, 0 );
	vgaStdClearScr( BLACK );

	vgaStdSetForeColor( color );
	free( effectPal );
}

void storyOpening( void )
{
    int i, j, x, y;
	int heroX, heroY,
		princessX, princessY,
		kingX, kingY;

    heroX = STAGE_SZX / 2 - 1;
	heroY = STAGE_SZY / 2;
	princessX = heroX + 1;
	princessY = heroY;
	kingX = princessX;
    kingY = 1;

    for( y = 0 ; y < STAGE_SZY ; y++ )
        for( x = 0 ; x < STAGE_SZX ; x++ )
			stage[x][y] = IMG_FLOOR;

	randomize();
    setGamePalette();
	vgaStdBoxFill( 0, 0, VGA256XSIZE-1, VGA256YSIZE-1, BLACK );
    vgaGetAllPalette( effectPal );
	vgaFadeOutPalette( effectPal, 0 );

	vgaStdPutsxyC(  60, "µ•i µ•i Ðe µ•iµA..." );
	vgaStdPutsxyC(  80, "´á“a µ×Š‚µA Ã·´é·¡œa“e ·¡Ÿq·" );
	vgaStdPutsxyC( 100, "µ«e ‰·ºˆa ¬i´vºa." );
	vgaStdPutsxyC( 120, "´á“ai ‹a µ×Š‚·i »¡a”å" );
	vgaStdPutsxyC( 140, "˜á•©·¡ ‹¡¬aµÁ ¬aœwµA ¨a¹v”s“¡”a." );
	vgaStdPutsxyC( 180, "·¡´¡‹¡“e µa‹¡¬á¦Èá ¯¡¸b·¡µ¶a..." );
	vgaFadeInPalette( effectPal, 0 );
	if ( waitForSec( 5 ) == EXIT_OPENING_KEY ) return;
	vgaFadeOutPalette( effectPal, 0 );

	/* ¯aÉ¡Ÿ¡ µ¡Ïa“· ÑÁ¡e  e—i‹¡ */
	vgaStdBoxFill( 0, 0, VGA256XSIZE-1, STAGE_START_PTY-1, WHITE );
	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ; j < STAGE_SZX ; j++ )
			putImg( j, i, imgFloor );

	for( i = 0 ; i < 30 ; i++ )
	{
		x = rand() % STAGE_SZX;
		y = rand() % STAGE_SZY;
        if ( x == princessX || y == princessY ) continue;
		putImg( x, y, imgBlock );
	}

	putImg( heroX, heroY, imgHero[DOWN_SIDE] );
	putImg( princessX, princessY, imgPrincess );
	gameMusic( STORY_OPEN_MUSIC );
	fadeInMusicAndPal( effectPal, 0 );

	/* ”¬a Â‰b */
	dialogue( IMG_HERO,     "Ã·´é ‰·º!" );
	dialogue( IMG_PRINCESS, "µá¶a? º…!");
	dialogue( IMG_HERO,     "‹a”“e µá ·¡Ÿq·¡ Ã·´é·¡»¡?" );
	dialogue( IMG_PRINCESS, "¹A ·¡Ÿq·¡  qµA ´g—i´á¶a?" );
	dialogue( IMG_PRINCESS, "”w¯¥·¡ ¶¥Ðe”a¡e ¤aŽ‰A¶a." );
	dialogue( IMG_PRINCESS, "Â…Ð·,®…¸a, i¸a,˜â®…,µ·‘a,»¥¯©" );
	dialogue( IMG_PRINCESS, "¬¡¥, å¡, a•¥a,¥A¯¥¸á,®»¡,Ñ®" );
	dialogue( IMG_PRINCESS, "»¥‰w,·eµw,·E¸÷,£¡¬å(¸á¸a Ã¥Š—i)" );
	dialogue( IMG_PRINCESS, "‰wÓ,¹·®‚,­¡µw,¬å¶‚,»¡·e,Óµe" );
	dialogue( IMG_PRINCESS, "®q,·eÓ,£¡¬å,£¥Ó,£¡®‚,­¡Ñe" );
	dialogue( IMG_PRINCESS, "·e¸÷,­¡Ó,·e®‚,Ó‰w(¸á¸a Ò¤—i)" );
	dialogue( IMG_PRINCESS, "£¡¬å,´®‚,·EÓ,ºµe(¸á¸a ¬å¤—i)" );
	dialogue( IMG_HERO,     "·a´b!...¸qŒe! ‹a e! ‹a e!" );
	dialogue( IMG_PRINCESS, "´á á á! ´a»¢  g·¡ q´v´á¶a!" );
	dialogue( IMG_HERO,     "´a“¡! e »¡‹q ·¡Ÿq·¡  qµA —aµ¡" );
	dialogue( IMG_PRINCESS, "¶Ÿ¡ ¿¥Ða‰A Ç¡¯aa Ðe¤å Ðiœ¶a?" );
	dialogue( IMG_HERO,     "Ç¡¯a?! ·q..‹aœ,‹aˆá ¹½»¡..." );
	putImg( heroX, heroY, imgFloor );
	putImg( heroX, heroY, imgHero[RIGHT_SIDE] );
	gameSound( MOVE_SOUND );
	heroSide = RIGHT_SIDE;
	dialogue( IMG_HERO,     "Ã·´áŸ¡..·¡Ÿ¡µÁ~~" );
	dialogue( IMG_PRINCESS, "·w? ·¡‰A ¢¯e ­¡Ÿ¡ºa?!" );
	dialogue( IMG_HERO,     "­¡Ÿ¡“e ¢¯e­¡Ÿ¡! e ´e—iŸ¡“e•A." );
	dialogue( IMG_HERO,     "‹aˆõ¥¡”a...¸a, ·¡Ÿ¡¡ µ¡œaŠ." );

	/* µ× Â‰Ñe */
	for( i = kingY ; i < heroY ; i++ )
	{
		putImg( kingX, i - 1, imgFloor );
		gameSound( MOVE_SOUND );
		putImg( kingX, i, imgKing[i%2] );
		delayMiliSec( 500 );
	}

	kingY = i - 1;

	dialogue( IMG_KING,     "·¡ ‘±—i!" );
	dialogue( IMG_PRINCESS, "´á á! ´a¨a!" );
	dialogue( IMG_PRINCESS, "ºÀ‚ x‰A ·¡œå˜ aÈaa¶a!" );
	dialogue( IMG_KING,     "ˆa ¸á ‘a¬â‰Á  ea»¡  iœ–»¡!" );
	dialogue( IMG_PRINCESS, "´a¨a! e º…·¡µ¡¨i ¬aœwÐ¶a!" );
	dialogue( IMG_KING,     "¢¡! ¬a..¬a¬a ¬aœw!" );
	dialogue( IMG_PRINCESS, "‘A! ¬aœwÐ¶a!" );
	dialogue( IMG_KING,     "´é³¡Š! ¬aœwˆ{·e ­¡Ÿ¡ÐaŠ·¶‘A!" );
	dialogue( IMG_HERO,     "¸w·¥! ´a“¡Àq, ÍÐa! »¥¸÷Ð¶a!" );
	dialogue( IMG_KING,     "·¡ ‘±—i,   a¤ó·a¡..." );
	putImg( kingX, kingY, imgKing[(kingY+1)%2] );
	gameSound( CRASH_SOUND );
	dialogue( IMG_KING,     "Èa¤i¶ œa‹©¤a Ò a¬a “¡¤a¬a µ±" );
	dialogue( IMG_PRINCESS, "´a´b~~~¡±·¡ ¹A õ”¡~~~" );
	for( j = princessX + 1; j < STAGE_SZX; j++ )
	{
		putImg( j - 1, princessY, imgFloor );
		gameSound( MOVE_SOUND );
		putImg( j, princessY, imgPrincess );
		delayMiliSec( 500 );
	}

	princessX = j - 1;

	putImg( kingX, kingY, imgKing[(kingY)%2] );
	gameSound( CRASH_SOUND );
	dialogue( IMG_KING,     "·¡´³!  a¤ó· ´é·q·¡µa!" );
	/*  a¤ó Ëb® Òa‰Á */

	for( i = 0; i < 63; i++ )
	{
		incRGBPalette( B, 1 );
		delayMiliSec( 5 );
	}

	vgaStdPutImageInviCol( STAGE_START_PTX + princessX * IMG_SZX,
						   STAGE_START_PTY + princessY * IMG_SZY,
						   imgBlock );

	vgaSetAllPalette( effectPal );
	gameSound( OPENEXIT_SOUND );
	dialogue( IMG_KNIGHT,   "Òa‰Á·q:¶»¡»¢~~~~~(´é·q´á“e ­¡Ÿ¡)" );
	dialogue( IMG_HERO,     "´u! ‰·º!" );
	dialogue( IMG_PRINCESS, ".............");
	dialogue( IMG_KING,     "ÐaÐaÐa! ‘A‘±µA‰A ¶w‹¡ˆa ·¶”a¡e" );
	dialogue( IMG_KING,     "‰·ºŸi ŠÐ¥Áœa! ÓaÓaÓa" );

	heroSide = RIGHT_SIDE;
	for( j = heroX + 1 ; j < princessX ; j++ )
		moveHero( j - 1, heroY, j, heroY );

	heroX = j - 1;
	dialogue( IMG_HERO,     "‰·º! ¸qŒe e ‹¡”aa!" );
    dialogue( IMG_HERO,     "ˆa œa·¡Èa ¦‰¡...");
    dialogue( IMG_KNIGHT,   "Òa‰Á·q:Ã¢Ã¢(œa·¡Èa ¦‰¦›·¡“e ­¡Ÿ¡)" );
    dialogue( IMG_HERO,     "·w?" );
	dialogue( IMG_HERO,     "·¡œé®ˆa! ¸åÑa ‘¢´aŸ¡»¡ ´g´a!");
	putImg( heroX, heroY, imgHero[LEFT_SIDE]);
	gameSound( MOVE_SOUND );
	dialogue( IMG_HERO,     "·¡œå,¬a´bÐe  a¤ó·i!" );
	dialogue( IMG_HERO,     "¸a¯¥· ˜i¡ Ì{§·®  e—i ·© ·¶a!");
	dialogue( IMG_HERO,     "”w¯¥·¡ ·¡œá‰¡•¡ ´a¤á»¡·¥ˆa!" );
	dialogue( IMG_KING,     "”wµeÓ¡, ´a“¡»¡! ÓaÓaÓa!" );
	dialogue( IMG_HERO,     "¢¡! ‹aœñ ‘A‘±· ¸÷ÁAˆa ¢¡¡?" );
	dialogue( IMG_KING,     "µ×· ·BÁAŸi §©Ÿ¥  aµ×..." );
	dialogue( IMG_KING,     "‹a ·¡Ÿq a¸á•¡ ‰¡¬wÐe ¥w¬å aµ×·¡”a!" );
	dialogue( IMG_HERO,     "·¡œå, a«e ´b aˆ{·a“¡" );
	putImg( heroX, heroY, imgHero[RIGHT_SIDE]);
	gameSound( MOVE_SOUND );
	dialogue( IMG_HERO,     "´u! ‰·ºˆa!" );
	vgaLightOutPalette( effectPal, 0 );
	putImg( princessX, princessY, imgExit[1] );
	vgaLightInPalette( effectPal, 0 );
	dialogue( IMG_HERO,     "·¡œé®ˆa ‰·º“e ¬aœa»¡‰¡" );
	dialogue( IMG_HERO,     "¶Å ¢…·¡!");
	dialogue( IMG_HERO,     "µA·µ! ·¡Ìe¬aÌe·¡”a! ‰·º~~~" );

	putImg( heroX, heroY, imgFloor );
	gameSound( MOVE_SOUND );

	dialogue( IMG_KNIGHT,   "  ·¡œý‰A Ð¬á º…· ¡¡Ðñ·e ¯¡¸b–E”a!" );

	putImg( kingX, kingY, imgKing[(kingY+1)%2] );
	gameSound( CRASH_SOUND );
	delayMiliSec( 500 );
	putImg( kingX, kingY, imgKing[(kingY)%2] );
	gameSound( CRASH_SOUND );
	delayMiliSec( 500 );
	putImg( kingX, kingY, imgKing[(kingY+1)%2] );
	gameSound( CRASH_SOUND );
	delayMiliSec( 500 );

	fadeOutMusicAndPal( effectPal, 0 );
	vgaStdClearScr( BLACK );
	heroSide= DOWN_SIDE;
}

void storyEnding( void )
{
	int i, j, x, y;
	int heroX, heroY,
		princessX, princessY,
		kingX, kingY;

	heroX = 0;
	heroY = STAGE_SZY / 2;
	princessX = STAGE_SZX / 2;
	princessY = heroY;
	kingX = princessX - 1;
	kingY = princessY;

	for( y = 0 ; y < STAGE_SZY ; y++ )
		for( x = 0 ; x < STAGE_SZX ; x++ )
			stage[x][y] = IMG_FLOOR;

	vgaStdBoxFill( 0, 0, VGA256XSIZE-1, VGA256YSIZE-1, BLACK );

	vgaStdPutsxyC(  60, "‰iŠ‚ ‹¡¬a“e Ã·´é‰·ºŸi" );
	vgaStdPutsxyC(  80, "´ás‰A ´ás‰A Àx´a´ö”s“¡”a." );
	vgaStdPutsxyC( 100, "‹aœáa, ‹a‰µµA“e ´b aˆa" );
	vgaStdPutsxyC( 120, "‹¡¬a· ´|·i ˆa¡ b‰¡ ·¡´ö´á¶a." );
	vgaStdPutsxyC( 140, "‰·º“e ´Èa‰A ¸a¯¥·i ¤aœa¥¡‰¡ ·¶“e•A," );
	vgaStdPutsxyC( 180, "¸a...‹¡¬a“e ”iaˆv¯s“¡”a." );
	vgaFadeInPalette( effectPal, 0 );
	if ( waitForSec( 5 ) == EXIT_OPENING_KEY ) return;
	vgaFadeOutPalette( effectPal, 0 );

	/* ¯aÉ¡Ÿ¡ µE—· ÑÁ¡e  e—i‹¡ */
	vgaStdBoxFill( 0, 0, VGA256XSIZE-1, STAGE_START_PTY-1, WHITE );
	for( y = 0 ; y < STAGE_SZY ; y++ )
		for( x = 0 ; x < STAGE_SZX ; x++ )
			putImg( x, y, imgBlock );

	for( x = 0 ; x < princessX ; x++ )
		putImg( x, princessY, imgFloor );

	putImg( heroX, heroY, imgHero[RIGHT_SIDE] );
	putImg( princessX, princessY, imgPrincess );
	putImg( kingX, kingY, imgKing[kingY%2] );

	vgaStdPutImageInviCol( STAGE_START_PTX + princessX * IMG_SZX,
						   STAGE_START_PTY + princessY * IMG_SZY,
						   imgBlock );
	gameMusic( STORY_END_MUSIC );
	fadeInMusicAndPal( effectPal, 0 );

	/* ”¬a Â‰b */
	dialogue( IMG_HERO, "‰·º~~~~~~~ ˆaµÖ­¡!" );
	dialogue( IMG_KING, "ÓaÓaÓa...””eÐeˆé" );
	dialogue( IMG_KING, "¬é a µa‹¡Œa»¡ µ©º‰·¡´¡" );
	dialogue( IMG_HERO, "¨iŸ¡ ‰·ºŸi Î‰´áº¡!" );
	dialogue( IMG_KING, "ˆa £¡Áv¡! ‘A‘±•¡ ÐqA ´éaº a!");
	putImg( kingX, kingY, imgKing[(kingY+1)%2] );
	gameSound( CRASH_SOUND );
	dialogue( IMG_KING, "Èa¤i¶ œa‹©¤a Ò a¬a “¡¤a¬a µ±" );

	/* ÎŸe¬‚· ´é·q´á“e Òa‰Á */
	for( i = 0; i < 30; i++ )
	{
		incRGBPalette( B, 1 );
		delayMiliSec( 5 );
	}

	dialogue( IMG_HERO, "·¡œé®ˆa! ·¡œá”aˆe a a¸á..." );
	dialogue( IMG_HERO, "‹aœý”a! º¢…·i ˆáŽ¡ ¶A¶…”a¡e?!" );
	dialogue( IMG_HERO, "µ± ¬a¤a“¡ ¬a aÒ‘ ¤a‹©œa ¶¤iÈa" );
	dialogue( IMG_KING, "´a“¡! ·¡ ·¡œé®ˆa! ‹a º¢…·e!" );

	vgaSetAllPalette( effectPal );

	dialogue( IMG_HERO, "µ¡µµ! Òa‰Áˆa ·¶”a! Òa‰Áˆa!" );
	dialogue( IMG_HERO, "´á—¡ Ðe¤å”á!" );
	dialogue( IMG_HERO, "µ± ¬a¤a“¡ ¬a aÒ‘ ¤a‹©œa ¶¤iÈa" );

	for( i = 0; i < 63; i++ )
	{
		incRGBPalette( R, 1 );
		delayMiliSec( 5 );
	}

	for( i = 0 ; i < 30 ; i++ )
	{
		x = rand() % STAGE_SZX;
		y = rand() % STAGE_SZY;
		if ( y == princessY ) continue;
		putImg( x, y, imgFloor);
		gameSound( OPENEXIT_SOUND );
		delayMiliSec( 30 );
	}

	for( y = 0 ; y < STAGE_SZY / 2 ; y++ )
		for( x = 0 ; x < STAGE_SZX ; x++ )
			putImg( x, y, imgFloor );

	for( y = STAGE_SZY / 2 + 1 ; y < STAGE_SZY ; y++ )
		for( x = 0 ; x < STAGE_SZX ; x++ )
			putImg( x, y, imgFloor );

	for( x = princessX + 1 ; x < STAGE_SZX ; x++ )
		putImg( x, princessY, imgFloor );

	for( i = 0 ; i < 5 ; i++ )
		gameSound( OPENEXIT_SOUND );

	vgaSetAllPalette( effectPal );

	dialogue( IMG_KING, "·¡,·¡,·¡œé®ˆa! a·  a¤ó·¡!" );
	dialogue( IMG_HERO, "µA·µ!   a¤ó·¡  å¸á”a!  " );
	dialogue( IMG_HERO, "Èa¤i¶ œa‹©¤a Ò a¬a “¡¤a¬a µ±" );

	/* ÎŸe¬‚· ´é·q´á“e Òa‰Á */
	for( i = 0; i < 30; i++ )
	{
		incRGBPalette( B, 1 );
		delayMiliSec( 5 );
	}
	dialogue( IMG_KING, "a´a´a´a´a´a´b~~~~" );
	for( i = 0 ; i < 10 ; i++ )
	{
		putImg( kingX, kingY, imgKing[(kingY+i)%2] );
		gameSound( CRASH_SOUND );
		delayMiliSec( 100 );
	}
	/* ÎŸe¬‚· ´é·q´á“e Òa‰Á */
	for( i = 0; i < 33; i++ )
	{
		incRGBPalette( B, 1 );
		delayMiliSec( 5 );
	}
	vgaStdPutImageInviCol( STAGE_START_PTX + kingX * IMG_SZX,
						   STAGE_START_PTY + kingY* IMG_SZY,
						   imgBlock );
	vgaSetAllPalette( effectPal );
	gameSound( OPENEXIT_SOUND );
    dialogue( IMG_KNIGHT,   "Òa‰Á·q:ÅÁ»¡»¢~~~~~(´é·q´á“e ­¡Ÿ¡)" );

	dialogue( IMG_HERO, "·¡¤å·¡ ‘A‘±· ÂAÒ”a!" );
	dialogue( IMG_HERO, "¤h´aœu~~~~~~~~~~~" );
	gameSound( MOVE_SOUND );

	for( x = heroX+1 ; x < kingX ; x++ )
	{
		gameSound( CRASH_SOUND );
		putImg( x, kingY, imgSword );
		delayMiliSec( 50 );
	}

	for( x = heroX+1 ; x < kingX ; x++ )
	{
		putImg( x, kingY, imgFloor );
		delayMiliSec( 50 );
	}
	gameSound( MOVE_SOUND );

	for( i = 0 ; i < 5 ; i++ )
		gameSound( OPENEXIT_SOUND );

	putImg( kingX, kingY, imgFloor );

	vgaLightInPalette( effectPal, 0 );
	gameMusic( STORY_OPEN_MUSIC );
	dialogue( IMG_HERO, "´µÑ¡~ ´b aŸi º‚µv”a." );
	dialogue( IMG_HERO, "´aÀq! ‰·º! ‰·º!" );

	heroSide = RIGHT_SIDE;
	for( x = heroX + 1 ; x < princessX ; x++ )
		moveHero( x - 1, heroY, x, heroY );

	heroX = x - 1;

	dialogue( IMG_HERO, "‰·º! »¡‹q Œáº‰A" );
	dialogue( IMG_HERO, "µ± ¬a¤a“¡ ¬a aÒ‘ ¤a‹©œa ¶¤iÈa" );
	for( i = 0; i < 60; i++ )
	{
		incRGBPalette( R, 1 );
		delayMiliSec( 5 );
	}
	putImg( princessX, princessY, imgPrincess );
	vgaSetAllPalette( effectPal );

	dialogue( IMG_PRINCESS, "¸é ŠÐaœá µÁº­vŠ…¶a." );
	dialogue( IMG_HERO,     "”wµeÐa»¡..." );
	dialogue( IMG_PRINCESS, "´a´a~ ‹¡¨á¶a~" );
	dialogue( IMG_HERO,     "¶Ÿ¡, ´aŒa ¡µÐeˆá  a¸áÐa»¡.." );
	dialogue( IMG_PRINCESS, "´aŒa ¡µÐeˆá?! ‹a‰A ¢¡ºa?" );
	dialogue( IMG_HERO,     "·¡ˆá!" );

    putImg( heroX, heroY, imgFloor );
    vgaStdPutImageInviCol( STAGE_START_PTX + heroX * IMG_SZX + IMG_SZX/2,
                           STAGE_START_PTY + heroY * IMG_SZY,
                           imgHero[RIGHT_SIDE+1] );
    gameSound( CRASH_SOUND );

	dialogue( IMG_KNIGHT,   "Òa‰Á·q: ½¡~µ¢~(·³®‰  ea“e ­¡Ÿ¡)" );

    putImg( heroX, heroY, imgHero[RIGHT_SIDE] );
    putImg( princessX, princessY, imgPrincess );

    dialogue( IMG_PRINCESS, "´á á!" );
	dialogue( IMG_HERO,     "‰·º...¬aœwÐ!" );
	dialogue( IMG_PRINCESS, "¸á–¶a..." );
	dialogue( IMG_HERO,     "¶Ÿ¡...‰iÑ¥ÐiŒa?!" );
	dialogue( IMG_PRINCESS, "¹½´a¶a. ”¯¥ ¹¡ˆå·¡ ·¶´á¶a" );
	dialogue( IMG_HERO,     "¢¥•A?" );
	dialogue( IMG_PRINCESS, "Ðe¤å”á Ç¡¯aÐº¡¶a" );
	dialogue( IMG_HERO,     "´ás»¡ ´g»¡. ¸a~ ·¡Ÿ¡µÁ" );
	dialogue( IMG_PRINCESS, "´a·¡~~ ¸qŒe e¶a~~~" );
    dialogue( IMG_HERO,     "µá?" );
    dialogue( IMG_PRINCESS, "¦‰·i Œá´¡ºa! ¡¡“¡Èá¡ ¥¡‰¡ ·¶“e•A.." );
	dialogue( IMG_HERO,     "´aÐw! ¸qŒe e ‹¡”aa!!!!" );
    putImg( heroX, heroY, imgHero[DOWN_SIDE] );
	gameSound( MOVE_SOUND );
    dialogue( IMG_HERO,     "·¡¥Á¶a! ¦‰¹± s¯¡”a!" );
	fadeOutMusicAndPal( effectPal, 0 );
	vgaStdClearScr( BLACK );
	heroSide= DOWN_SIDE;
}

void mainEnding( void )
{
	gameMusic( NO_MUSIC );
    amdSetVolume( MAX_MUSIC_VOLUME );
	gameMusic( ENDING_MUSIC );

	vgaStdSetForeColor( YELLOW );
	vgaStdClearScr( BLACK );
	vgaStdPutsxyC( 100, "GAME OVER" );
	vgaStdPutsxyC( 170, "»iˆá¶… ¯¡ˆe·¡ –A´ö‹¡Ÿi..." );
	vgaFadeInPalette( effectPal, 0 );
    waitForSec( 10 );

	fadeOutMusicAndPal( effectPal, 0 );

	vgaStdCloseHan();
	vgaStdEndGraphMode();
	amdCloseMusic();
}

void setGamePalette( void )
{
	FILE *fpal;

	fpal = fopen( PALETTE_FNAME, "rb" );

	fread( effectPal, 768, 1, fpal );
	vgaSetAllPalette( effectPal );
}

void readStage( int stageNum )
{
	register i,j;
	FILE *stageFile;

	stageFile = fopen( STAGE_FNAME, "rb" );
	if( stageFile == NULL )
	errExit( "Can't find STAGE.DAT" );

	fseek( stageFile,
		   STAGE_SZX * STAGE_SZY * stageNum,
		   SEEK_SET );

	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ; j < STAGE_SZX ; j++ )
			stage[j][i] = getc( stageFile );

	fclose( stageFile );

    /* º·¥‰·· ¶áÃ¡Ÿi Àx“e”a */
	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ; j < STAGE_SZX ; j++ )
			if( stage[j][i] == IMG_HERO )
			{
				stage[j][i] = IMG_FLOOR;
				heroPosX = j;
				heroPosY = i;
				return;
			}
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

void mainLoop( void )
{
	int k;
	unsigned oldX, oldY;
	unsigned i, j;

	heroSide = DOWN_SIDE;

	oldX = heroPosX;
	oldY = heroPosY;
	putImg( heroPosX, heroPosY, imgHero[ heroSide ] );

	while( TRUE )
	{
		refreshMonsters();

		if( ( oldX != heroPosX ) || ( oldY != heroPosY ) )
		{
			moveHero( oldX, oldY, heroPosX, heroPosY );
			oldX = heroPosX;
			oldY = heroPosY;
		}

		if ( checkEvent( heroPosX, heroPosY ) == DONTMOVE )
		{
			heroPosX = oldX;
			heroPosY = oldY;
		}

		if( ( oldX != heroPosX ) || ( oldY != heroPosY ) )
		{
			moveHero( oldX, oldY, heroPosX, heroPosY );
			oldX = heroPosX;
			oldY = heroPosY;
		}

		k = keyGet();

		if ( k == KEY_Esc ) return;

        /* ÑÁ¬iÎa ¯aÏaœa·¡Ëa ÀáŸ¡ */
        if ( ( stage[heroPosX][heroPosY] == IMG_UP ) ||
			 ( stage[heroPosX][heroPosY] == IMG_DOWN ) ||
			 ( stage[heroPosX][heroPosY] == IMG_LEFT ) ||
			 ( stage[heroPosX][heroPosY] == IMG_RIGHT ) )
			 continue;

		switch( k )
		{
			case KEY_Up		:
				if( heroPosY - 1 <= -1 ) break;
				if( checkEvent( heroPosX, heroPosY - 1 ) == MOVE )
					--heroPosY;
				heroSide = UP_SIDE;
				break;

			case KEY_Down	:
				if( heroPosY + 1 >= STAGE_SZY ) break;
				if( checkEvent( heroPosX, heroPosY + 1 ) == MOVE )
					++heroPosY;
				heroSide = DOWN_SIDE;
				break;

			case KEY_Left	:
				if( heroPosX - 1 <= -1 ) break;
				if( checkEvent( heroPosX - 1, heroPosY ) == MOVE )
					--heroPosX;
				heroSide = LEFT_SIDE;
				break;

			case KEY_Right	:
				if( heroPosX + 1 >= STAGE_SZX ) break;
				if( checkEvent( heroPosX + 1, heroPosY ) == MOVE )
					++heroPosX;
				heroSide = RIGHT_SIDE;
				break;

			case AFTER_STAGE_KEY :
				stageCleared = TRUE;
				break;

			case BEFORE_STAGE_KEY :
				stageNo -= 2;
				if( stageNo < -1 ) stageNo = -1;
				stageCleared = TRUE;
				break;

            case CLEAR_GAME_KEY :
                stageNo = 29;
                stageCleared = TRUE;
                break;
		}

		/* ‹a Ìe·i ÇiŸ¡´á Ð–·i˜ */
		if( stageCleared == TRUE )
		{
			putImg( oldX, oldY, img[IMG_FLOOR] );
			return;
		}
	}
}

void refreshMonsters( void )
{
    /* REFRESH_RATE º‹¡µA ˜aœa ¡¥¯aÈá —i·i ”a¯¡‹aŸ¥”a(refresh) */

    static int oldTick = 0;
	static int refreshFlag = 1;

	int tick = 0;
	register i, j;

	tick = amdGetPos( currentMusic );
	if( tick == oldTick ) return;

	oldTick = tick;

	if( !( tick % REFRESH_RATE ) )
	{
		for( i = 0 ; i < STAGE_SZY ; i++ )
			for( j = 0 ; j < STAGE_SZX ; j++ )
			{
				if( stage[j][i] == IMG_SLIME )
					putImg( j, i, imgSlime[refreshFlag] );
				if( stage[j][i] == IMG_KNIGHT )
					putImg( j, i, imgKnight[refreshFlag] );
				if( stage[j][i] == IMG_GHOST )
					putImg( j, i, imgGhost[refreshFlag] );
			}
		refreshFlag = 1 - refreshFlag;
	}
}

void moveHero( int oldX, int oldY,
			   int heroPosX, int heroPosY )
{
	int x, y, xd, yd;

	if ( ( oldX == heroPosX ) && ( oldY == heroPosY ) ) return;

	if ( heroPosX >= STAGE_SZX ) heroPosX = STAGE_SZX - 1;
	if ( heroPosX <= -1 ) heroPosX = 0;
	if ( heroPosY >= STAGE_SZY ) heroPosY = STAGE_SZY - 1;
	if ( heroPosY <= -1 ) heroPosY = 0;

	x = STAGE_START_PTX + heroPosX * IMG_SZX;
	y = STAGE_START_PTY + heroPosY * IMG_SZY;

	putImg( oldX, oldY, img[stage[oldX][oldY]] );

	switch ( heroSide )
	{
		case LEFT_SIDE :
			xd = ( IMG_SZX / 2 );
			yd = 0;
			break;
		case RIGHT_SIDE :
			xd = -( IMG_SZX / 2 );
			yd = 0;
			break;
		case UP_SIDE :
			xd = 0;
			yd = ( IMG_SZY / 2 );
			break;
		case DOWN_SIDE :
			xd = 0;
			yd = -( IMG_SZY / 2 );
			break;
	}

    /* º·¥‰··¡ ¤eÄe ¶‘»¢·¥ ¡¡¯s·i ‹aŸ¥”a */
	vgaStdPutImageInviCol( x + xd, y + yd, imgHero[heroSide + 1] );

	gameSound( MOVE_SOUND );

	delayMiliSec( 100 );

	putImg( oldX, oldY, img[stage[oldX][oldY]] );

	putImg( heroPosX, heroPosY, img[stage[heroPosX][heroPosY]] );

	/* º·¥‰··¡ ”a¯¡ ¤eÄe ¶‘»¢·¥ ¡¡¯s·i ‹aa,
	   ‰i‰Á¸â·a¡ ÐeÄe·i ¶‘»¢·¡‰A Ðe”a */
	vgaStdPutImageInviCol( x, y, imgHero[heroSide] );

	gameSound( MOVE_SOUND );

	delayMiliSec( 100 );
}

int match( int item, int enemy )
{
	if( ( enemy == IMG_SLIME ) && ( item == IMG_STICK ) )
		return TRUE;

	if( ( enemy == IMG_KNIGHT ) && ( item == IMG_SWORD ) )
		return TRUE;

	if( ( enemy == IMG_GHOST ) && ( item == IMG_CROSS ) )
		return TRUE;

	return FALSE;
}

int checkEventMain( int x, int y )
{
	register i, j;

    /* ÑÁ¬iÎa ¯aÏaœa·¡Ëa ÀáŸ¡ */
    if( ( stage[x][y] == IMG_UP ) || ( stage[x][y] == IMG_DOWN ) ||
		( stage[x][y] == IMG_LEFT ) || ( stage[x][y] == IMG_RIGHT ) )
		return MOVE;

    /* Ã·´é‰·ºŸi Àx´v·i˜ ÀáŸ¡ */
    if ( stage[x][y] == IMG_PRINCESS )
	{
		dispScoreBoard( 9999 );
		stageCleared = TRUE;
		return MOVE;
	}

    /* µi®AŸi ˆx‰¡·¶‰¡, Â‰ŠµA •¡ÀbÐ–·i˜ ÀáŸ¡ */
    if ( ( heroKey == TRUE ) && ( stage[x][y] == IMG_EXIT ) )
	{
		dispScoreBoard( 100 );
		stageCleared = TRUE;
		return MOVE;
	}

    /* µi®AŸi Àx´v·i˜ ÀáŸ¡ */
    if( stage[x][y] == IMG_KEY )
	{
		dispScoreBoard( 5 );
		stage[x][y] = IMG_FLOOR;
		heroKey = TRUE;

		for( i = 0 ; i < STAGE_SZY ; i++ )
			for( j = 0 ; j < STAGE_SZX ; j++ )
				if( stage[j][i] == IMG_EXIT )
					putImg( j, i, imgExit[1] );

		gameSound( OPENEXIT_SOUND );
		return MOVE;
	}

    /* ´a·¡ÉQ·i ˆx‰¡·¶‰¡, ¸â·i  ev·i ˜ ÀáŸ¡ */
	if( ( heroItem != FALSE ) && ( stage[x][y] != IMG_FLOOR ) )
	{
		if ( match( heroItem, stage[x][y] ) )
		{
			dispScoreBoard( 10 );
			gameSound( CRASH_SOUND );
			heroItem = FALSE;
			stage[x][y] = IMG_FLOOR;
			dispItemBox();
			return MOVE;
		}
		return DONTMOVE;
	}

    /* ´a·¡ÉQ·i Àx´v·i˜ ÀáŸ¡ */
	if( ( heroItem == FALSE ) && ( stage[x][y] != IMG_FLOOR ) )
	{
		if( ( stage[x][y] == IMG_STICK ) ||
			( stage[x][y] == IMG_SWORD ) ||
			( stage[x][y] == IMG_CROSS ) )
		{
			dispScoreBoard( 10 );
			heroItem = stage[x][y];
			stage[x][y] = IMG_FLOOR;
			dispItemBox();
			return MOVE;
		}
		return DONTMOVE;
	}

	return MOVE;
}

int checkEvent( int x, int y )
{
	register i, j;
	int move;

	if ( ( heroPosX == x ) && ( heroPosY == y ) )
	{
        /* ÑÁ¬iÎa ¯aÏaœa·¡Ëa ÀáŸ¡ */
        switch ( stage[x][y] )
		{
			case IMG_UP :
				heroPosY--;
				heroSide = UP_SIDE;
				break;
			case IMG_DOWN :
				heroPosY++;
				heroSide = DOWN_SIDE;
				break;
			case IMG_LEFT :
				heroPosX--;
				heroSide = LEFT_SIDE;
				break;
			case IMG_RIGHT :
				heroPosX++;
				heroSide = RIGHT_SIDE;
				break;
		}

		return checkEventMain( heroPosX, heroPosY );

	} else
	{
        /* ÑÁ¬iÎa ¯aÏaœa·¡Ëa ÀáŸ¡ */
        if( ( stage[x][y] == IMG_UP ) || ( stage[x][y] == IMG_DOWN ) ||
			( stage[x][y] == IMG_LEFT ) || ( stage[x][y] == IMG_RIGHT ) )
			return MOVE;
	}

	if ( stage[x][y] == IMG_BLOCK ) return DONTMOVE;
	if ( stage[x][y] == IMG_FLOOR ) return MOVE;

	return checkEventMain( x, y );
}

void dispScoreBoard( int value )
{
	static score = 0;
	int color;

	color = vgaStdGetForeColor();

	score += value;

	vgaStdBoxFill( SCORE_NUM_PTX, SCORE_NUM_PTY,
				   VGA256XSIZE - 1, SCORE_NUM_PTY + 16,
				   WHITE );

	vgaStdSetForeColor( YELLOW );

	shadowPutsxy( SCORE_NUM_PTX, SCORE_NUM_PTY,
				  LIGHTCYAN, CYAN, BLACK,
				  "SCORE ");

	vgaStdSetForeColor( LIGHTGREEN );
	vgaStdPrintf( "%d", score );

	vgaStdSetForeColor( color );
}

void dispStageBoard( void )
{
	int color;

	color = vgaStdGetForeColor();

	vgaStdSetForeColor( YELLOW );

	shadowPutsxy( STAGE_NUM_PTX, STAGE_NUM_PTY,
				  YELLOW, BLUE, BLACK,
                  "STAGE " );

    vgaStdSetForeColor( LIGHTGREEN );
	vgaStdPrintf( "%d", stageNo + 1 );
	vgaStdSetForeColor( color );
}

void dispItemBox( void )
{
	int enemy;
	int x,y;

    /* ˆa»¡‰¡ ·¶“e ´a·¡ÉQµA ˜aœa Àå¸â‰Å‰· ¡¥¯aÈáŸi Â‰b */
    switch( heroItem )
	{
		case IMG_STICK :
			enemy = IMG_SLIME;
			break;
		case IMG_SWORD :
			enemy = IMG_KNIGHT;
			break;
		case IMG_CROSS :
			enemy = IMG_GHOST;
			break;
		default :
			enemy = IMG_FLOOR;
			break;
	}

	x = ITEM_BOX_PTX;
	y = ITEM_BOX_PTY;

	vgaStdBox( x - 1, y - 1,
			   x + IMG_SZX, y + IMG_SZY,
			   YELLOW );

	vgaStdBox( x + IMG_SZX, y - 1,
			   x + IMG_SZX * 2 + 1, y + IMG_SZY,
			   YELLOW);

	if( heroItem != NULL )
	{
		vgaStdPutImageInviCol( x, y, img[heroItem] );
		vgaStdPutImageInviCol( x + IMG_SZX + 1, y, img[enemy]);
	}
	else
	{
		vgaStdBoxFill( x, y,
					   x + IMG_SZX - 1, y + IMG_SZY - 1,
					   BLACK);

		vgaStdBoxFill( x + IMG_SZX + 1, y,
					   x + IMG_SZX * 2, y + IMG_SZY - 1,
					   BLACK );
	}
}

void drawStage( void )
{
	register i, j;

	vgaStdClearScr( BLUE );

	vgaStdBoxFill( 0, 0, 319, STAGE_START_PTY-1, WHITE );
	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ; j < STAGE_SZX ; j++ )
            putImg( j, i, img[stage[j][i]] );
}

void putImg( int x, int y, byte *id )
{
	vgaStdPutImageInviCol( STAGE_START_PTX + x * IMG_SZX,
						   STAGE_START_PTY + y * IMG_SZY,
						   imgFloor );
	vgaStdPutImageInviCol( STAGE_START_PTX + x * IMG_SZX,
						   STAGE_START_PTY + y * IMG_SZY,
						   id );
}

void gameSound( int choice )
{
	int i;

	switch( choice )
	{
		case MOVE_SOUND :
			sound( 1000 );
			delayMiliSec( 30 );
			nosound();
			break;
		case CRASH_SOUND :
			sound( 2000 );
			delayMiliSec( 60 );
			nosound();
			break;
		case OPENEXIT_SOUND :
			for( i = 0 ; i < 3 ; i++ )
			{
                sound( 200*i );
                delayMiliSec( 10 );
				nosound();
				delayMiliSec( 10 );
			}
			break;
	}
}

void gameMusic( int choice )
{
	if ( choice == NO_MUSIC )
		amdStop();
	else
	{
		amdRewind( music[choice] );
		amdPlay( music[choice] );
		currentMusic = music[choice];
	}
}

int replayStage( void )
{
	int k;
	int color;
	byte *str = "·¡ Ìe·i ”a¯¡Ða‰V¯s“¡Œa? (Y/N)";

	vgaGetAllPalette( effectPal );
	vgaDecPalette( effectPal, 16, 30 );
	vgaSetAllPalette( effectPal );

	color = vgaStdGetForeColor();
	vgaStdSetForeColor( LIGHTBLUE );
	shadowPutsxy( vgaStdGetCenterOfStr( str ), 100,
				  YELLOW, LIGHTRED, RED, str );
	vgaStdSetForeColor( color );

	while ( TRUE )
	{
		k = getch();
		if( k == 'Y' || k == 'y' ) return TRUE;
		if( k == 'N' || k == 'n' ) return FALSE;
	}
}

/*********************** ETC FUNCTIONS **************************/

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

void putSpeaker( byte *img )
{
	vgaStdBoxFill( 1, 3, 1 + IMG_SZX, 3 + IMG_SZY, WHITE );
	vgaStdPutImageInviCol( 1, 3, img);
}

void dialogue( int who, char *msg )
{
	int x, y;

	vgaStdBoxFill( DIALOGUE_SX, DIALOGUE_SY,
				   DIALOGUE_EX, DIALOGUE_EY,
				   BLUE );

	vgaStdHLine( DIALOGUE_SX, DIALOGUE_SY, DIALOGUE_EX, LIGHTBLUE );
	vgaStdVLine( DIALOGUE_SX, DIALOGUE_SY, DIALOGUE_EY, LIGHTBLUE );
	vgaStdHLine( DIALOGUE_SX, DIALOGUE_EY, DIALOGUE_EX, BLACK );
	vgaStdVLine( DIALOGUE_EX, DIALOGUE_SY, DIALOGUE_EY, BLACK );

	x = DIALOGUE_SX + 3;
	y = DIALOGUE_SY + 3;

	switch ( who )
	{
		case IMG_HERO :
			putSpeaker( imgHero[DOWN_SIDE] );
			shadowPutsxy( x, y, LIGHTCYAN, CYAN, BLACK, msg );
			break;

		case IMG_PRINCESS :
			putSpeaker( imgPrincess );
			shadowPutsxy( x, y, LIGHTMAGENTA, MAGENTA, BLACK, msg );
			break;

		case IMG_KING :
			putSpeaker( imgKing[0] );
			shadowPutsxy( x, y, LIGHTRED, RED, BLACK, msg );
			break;
		default :
			putSpeaker( imgKnight[0] );
			shadowPutsxy( x, y, LIGHTGREEN, GREEN, BLACK, msg );
			break;
	}

	waitForSec( 3 );
}

int  waitForSec( long sec )
{
	long start, end;
	int  key;

	start = getClock();
	end   = getClock();

	while ( ( end - start ) < sec * BASIC_CLOCK_HZ )
	{
        if ( (key = keyGet()) != FALSE ) return key;
		end = getClock();
	}

    return SUCCESS;
}

void delayMiliSec( long miliSec )
{
	long start, end;

	start = getClock();
	end   = getClock();

	while ( ( end - start ) < miliSec * BASIC_CLOCK_HZ / 1000 )
		end = getClock();

}

int  fadeInMusicAndPal( byte *pal, int start )
{
    register i;
	byte *temp;

	if( !pal ) return FAIL;

	temp = ( byte * )malloc( 768 );
	memcpy( temp, pal, 768 );
	vgaCheckVSync();

	for( i = 63 ; i >= 0 ; i-- )
	{
        amdSetVolume( 63 - i );
		delayMiliSec( 1 );
        vgaDecPalette( temp, start, i );
		vgaSetAllPalette( temp );
		memcpy( temp, pal, 768 );
	}

    amdSetVolume( MAX_MUSIC_VOLUME );
	free( temp );

	return SUCCESS;
}

int  fadeOutMusicAndPal( byte *pal, int start )
{
	register i;
	byte *temp;

	if( !pal ) return FAIL;

	temp = ( byte * )malloc( 768 );
	memcpy( temp, pal, 768 );
	vgaCheckVSync();

	for( i = 0 ; i <= 63 ; i++ )
	{
        amdSetVolume( MAX_MUSIC_VOLUME - i );
		delayMiliSec( 1 );
		vgaDecPalette( temp, start, 1 );
		vgaSetAllPalette( temp );
	}

	amdSetVolume( 0 );
	free( temp );

	return SUCCESS;
}

void incRGBPalette( int rgb, int value )
{
	int i;
	byte *temp;

	temp = ( byte * ) malloc( 768 );
    if( !temp ) errExit( "no memory in incRGBPalette()" );

	vgaCheckVSync();

	vgaGetAllPalette( temp );
	delayMiliSec( 1 );

	for( i = rgb ; i < 768 ; i += 3 )
	{
		if ( *( temp + i ) + value <= 63 )
			 *( temp + i ) += value;
		else
			 *( temp + i ) = 63;
	}

	vgaSetAllPalette( temp );
	free( temp );
}

void decRGBPalette( int rgb, int value )
{
	int i;
	byte *temp;

	temp = ( byte * ) malloc( 768 );
	if( !temp ) errExit( "no memory in decRGBPalette()" );

	vgaCheckVSync();

	vgaGetAllPalette( temp );
	delayMiliSec( 1 );

	for( i = rgb ; i < 768 ; i += 3 )
	{
		if ( *( temp + i ) >= value )
			 *( temp + i ) -= value;
		else
			 *( temp + i ) = 0;
	}

	vgaSetAllPalette( temp );
	free( temp );
}

int  keyGet( void )
{
	if ( keyPressed() ) return keyRead();
	return FALSE;
}
