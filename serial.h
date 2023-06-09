#define SERIAL_BUF_LEN 256
extern int serial_port;
extern char serial_buf[SERIAL_BUF_LEN];
int init_serial();
int read_serial();
double get_distance();
