#define SERIAL_BUF_LEN 256
// 距離
extern double serial_distance;
// ボタン
extern int serial_button;

int init_serial(); // 初期化 エラー=0 ok=1
int read_serial(); // 読む
int serial_button_trigger();
int serial_distance_trigger(double target);
