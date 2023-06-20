#include "game.h"
#include "serial.h"

#define D_MIN 5
#define D_MAX 15
#define Y_RANGE 3

double y;
int score;
int game_update(){
  if(read_serial()){
    double yp = (serial_distance - (D_MAX + D_MIN) / 2) / ((D_MAX - D_MIN) / 2);
    if(yp < -1){
      yp = -1;
    }
    if(yp > 1){
      yp = 1;
    }
    y = yp * Y_RANGE;

    if(serial_distance_trigger(10) == -1){
      score++;
    }
    if(serial_button_trigger() == -1){
      score += 3;
    }
    return 1;
  }
  return 0;
}

