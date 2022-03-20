#ifndef __MOUSE__
#define __MOUSE__

int initMouse( void );  /* return 0 if not initialized */
void mouseOn( void );
void mouseOff( void );
void getMousePos( int *, int * );
void setMousePos( int, int );
int getMouseLeftButton( void );
int getMouseRightButton( void );

#endif
