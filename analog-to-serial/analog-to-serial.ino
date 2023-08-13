// const int MOISTURE_SENSOR_PIN1 = 12;
// const int MOISTURE_SENSOR_PIN2 = 13;
// const int MOISTURE_SENSOR_PIN3 = 14;
// const int MOISTURE_SENSOR_PIN4 = 27;
// const int MOISTURE_SENSOR_PIN5 = 32;

// void setup() {
//   Serial.begin(9600);
// }

// void loop() {
//   int sensorValue1 = analogRead(MOISTURE_SENSOR_PIN1);
//   int sensorValue2 = analogRead(MOISTURE_SENSOR_PIN2);
//   int sensorValue3 = analogRead(MOISTURE_SENSOR_PIN3);
//   int sensorValue4 = analogRead(MOISTURE_SENSOR_PIN4);
//   int sensorValue5 = analogRead(MOISTURE_SENSOR_PIN5);

//   Serial.print("Analog values: ");
//   Serial.print("Pin1: ");
//   Serial.print(sensorValue1);
//   Serial.print(", Pin2: ");
//   Serial.print(sensorValue2);
//   Serial.print(", Pin3: ");
//   Serial.print(sensorValue3);
//   Serial.print(", Pin4: ");
//   Serial.print(sensorValue4);
//   Serial.print(", Pin5: ");
//   Serial.println(sensorValue5);

//   delay(500);  // Adjust the delay according to your needs
// }
const int RED_PIN = 26;
const int GREEN_PIN = 25;
const int BLUE_PIN = 33;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void loop() {
  // Turn on red color
  analogWrite(RED_PIN, 255);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  delay(1000);
  
  // Turn on green color
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 255);
  analogWrite(BLUE_PIN, 0);
  delay(1000);
  
  // Turn on blue color
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 255);
  delay(1000);
  
  // Turn off the LED
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  delay(1000);
}
