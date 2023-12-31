#include "game.h"
#include "serial.h"
#include "audio.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// 超音波センサの使用する距離の範囲
#define D_MIN 5
#define D_MAX 15

double y = 0, vy = 0;
double score = 0;
int hiscore = 0;
double my_angle(){
  return atan(vy / Y_RANGE / 4); // いい感じに見えるようパラメータを調整
}

double mouse_x_rat, mouse_y_rat;
int use_mouse = 0;
int mouse_clicked = 0, mouse_clicked_prev = 0;

void init_game(){
  srand((unsigned int)time(NULL));

  // セーブデータの読み込み
  int f = open("hiscore.txt", O_RDONLY);
  if(f >= 0){
    char buf[20];
    read(f, buf, sizeof(buf));
    hiscore = atoi(buf);
  }
  close(f);

  bgm_change(g_title);
}
void save_score(){
  // スコアが最高記録ならセーブする
  score = (double)(int)(score);
  if(score > hiscore){
    hiscore = score;
    int f = open("hiscore.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(f >= 0){
      char buf[20];
      int n = sprintf(buf, "%d", hiscore);
      write(f, buf, n);
    }
    close(f);
  }
}

struct GameObj game_obj[GAME_OBJ_NUM];
int game_obj_current = 0;
enum GameState game_state = g_title;
double game_main_t = 0;
double last_obj_appear = 0; // 最後にオブジェクト出現した時刻
double obj_mass[] = {0, 5, 2, 15, 1}; // 各オブジェクトの質量

void state_change(enum GameState g){
  // 状態切替時の処理
  game_main_t = 0;
  game_state = g;
  bgm_change(g);
  if(g == g_over){
    save_score();
  }
  if(g == g_title){
    score = 0;
    obj_clear();
  }
  if(g == g_main){
    last_obj_appear = -1000; // てきとうに小さい値
  }
}

double rand1(){
  return rand() * 1.0 / RAND_MAX;
}

void move_myship(double sec_diff){
  double yp; // y座標を-1〜1にする
  if(use_mouse){
    yp = (0.5 - mouse_y_rat) * 3;
  }else{
    yp = (serial_distance - (D_MAX + D_MIN) / 2) / ((D_MAX - D_MIN) / 2);
  }
  double margin = 1 + Y_MARGIN / Y_RANGE;
  // ±1ちょっとの範囲におさめる
  if(yp < -1 * margin){
    yp = -1 * margin;
  }
  if(yp > 1 * margin){
    yp = 1 * margin;
  }
  // biquadフィルタをかける
  // https://www.utsbox.com/?page_id=523
  static double yp_1 = 0, yp_2 = 0;
  static double ypf_1 = 0, ypf_2 = 0;
  double omega = 2 * 3.14 * 4 / 30;
  double alpha = sin(omega) / (2.0 * 0.7);
  double a0 = 1 + alpha, a1 = -2 * cos(omega), a2 = 1 - alpha,
    b0 = (1 - cos(omega)) / 2, b1 = 1 - cos(omega), b2 = b0;
  double ypf = b0 / a0 * yp + b1 / a0 * yp_1 + b2 / a0 * yp_2
    - a1 / a0 * ypf_1 - a2 / a0 * ypf_2;
  yp_2 = yp_1;
  yp_1 = yp;
  ypf_2 = ypf_1;
  ypf_1 = ypf;

  // ±Y_RANGEの範囲に変換
  double new_y = ypf * Y_RANGE;
  vy = (new_y - y) / sec_diff;
  y = new_y;
}

