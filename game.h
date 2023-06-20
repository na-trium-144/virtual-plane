extern double y;
extern int score;
int game_update();
#define Y_RANGE 3
#define X_RANGE 10

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
extern int game_over;
