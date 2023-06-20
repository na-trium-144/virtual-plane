#include "game.h"
#include "serial.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define D_MIN 5
#define D_MAX 15

double y;
int score;

struct GameObj game_obj[GAME_OBJ_NUM];
int game_obj_current;
int game_over = 0;
  
double rand1(){
  return rand() * 1.0 / RAND_MAX;
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

  if(game_over){
    return 0;
  }
  
  if(!game_over && read_serial()){
    double yp = (serial_distance - (D_MAX + D_MIN) / 2) / ((D_MAX - D_MIN) / 2);
    if(yp < -1){
      yp = -1;
    }
    if(yp > 1){
      yp = 1;
    }
    y = yp * Y_RANGE;
  }

  for(struct GameObj *g = game_obj; g < game_obj + GAME_OBJ_NUM; g++){
    if(g->kind == g_none){
      continue;
    }
    double gx_prev = g->x;
    if(!g->hit_me){
      g->x -= g->vx * sec_diff;
      g->y -= g->vy * sec_diff;
      g->t += sec_diff;
      g->score_t -= sec_diff;
      if(fabs(g->x) <= 0.5 && fabs(g->y - y) <= 0.5){
	// 自機に衝突
	g->hit_me = 1;
	game_over = 1;
      }
      if(g->x < -0.5 && gx_prev >= -0.5){
	// 自機通過
	if(!game_over){
	  g->score = (int)((1 - fabs(g->y - y) / (Y_RANGE * 2)) * 10);
	  score += g->score;
	  g->score_t = 0.5;
	}
      }
      if(g->x < -3){
	// 画面外
	g->kind = g_none;
      }
    }
  }
  if(rand1() < 1.0 / 60){
    struct GameObj *g = &game_obj[game_obj_current];
    game_obj_current = (game_obj_current + 1) % GAME_OBJ_NUM;
    g->kind = g_block;
    g->x = X_RANGE;
    g->vx = 3.0; 
    g->y = (2 * rand1() - 1) * Y_RANGE;
    g->vy = 0;
    g->t = 0;
    g->score = 0;
    g->score_t = 0;
  }
  
  return 1;
}

