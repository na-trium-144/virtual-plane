extern double y;
extern int score, hiscore;
void init_game();
int game_update();
#define Y_RANGE 3
#define X_RANGE 10

#define READY_T 1.5
enum GameState { g_ready, g_main, g_over, };
extern enum GameState game_state;
extern double game_main_t;

#define GAME_OBJ_NUM 50
enum ObjKind { g_none = 0, g_block, g_coin, };
extern struct GameObj{
  enum ObjKind kind;
  double x, y, vx, vy;
  double t; // アニメーション用
  int hit_me; // 自機にあたった→動かなくなる
  int score; // このオブジェクトに表示するスコア
  double score_t; // スコアの残り表示時間
} game_obj[];
extern int game_obj_current;

void obj_check(double sec_diff);
void obj_clear();
