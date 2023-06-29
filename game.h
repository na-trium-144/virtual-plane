extern double y, vy;
double my_angle();
extern double score;
extern int hiscore;
void init_game();
double rand1();
int game_update();
#define Y_RANGE 3
#define X_RANGE 15
#define BPM 148
#define BGM_1LOOP (60.0 / BPM * 4 * 48)
// bgmのbpmにあわせて変える
#define TIME_SCORE_RATE (1 / (60.0 / BPM))

#define READY_T (60.0 / BPM * 4)
enum GameState { g_title, g_ready, g_main, g_over, };
extern enum GameState game_state;
extern double game_main_t;
void state_change(enum GameState g);

#define GAME_OBJ_NUM 50
#define GAME_OBJ_INTERVAL_INIT 1 // 秒に1個出現する
enum ObjKind { g_none = 0, g_block, g_coin, g_yakan, g_bullet, };
extern double obj_mass[5]; // 各オブジェクトの質量
extern struct GameObj{
  enum ObjKind kind;
  double x, y, vx, vy;
  double t; // アニメーション用
  int hit_me; // 自機にあたった→動かなくなる
  int score; // このオブジェクトに表示するスコア
  double score_x, score_y; // スコア表示時のy座標(固定するため)
  double score_t; // スコアの残り表示時間
} game_obj[];
extern int game_obj_current;

extern double mouse_x_rat, mouse_y_rat;
extern int use_mouse;
extern int mouse_clicked;

void obj_collision(struct GameObj* b, struct GameObj* g);
void move_myship(double sec_diff);
void obj_check(double sec_diff);
void obj_clear();
void obj_appear();
void bullet_appear();
void star_check();
