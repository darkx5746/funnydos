#include "rol.h"
#include "bank.h"
#include "amd.h"


#define ROL_MAX_INSTR_NUM   0xff

typedef struct {
    Bank_Instr instr;
    char name[10];
} Rol_InstrData;

extern Rol_Header rolHd;
extern Rol_InstrData instrTable[ROL_MAX_INSTR_NUM];
extern word instrNum;
extern word voiceNum;
extern word tempoChanges;
extern Tempo_Event *tempo;
extern Voice_Event *voice;

extern int loadRol( char *rol_fname );
extern int closeRol( void );

extern void loadInstrTableData( int speedSearchMode );

extern int loadBank( char *bank_fname );
extern int getBankInstr( char *instr_name, Bank_Instr *instr, int speedSearchMode );
extern int closeBank( void );

extern int convertRol2Amd( char *amd_fname );

