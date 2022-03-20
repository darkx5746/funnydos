#include <stdio.h>
#include <conio.h>

#include "rol2amd.h"

void loadInstrTableData( int speedSearchMode )
{
	int i;

	for ( i = 0; i < instrNum; i++ )
	{
		if ( i%5 == 0 ) printf( "\n" );
		printf( " %2d - %-8s  ", i, instrTable[i].name );
		if ( !getBankInstr( instrTable[i].name,
			&instrTable[i].instr, speedSearchMode ) )
		{
            printf( "\n %2d - not found in BANK. Press any key...\n", i );
			getch();
        }
	}
    printf( "\n" );
}
