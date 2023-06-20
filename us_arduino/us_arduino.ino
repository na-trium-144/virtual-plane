#define TRIG_PIN 2 // set digital pin ID
#define ECHO_PIN 3
#define BUTTON_PIN 4
const float time_gain = 1.0/58; // gain for duration [us] to distance [cm]
float read_sensor(){
// trigger
digitalWrite(TRIG_PIN, HIGH);
delayMicroseconds(10); // 10 usec
digitalWrite(TRIG_PIN, LOW);
// measure the duration when echo is high
return time_gain*pulseIn(ECHO_PIN,HIGH);
}
void setup() {
// ultrasonic sensor
pinMode(TRIG_PIN, OUTPUT);
pinMode(ECHO_PIN, INPUT);
pinMode(BUTTON_PIN, INPUT);
Serial.begin(9600);
}
float distance;
void loop() {
distance = read_sensor();
Serial.print("d");
Serial.println(distance);
Serial.print("b");
Serial.println(digitalRead(BUTTON_PIN));
delay(10);
}
