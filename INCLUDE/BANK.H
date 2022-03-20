#ifndef __BANK__
#define __BANK__

#include "common.h"


#define BANK_MAJ_VERSION 1
#define BANK_MIN_VERSION 0

#define BANK_SIG            "ADLIB-"
#define BANK_SIG_LEN        6


/* Instrument bank file header */
typedef struct {
    byte majorVersion;
    byte minorVersion;
    char sig[BANK_SIG_LEN]; /* signature: "ADLIB-" */
    word nrDefined;         /* number of list entries used */
    word nrEntry;           /* number of total entries in list */
    long offsetIndex;       /* offset in file of start of list of names */
    long offsetData;        /* offset in file of start of data */
    char filler[8];         /* must be zero */
} Bank_Header;

/* Name of instrument with a pointer to its position in the file */
typedef struct {
    word nrReference;
    char usedFlag;          /* 0 if not used */
    char instrName[9];      /* max 8 char's + NULL */
} Bank_Entry;

/* Operator structure */
typedef struct {
	char ksl;
	char freqMult;
    char feedBack;          /* used by operator 0 only */
	char attack;
	char sustLevel;
	char sustain;
	char decay;
	char release;
	char output;
	char am;
	char vib;
	char ksr;
    char fm;                /* used by operator 0 only */
} Operator;

typedef struct {
    char mode;
	char percVoice;
    Operator op0;
    Operator op1;
	char wave0;
	char wave1;
} Bank_Instr;

#endif

