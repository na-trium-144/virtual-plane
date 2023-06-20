#include "game.h"
#include "serial.h"
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

double y = 0;
int score = 0;
int hiscore = 0;
void init_game(){
  srand((unsigned int)time(NULL));

  int f = open("hiscore.txt", O_RDONLY);
  if(f >= 0){
    char buf[20];
    read(f, buf, sizeof(buf));
    hiscore = atoi(buf);
  }
  close(f);
}
void save_score(){
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
int game_obj_current;
int game_over = 0;
enum GameState game_state = g_ready;
double game_main_t = 0;

double rand1(){
  return rand() * 1.0 / RAND_MAX;
}

void move_myship(){
  double yp = (serial_distance - (D_MAX + D_MIN) / 2) / ((D_MAX - D_MIN) / 2);
  if(yp < -1){
    yp = -1;
  }
  if(yp > 1){
    yp = 1;
  }
  y = yp * Y_RANGE;
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
      g->x -= g->vx * sec_diff;
      g->y -= g->vy * sec_diff;
      g->t += sec_diff;
      if(fabs(g->x) <= 0.5 && fabs(g->y - y) <= 0.5){
	// 自機に衝突
	g->hit_me = 1;
	switch(g->kind){
	case g_block:
	  game_state = g_over;
	  game_main_t = 0;
	  save_score();
	  break;
	case g_coin:
	  g->score = 100;
	  score += g->score;
	  g->score_t = 0.5;
	  g->kind = g_none;
	  break;
	}
      }
      if(g->x < -0.5 && gx_prev >= -0.5){
	// 自機通過
	if(fabs(y) < Y_RANGE){
	  switch(g->kind){
	  case g_block:
	    g->score = (int)((1 - fabs(g->y - y) / (Y_RANGE * 2)) * 50);
	    score += g->score;
	    g->score_t = 0.5;
	    break;
	  case g_coin:
	    break;
	  }
	}
      }
      if(g->x < -3){
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

  read_serial();

  switch(game_state){
  case g_ready:
    move_myship();
    if(fabs(y) < Y_RANGE){
      game_main_t += sec_diff;
      if(game_main_t >= READY_T){ // 1.5秒間画面内にすればスタート
	game_state = g_main;
	game_main_t = 0;
      }
    }else{
      game_main_t = 0;
    }
    if(serial_button_trigger() == 1){
      printf("exit\n");
      exit(0);
    }
    break;
  case g_over:
    if(serial_button_trigger() == 1){
      game_state = g_ready;
      game_main_t = 0;
      score = 0;
      obj_clear();
    }
    break;
  case g_main:
    move_myship();
    game_main_t += sec_diff;
    obj_check(sec_diff);
    if(rand1() < 1.0 / 60){
      struct GameObj *g = &game_obj[game_obj_current];
      game_obj_current = (game_obj_current + 1) % GAME_OBJ_NUM;
      if(rand1() < 0.7){
	g->kind = g_block;
      }else{
	g->kind = g_coin;
      }
      g->x = X_RANGE;
      g->vx = 1.0 + 5.0 * rand1(); // 1〜6 
      g->y = (2 * rand1() - 1) * Y_RANGE;
      g->vy = 1.0 * rand1() - 0.5; // -0.5〜0.5
      g->t = 0;
      g->score = 0;
      g->score_t = 0;
    }
    break;
  }
  return 1;
}

