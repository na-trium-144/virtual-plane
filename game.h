// 自機の位置
extern double y, vy;
double my_angle();
// スコア
extern double score;
extern int hiscore;
void init_game(); // 初期化
double rand1(); // 0〜1の乱数を返す
int game_update(); // メインループ
#define Y_RANGE 3 // y範囲
#define Y_MARGIN 0.1 // 追加で自機が移動可能な範囲
#define X_RANGE 15 // x範囲(0〜)
#define BPM 148 // BGMのBPM
// bgmのbpmにあわせて変える
#define BGM_1LOOP (60.0 / BPM * 4 * 48)
#define TIME_SCORE_RATE (1 / (60.0 / BPM)) // この時間ごとにスコア1増える
#define READY_T (60.0 / BPM * 4) // Ready? の時間

// ゲームの状態
enum GameState { g_title, g_ready, g_main, g_over, };
extern enum GameState game_state;
extern double game_main_t; // その状態での経過時間
void state_change(enum GameState g); // 状態を変更する

#define GAME_OBJ_NUM 50 // オブジェクト数
#define GAME_OBJ_INTERVAL_INIT 1 // 秒に1個出現する
// オブジェクトの種類
enum ObjKind { g_none = 0, g_block, g_coin, g_yakan, g_bullet, };
extern double obj_mass[5]; // 各オブジェクトの質量
extern struct GameObj{
  enum ObjKind kind;
  double x, y, vx, vy; // 位置
  double t; // アニメーション用
  int hit_me; // 自機にあたった→動かなくなる
  int score; // このオブジェクトに表示するスコア
  double score_x, score_y; // スコア表示時のy座標(固定するため)
  double score_t; // スコアの残り表示時間
} game_obj[];
extern int game_obj_current; // 次に出現するオブジェクトのインデックス

// マウス
extern double mouse_x_rat, mouse_y_rat; // マウス座標の画面幅高さに対する割合(0〜1)
extern int use_mouse; // マウス使用=1
extern int mouse_clicked; // クリックされている=1 処理後0に戻す

// 衝突したとき跳ね返る処理
void obj_collision(struct GameObj* b, struct GameObj* g);
// 自機を動かす sec_diff=1フレームの秒数
void move_myship(double sec_diff);
// 各オブジェクトを動かす
void obj_check(double sec_diff);
// オブジェクト消去
void obj_clear();
// オブジェクトがランダムに出現
void obj_appear();
// 弾が出現
void bullet_appear();
