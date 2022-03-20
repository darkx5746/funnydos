#ifndef __COMMON__
#define __COMMON__

#define TRUE	1
#define FALSE	0

#define SUCCESS 1
#define FAIL	0

#define ON      1
#define OFF     0

typedef unsigned char byte;
typedef unsigned int word;
typedef enum { false = 0, true } boolean;

extern char *ERR_FATAL;
extern char *ERR_FILE;
extern char *ERR_MEMORY;

extern void setError( char *str );
extern void errExit( char *str );

#endif

