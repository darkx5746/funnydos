#include "common.h"
#include "amdmusic.h"

#include "shmain.h"

/* default sprite definitions */

typedef enum
{
    CENTER, UP, DOWN, LEFT, RIGHT
}   Direction;

#define BACK    LEFT
#define FORWARD RIGHT

#define SHIELD_SPR      0

#define OPTION_SPR      (SHIELD_SPR+1)
#define OPTION_SPR_MAX  3
#define OPTION_SPR_END  (OPTION_SPR + OPTION_SPR_MAX)

#define SHIP_SPR        OPTION_SPR_END
#define SHIP_SPR_END    (SHIP_SPR+1)

#define ITEM_SPR        SHIP_SPR_END
#define ITEM_SPR_MAX    8
#define ITEM_SPR_END    (ITEM_SPR + ITEM_SPR_MAX)

#define OBJECT_SPR      ITEM_SPR_END
#define OBJECT_SPR_MAX  64
#define OBJECT_SPR_END  (OBJECT_SPR + OBJECT_SPR_MAX)

#define ENEMY_SPR       OBJECT_SPR_END
#define ENEMY_SPR_MAX   64
#define ENEMY_SPR_END   (ENEMY_SPR + ENEMY_SPR_MAX)

#define BOSS_NORMAL     (ENEMY_SPR_END)
#define BOSS_ATTACK     (BOSS_NORMAL+1)
#define BOSS_SPR_END    (BOSS_ATTACK+1)

#define SHOOT_SPR       (BOSS_SPR_END)
#define SHOOT_SPR_MAX   16
#define SHOOT_SPR_END   (SHOOT_SPR + SHOOT_SPR_MAX)

#define ESHOOT_SPR      SHOOT_SPR_END
#define ESHOOT_SPR_MAX  64
#define ESHOOT_SPR_END  (ESHOOT_SPR + ESHOOT_SPR_MAX)

typedef enum
{
    INIT, RUN, SET, UNSET = 32, OTHER = 256
} ObjCommand;

/* palette scroll color definitions */

#define STAR_COLOR_START 129
#define STAR_COLOR_END   199
#define STAR_DISP_MAX    100
#define STAR_COLOR_NUM   (STAR_COLOR_END-STAR_COLOR_START+1)

/* main game definitions */

/* type define for shoot */

typedef enum
{
    NORMAL,
    LASER,
    RIPPLE,
    shootTypeMax
}   shootType;

/* sub module include file */

#include "shkey.h"
#include "shstage.h"
#include "shsubf.h"

/* image buffer define */

extern byte *shipImg[9];
extern byte *miniNum[11];
extern byte *enemyImg[255];
extern byte *optionImg;
extern byte *shieldImg;
extern byte *shootImg[5];
extern byte *itemImg[3];
extern byte *boomImg[3];
extern byte *panelImg[12];
extern byte *eShootImg[3];

extern AMD_Music *stageMusic[2];
extern AMD_Music *instrument;
extern soundTable[10];

void scroll(void);
int ship(int);
int shoot(int);
int option(int);
int shield(int);
int powerUp(int);
void enemy(int, int *);
void object(int, int *);
void eShoot(int, int *);
void item(int, int *);
void boss(int);