void obj_collision(struct GameObj* b, struct GameObj* g){
  // なんとなく運動量保存をする
  double ca = atan2(g->y - b->y, g->x - b->x); // 衝突角度
  double b_vv = b->vx * cos(ca) + b->vy * sin(ca); // ca方向の成分(右上正)
  double b_vh = - b->vx * sin(ca) + b->vy * cos(ca); // 衝突面に平行の成分(左上正)
  double g_vv = g->vx * cos(ca) + g->vy * sin(ca);
  double g_vh = - g->vx * sin(ca) + g->vy * cos(ca);
  printf("b x=%.2f y=%.2f g x=%.2f y=%.2f\n", b->vx, b->vy, g->vx, g->vy);
  printf("b v=%.2f h=%.2f g v=%.2f h=%.2f\n", b_vv, b_vh, g_vv, g_vh);
  // -v1 + v2 = v1' - v2' ...1
  // v1 + mv2 = v1' + mv2' ...2
  double m = obj_mass[g->kind] / obj_mass[b->kind];
  double b_vv_new = ((1 - m) * b_vv + 2 * m * g_vv) / (1 + m); // 1 * m + 2
  double g_vv_new = (2 * b_vv + (m - 1) * g_vv) / (m + 1); // 2 - 1
  // 衝突後の速度をセット
  b->vx = b_vv_new * cos(ca) - b_vh * sin(ca);
  b->vy = b_vv_new * sin(ca) + b_vh * cos(ca);
  g->vx = g_vv_new * cos(ca) - g_vh * sin(ca);
  g->vy = g_vv_new * sin(ca) + g_vh * cos(ca);
}
void obj_check(double sec_diff){
  for(struct GameObj *g = game_obj; g < game_obj + GAME_OBJ_NUM; g++){
    g->score_t -= sec_diff;
    if(g->kind == g_none){
      continue;
    }
    if(game_state != g_main){
      continue;
    }
    // x,yを動かす
    double gx_prev = g->x;
    g->x += g->vx * sec_diff;
    g->y += g->vy * sec_diff;
    g->t += sec_diff;
    for(struct GameObj *b = g + 1; b < game_obj + GAME_OBJ_NUM; b++){
      if(b->kind == g_none){
        continue;
      }
      // 他のオブジェクトと衝突チェック
      if(fabs(b->x - g->x) <= 0.5 && fabs(b->y - g->y) <= 0.5){
        struct GameObj *c1 = b, *c2 = g; // c1->kind >= c2->kind に注意
        if(b->kind < g->kind){
          c2 = b;
          c1 = g;
        }
        printf("collision %d, %d\n", c1->kind, c2->kind);
        switch(c1->kind){
        case g_bullet:{
          // 弾に衝突
          switch(c2->kind){
          case g_block:
            c2->score = (int)(80 * c2->x / X_RANGE);
            score += c2->score;
            c2->kind = g_none;
            c2->score_x = c2->x;
            c2->score_y = c2->y;
            c2->score_t = 0.5;
            se_play(se_hit);
            c1->kind = g_none;
            break;
          case g_coin:
            break;
          case g_yakan:
            obj_collision(c1, c2);
            se_play(se_yakan);
            break;
          default:
            obj_collision(c1, c2);
            break;
          }
          break;
        }
        default:{
          obj_collision(c1, c2);
          break;
        }
        }
      }
    }
    if(fabs(g->x) <= 0.4 && fabs(g->y - y) <= 0.4){
      // 自機に衝突
      g->hit_me = 1;
      switch(g->kind){
      case g_block:
      case g_yakan:
      case g_bullet:
        state_change(g_over);
        save_score();
        break;
      case g_coin:
        g->score = 100;
        score += g->score;
        g->score_t = 0.5;
        g->kind = g_none;
        g->score_x = 0;
        g->score_y = g->y;
        se_play(se_coin);
        break;
      case g_none:
        break;
      }
    }
    if(g->x < -0.5 && gx_prev >= -0.5){
      // 自機通過
      // if(fabs(y) < Y_RANGE){
      // }
    }
    if(fabs(g->y) > Y_RANGE * 2 || g->x > X_RANGE || g->x < -3){
      // 画面外
      g->kind = g_none;
    }
  }
}
void obj_clear(){
  for(struct GameObj *g = game_obj; g < game_obj + GAME_OBJ_NUM; g++){
    g->score_t = 0;
    g->kind = g_none;
  }
}
void bullet_appear(){
  struct GameObj *b = &game_obj[game_obj_current];
  game_obj_current = (game_obj_current + 1) % GAME_OBJ_NUM;
  b->kind = g_bullet;
  b->x = 0.6;
  b->vx = 10;
  b->y = y + 0.2;
  b->vy = b->vx * tan(my_angle());
  printf("angle=%f\n", my_angle());
  b->t = 0;
  se_play(se_shoot);
  score -= 10;
}
void obj_appear(){
  struct GameObj *g = &game_obj[game_obj_current];
  game_obj_current = (game_obj_current + 1) % GAME_OBJ_NUM;
  // オブジェクトの種類をランダムに設定
  double a = rand1();
  if(a < 0.1){
    g->kind = g_yakan;
  }else if(a < 0.4){
    g->kind = g_coin;
  }else{
    g->kind = g_block;
  }
  g->x = X_RANGE; // 右端
  // 速度をランダムにする
  // 最初は2〜5、時間が経つごとに速くなる
  g->vx = -(2.0 + 3.0 * rand1()) * (1 + game_main_t / BGM_1LOOP);
  // y速度を決めた後、±Y_RANGEに到達するようy初期位置を逆算
  g->vy = 1.0 * rand1() - 0.5; // -0.5〜0.5
  g->y = (2 * rand1() - 1) * Y_RANGE - g->vy * fabs(g->x / g->vx);
  g->t = 0;
  g->score = 0;
  g->score_t = 0;
  g->hit_me = 0;
  printf("v = (%.3f, %.3f)\n", g->vx, g->vy);
}

