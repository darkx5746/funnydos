#define STAGE_TIME 95

extern int stageMap[4096];
extern int bossPtn[1024];
extern int *enemyPtn['Z' - 'A' + 1];

extern AMD_Music *stageMusic[2];
extern AMD_Music *instrument;

typedef enum
{
	SKIP = 0,
	ENEMY = 3,
    OBJECT = 4,
    ITEM = 5,
    END = 9999
} MAP_EVENT;

typedef enum
{
    //SKIP = 0,
    MOVE = 1,
    SHOOT = 2,
    BOSSSHOOT = 4,
    JUMP = 9,
    INVINCIBLE = 777
    // END = 9999
} ENEMY_EVENT;

void loadMusic(int st);
void loadSprite(int st);
int advanceStage(int command);
void shipBoom(void);
void readyStage(int);
void playGame(void);
void stageEnding(int);