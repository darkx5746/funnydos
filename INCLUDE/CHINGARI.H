/*[]===============================================

	Ã·´é‰·ºŸi Àx´a¬á v1.0
	¹·    ŸA : ÑA”áÑÁ·© [ CHINGARI.H ]

	¬a¶w LIB : VGASTD.LIB, AMD.LIB, BASIC.LIB
	‰A·±¹A¸b : NEWTYPE
	¹A¸b·©¯¡ : 1994.2.x
	¤á ¸å ´ó : 1994.7.x

===============================================[]*/

#include "vgastd.h"
#include "palette.h"
#include "stdhan.h"
#include "stdpcx.h"
#include "color.h"
#include "amdmusic.h"
#include "key.h"
#include "timer.h"

#define MAX_STAGE           	30
#define MAX_MUSIC				10

#define IMG_FLOOR				0
#define IMG_BLOCK              	1
#define IMG_HEROUP             	16
#define IMG_HERODOWN           	17
#define IMG_HEROLEFT           	18
#define IMG_HERORIGHT          	19
#define IMG_HERO 				2
#define IMG_SLIME				3
#define IMG_SLIME1             	20
#define IMG_SLIME2             	21
#define IMG_KNIGHT				4
#define IMG_KNIGHT1            	22
#define IMG_KNIGHT2            	23
#define IMG_GHOST				5
#define IMG_GHOST1             	24
#define IMG_GHOST2             	25
#define IMG_STICK				6
#define IMG_SWORD				7
#define IMG_CROSS				8
#define IMG_PRINCESS			9
#define IMG_KEY					10
#define IMG_EXIT				11
#define IMG_CLOSEEXIT          	26
#define IMG_OPENEXIT           	27
#define IMG_UP                  12
#define IMG_DOWN                13
#define IMG_LEFT                14
#define IMG_RIGHT               15
#define IMG_KING				28
#define IMG_LOGO				29

#define IMG_SZX					20
#define IMG_SZY					16
#define IMG_SIZE				(IMG_SZX * IMG_SZY + 4)
#define STAGE_SZX				16
#define STAGE_SZY				11
#define STAGE_START_PTX			0
#define STAGE_START_PTY			24

#define STAGE_FNAME				"STAGE.DAT"
#define PALETTE_FNAME			"CHINGARI.PAL"
#define IMG_FNAME				"CHINGARI.IMG"

#define LOGO_IMG_SIZE			(16 * 16 + 4)