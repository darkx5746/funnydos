#include <stdio.h>
#include <stdlib.h>
#include "chingari.h"

byte stage[STAGE_SZX][STAGE_SZY];

void loadStage( char *loadFile, int stageNum )
{
	register i, j;
	FILE *stageFile;

    --stageNum;

    stageFile = fopen( loadFile, "rb" );
	if ( stageFile == NULL )
	{
		puts( "Can't load stage data file" );
		exit( 0 );
	}

	fseek( stageFile, STAGE_SZX*STAGE_SZY*stageNum, SEEK_SET );

	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ; j < STAGE_SZX ; j++ )
			stage[j][i] = getc( stageFile );

	fclose( stageFile );
}

void saveStage( char *saveFile, int stageNum )
{
	register i, j;
	FILE *stageFile;

    --stageNum;

    stageFile = fopen( saveFile, "r+b" );

	fseek( stageFile, STAGE_SZX*STAGE_SZY*stageNum, SEEK_SET );

	for( i = 0 ; i < STAGE_SZY ; i++ )
		for( j = 0 ;j < STAGE_SZX ; j++ )
			putc( stage[j][i], stageFile );

	fclose( stageFile );
}

void main( int argc, char *argv[] )
{
	if ( argc != 5 )
	{
		puts("Usage:>STAGECP source-file dest-file source-stage dest-stage" );
		return;
	}

	loadStage( argv[1], atoi( argv[3] ) );
	saveStage( argv[2], atoi( argv[4] ) );
	puts( "Successfully copied." );
}
