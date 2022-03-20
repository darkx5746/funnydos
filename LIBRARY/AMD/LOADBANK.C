#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rol2amd.h"


static Bank_Header bankHd;
static Bank_Entry *bankEntry;

static FILE *bankFp;


int loadBank( char *fname )
{
	if ( bankFp ) closeBank();

	if ( !(bankFp = fopen( fname, "rb" )) )
        errExit( "BANK file open error!" );

    /* read BANK header */
	fread( &bankHd, sizeof(Bank_Header), 1, bankFp );
    if ( (bankHd.majorVersion != BANK_MAJ_VERSION)
        || (bankHd.minorVersion != BANK_MIN_VERSION)
        || memcmp( bankHd.sig, BANK_SIG, BANK_SIG_LEN ) )
             errExit( "Invalid BANK file." );

    if ( !(bankEntry = malloc( sizeof(Bank_Entry) * bankHd.nrEntry )) )
        errExit( ERR_MEMORY );

	fseek( bankFp, bankHd.offsetIndex, SEEK_SET );
	fread( bankEntry, sizeof(Bank_Entry), bankHd.nrEntry, bankFp );

	return SUCCESS;
}

int closeBank( void )
{
    if ( !bankFp ) return FAIL;

    free( bankEntry );
    fclose( bankFp );

    return SUCCESS;
}

int getBankInstr( char *instrName, Bank_Instr *instr, int speedSearchMode )
{
    int i, tmpReference;

	if ( !bankFp )
        errExit( "Cannot read BANK file. No BANK files loaded" );

    /* from entry, search 'instrName' */
    if ( speedSearchMode )
    {
		int low, mid, hi, cmpResult, foundEntry = FALSE;

        low = 0;
        hi = bankHd.nrEntry - 1;
        while ( (low <= hi) && !foundEntry )
        {
            mid = (low + hi)/2;
            cmpResult = stricmp( instrName, bankEntry[mid].instrName );
            if ( cmpResult > 0 )
                low = mid + 1;
            else if ( cmpResult < 0 )
				hi = mid - 1;
            else
            {
                tmpReference = bankEntry[mid].nrReference;
                foundEntry = TRUE;
            }
        }
        if ( !foundEntry ) return FAIL;
    }
    else
    {
        for ( i = 0; i < bankHd.nrEntry; i++ )
            if ( !(stricmp( instrName, bankEntry[i].instrName )) )
            {
                tmpReference = bankEntry[i].nrReference;
                break;
            }
        if ( i == bankHd.nrEntry ) return FAIL;
    }

    /* read 'instr' */
    fseek( bankFp, (long) bankHd.offsetData + (long) sizeof(Bank_Instr) * tmpReference, SEEK_SET );
	fread( instr, sizeof(Bank_Instr), 1, bankFp );

	return SUCCESS;
}

