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

struct GameObj game_obj[GAME_OBJ_NUM], game_bullet[GAME_BULLET_NUM];
int game_obj_current = 0, game_bullet_current = 0;
int game_over = 0;
enum GameState game_state = g_title;
double game_main_t = 0;
double last_obj_appear = 0;
void state_change(enum GameState g){
  game_main_t = 0;
  game_state = g;
  bgm_change(g);
  if(g == g_over){
    save_score();
  }
  if(g == g_title){
    score = 0;
    obj_clear();
    bullet_clear();
  }
  if(g == g_main){
    last_obj_appear = -1000; // てきとうに小さい値
  }
}

double rand1(){
  return rand() * 1.0 / RAND_MAX;
}

void move_myship(double sec_diff){
  double yp;
  if(use_mouse){
    yp = (0.5 - mouse_y_rat) * 3;
  }else{
    yp = (serial_distance - (D_MAX + D_MIN) / 2) / ((D_MAX - D_MIN) / 2);
  }
  if(yp < -1){
    yp = -1;
  }
  if(yp > 1){
    yp = 1;
  }
  double new_y = yp * Y_RANGE;
  vy = (new_y - y) / sec_diff;
  y = new_y;
}

void move_bullet(double sec_diff){
  for(struct GameObj *b = game_bullet; b < game_bullet + GAME_BULLET_NUM; b++){
    if(b->kind == g_none){
      continue;
    }
    b->x += b->vx * sec_diff;
    b->y += b->vy * sec_diff;
    b->t += sec_diff;
    if(fabs(b->y) > Y_RANGE * 2 || b->x > X_RANGE || b->x < -3){
      b->kind = g_none;
    }
  }
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
    double gx_prev = g->x;
    if(!g->hit_me){
      g->x += g->vx * sec_diff;
      g->y += g->vy * sec_diff;
      g->t += sec_diff;
      for(struct GameObj *b = game_bullet; b < game_bullet + GAME_BULLET_NUM; b++){
        if(b->kind == g_bullet && fabs(b->x - g->x) <= 0.5 && fabs(b->y - g->y) <= 0.5){
          // 弾に衝突
          switch(g->kind){
          case g_block:
            g->score = (int)(80 * g->x / X_RANGE);
            score += g->score;
            g->kind = g_none;
            g->score_x = g->x;
            g->score_y = g->y;
            g->score_t = 0.5;
            se_play(se_hit);
            b->kind = g_none;
            break;
          case g_yakan:
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
            double m = 10;
            double b_vv_new = ((1 - m) * b_vv + 2 * m * g_vv) / (1 + m); // 1 * m + 2
            double g_vv_new = (2 * b_vv + (m - 1) * g_vv) / (m + 1); // 2 - 1
            b->vx = b_vv_new * cos(ca) - b_vh * sin(ca);
            b->vy = b_vv_new * sin(ca) + b_vh * cos(ca);
            g->vx = g_vv_new * cos(ca) - g_vh * sin(ca);
            g->vy = g_vv_new * sin(ca) + g_vh * cos(ca);
            se_play(se_yakan);
          case g_coin:
            break;
          }
        }
      }
      if(fabs(g->x) <= 0.5 && fabs(g->y - y) <= 0.5){
        // 自機に衝突
        g->hit_me = 1;
        switch(g->kind){
        case g_block:
        case g_yakan:
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
}
void obj_clear(){
  for(struct GameObj *g = game_obj; g < game_obj + GAME_OBJ_NUM; g++){
    g->score_t = 0;
    g->kind = g_none;
  }
}
void bullet_clear(){
  for(struct GameObj *b = game_bullet; b < game_bullet + GAME_BULLET_NUM; b++){
    b->kind = g_none;
  }
}
void bullet_appear(){
  struct GameObj *b = &game_bullet[game_bullet_current];
  game_bullet_current = (game_bullet_current + 1) % GAME_BULLET_NUM;
  b->kind = g_bullet;
  b->x = 0;
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
  double a = rand1();
  if(a < 0.1){
    g->kind = g_yakan;
  }else if(a < 0.4){
    g->kind = g_coin;
  }else{
    g->kind = g_block;
  }
  g->x = X_RANGE;
  g->vx = -(2.0 + 2.0 * rand1()) * (1 + game_main_t / BGM_1LOOP);
  g->y = (2 * rand1() - 1) * Y_RANGE;
  g->vy = 1.0 * rand1() - 0.5; // -0.5〜0.5
  g->t = 0;
  g->score = 0;
  g->score_t = 0;
  g->hit_me = 0;
  printf("v = (%.3f, %.3f)\n", g->vx, g->vy);
}

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

  int mouse_click_trigger = mouse_clicked && !mouse_clicked_prev;
  mouse_clicked_prev = mouse_clicked;
  mouse_clicked = 0;

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
    move_bullet(sec_diff);
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

