#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "serial.h"

// main() をコンパイルするかどうか
//#define IS_MAIN

int serial_port;
char serial_buf[SERIAL_BUF_LEN];

double serial_distance, serial_distance_prev;
int serial_button, serial_button_prev;

// serial_portの初期化
int init_serial(){
  serial_port = open("/dev/ttyUSB0", O_RDWR);
  if ( serial_port < 0 ) {
    printf("Error %i from open: %s\n", errno, strerror(errno));
    return 0;
  }
  struct termios tty;
  // Save tty settings, also checking for error
  if (tcgetattr(serial_port, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return 0;
  }
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);
  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return 0;
  }

  // serial_portをnonblockingにする
  int flags = fcntl(serial_port, F_GETFL, 0);
  fcntl(serial_port, F_SETFL, flags | O_NONBLOCK);
  
  return 1;
}

int current_pos = 0;
char buf[SERIAL_BUF_LEN] = {};
int tmp_pos = 0, tmp_len = 0;

int read_serial_next(){
  int new_len = read(serial_port, buf, sizeof(buf));
  if(new_len > 0){
    tmp_len = new_len;
    tmp_pos = 0;
    return 1;
  }
  return 0;
}
  
int read_serial_line(){
  // シリアルを\r\nの改行で区切って1回実行ごとに1行ずつ読む
  // serial_bufに文字列を入れ、1行の文字数を返す
  while(1){
    for(; tmp_pos < tmp_len; tmp_pos++){
      if(buf[tmp_pos] == '\n'){
	serial_buf[current_pos] = 0;
	int current_len = current_pos;
	current_pos = 0;
	tmp_pos += 1;
	if(current_len > 0){
	  //	  printf("%s\n", serial_buf);
	  return current_len;
	}
      }else if(buf[tmp_pos] == '\r'){
	// skip
      }else{
	serial_buf[current_pos++] = buf[tmp_pos];
      }
      if(current_pos >= SERIAL_BUF_LEN){
	current_pos = 0;
      }
    }
    // 続きがあるか確認する
    if(read_serial_next()){
      continue;
    }else{
      return 0;
    }
  }
}

int read_serial(){
  serial_button_prev = serial_button;
  serial_distance_prev = serial_distance;

  int ret = 0;
  while(read_serial_line() > 0){
    if(serial_buf[0] == 'd'){
      serial_distance = atof(serial_buf + 1);
      ret = 1;
    }
    if(serial_buf[0] == 'b'){
      serial_button = atoi(serial_buf + 1);
      ret = 1;
    }
  }
  return ret;
}

// 押した瞬間だけ1,離した瞬間だけ-1になる
int serial_button_trigger(){
  return serial_button - serial_button_prev;
}
// dを横切った瞬間だけ1,-1になる
int serial_distance_trigger(double target){
  if(serial_distance < target && serial_distance_prev >= target){
    return -1;
  }else if(serial_distance >= target && serial_distance_prev < target){
    return 1;
  }else{
    return 0;
  }
}

#ifdef IS_MAIN
int main() {
  if(!init_serial()){
    return 1;
  }
  while(1){
    if ( read_serial() ) {
      printf(">> distance = %f (%d), button = %d (%d)\n",
	     serial_distance,
	     serial_distance_trigger(10),
	     serial_button,
	     serial_button_trigger());
    }
  }
}
#endif
