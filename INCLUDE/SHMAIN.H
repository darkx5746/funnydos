#define STAGE_NUM 1
#define SHIP_NUM  9

typedef enum
{
	PLAY,
	BOSS,
    STOP,
    STAGECLEAR,
    BOOM,
    GAMECLEAR,
    GAMEOVER
}   GAME_STATE;

extern gameState;
extern score;
extern shipNum;
extern _super;