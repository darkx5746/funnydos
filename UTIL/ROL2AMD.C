#include <stdio.h>
#include <string.h>

#include "rol2amd.h"

void main( int argc, char * argv[] )
{
    int speedSearchMode = OFF;

    printf( "\nROL2AMD Ver 1.0 (Converts ROL to AMD v%d.%02d) by Gapacci 1994/03/21\n\n", AMD_MAJ_VER, AMD_MIN_VER );

	if ( !strcmp( argv[4], "/s" ) && (argc == 5) )
		speedSearchMode = ON;
	else if ( argc != 4 )
        errExit(
			"Usage:  ROL2AMD <rol_fname> <bank_fname> <amd_fname> <option>\n"
			"Option: /s - speed search for sorted BANK file. default = off" );

	printf( "Loading ROL...\n" );
	if ( !loadRol( argv[1] ) )
        errExit( "Rol file reading error!" );

	printf( "Loading BANK...\n" );
	if ( !loadBank( argv[2] ) )
        errExit( "Bank file reading error!" );

	printf( "Loading instrument data...\n" );
	loadInstrTableData( speedSearchMode );

	printf( "\nConverting...\n\n" );
	convertRol2Amd( argv[3] );
	printf( "\nConverting success !\n" );
}
