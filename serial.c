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
  return 1;
}

int read_serial(){
  // シリアルを\r\nの改行で区切って1回実行ごとに1行ずつ読む
  // serial_bufに文字列を入れ、1行の文字数を返す
  static int current_pos = 0;
  static char buf[SERIAL_BUF_LEN] = {};
  static int tmp_pos = 0, tmp_len = 0;
  while(1){
    for(; tmp_pos < tmp_len; tmp_pos++){
      if(buf[tmp_pos] == '\n'){
	serial_buf[current_pos] = 0;
	int current_len = current_pos;
	current_pos = 0;
	tmp_pos += 1;
	if(current_len > 0){
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
    //printf("current %d: '%s'\n", current_pos, serial_buf);
    //printf("read");
    tmp_pos = 0;
    tmp_len = read(serial_port, buf, sizeof(buf));
    //printf("%d: '%s'\n", tmp_len, buf);
  }
}

// serial_bufをdoubleに変換
double get_distance(){
  return atof(serial_buf);
}

#ifdef IS_MAIN
int main() {
  if(!init_serial()){
    return 1;
  }
  while(1){
    if ( read_serial() > 0 ) {
      //printf(">> %s\n", serial_buf);
      printf(">> %f\n", get_distance());
    }
  }
}
#endif
