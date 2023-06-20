#define SERIAL_BUF_LEN 256
extern double serial_distance;
extern int serial_button;
int init_serial();
int read_serial();
int serial_button_trigger();
int serial_distance_trigger(double target);
