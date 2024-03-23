#define BAUD_RATE     115200
#define LED_PIN       PIN_PA3

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD_RATE);
  while (!Serial) {;} // wait for serial port to connect

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  

  Serial.println("Serial init done.");
}

void loop() {
  // put your main code here, to run repeatedly:
}