// 繰り返し実行する処理
// 30fpsで処理する。1/30秒経っていなければ何もせずreturn 0
// 処理をしたらreturn 1
int game_update(){
  static double sec_prev = 0;
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  double sec = spec.tv_nsec / 1.0e9;
  double sec_diff = sec - sec_prev;
  if(sec_diff < 0){
    sec_diff += 1;
  }
  if(sec_diff < 1.0 / 30){
    return 0;
  }
  sec_prev = sec;

  // シリアルとマウスを読む
  read_serial();
  // mouse_click_triggerはクリックした瞬間だけ1となる
  int mouse_click_trigger = mouse_clicked && !mouse_clicked_prev;
  mouse_clicked_prev = mouse_clicked;
  mouse_clicked = 0; // 0に戻す

  // 現在の状態に応じて処理を記述
  switch(game_state){
  case g_title:
    move_myship(sec_diff);
    if(serial_button_trigger() == 1 || mouse_click_trigger == 1){
      state_change(g_ready);
    }
    break;
  case g_ready:
    move_myship(sec_diff);
    if(fabs(y) < Y_RANGE){
      if(game_main_t == 0){
        bgm_change(g_ready);
      }
      game_main_t += sec_diff;
      if(game_main_t >= READY_T){ // 1.5秒間画面内にすればスタート
        state_change(g_main);
      }
    }else{
      game_main_t = 0;
      bgm_stop();
    }
    if(serial_button_trigger() == 1 || mouse_click_trigger == 1){
      state_change(g_title);
    }
    break;
  case g_over:
    if(serial_button_trigger() == 1 || mouse_click_trigger == 1){
      state_change(g_title);
    }
    break;
  case g_main:
    move_myship(sec_diff);
    if(serial_button_trigger() == 1 || mouse_click_trigger == 1){
      bullet_appear();
    }
    game_main_t += sec_diff;
    if(fabs(y) < Y_RANGE){
      // 自機が範囲内なら時間で点数増える
      score += sec_diff * TIME_SCORE_RATE;
    }
    obj_check(sec_diff);
    double obj_interval = GAME_OBJ_INTERVAL_INIT / (1 + game_main_t / BGM_1LOOP);
    // BGM1周ごとに 1 → 1/2 → 1/3 ...となる
    if(game_main_t - last_obj_appear > obj_interval){
      obj_appear();
      last_obj_appear = game_main_t;
    }
    break;
  }
  return 1;
}

