/*
   This code is for calibration. Please do step by step:
   - Upload the code to Arduino or ESP32

   - Without pressing any key/button, write down the value on Serial Monitor
   - Update this value on your code at setNoPressValue(value) function

   - Press each key one by one and write down the value each time press
   - Update these values on your code at registerKey(key, value)

   NOTE: when no press or press one key/button, the values on Serial Monitor may be NOT consistent, they may be slighly different => use a central value
*/

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
}

void loop() {
  Serial.println("A0: ");
  Serial.println(analogRead(A0));
  Serial.println("A1: ");
  Serial.println(analogRead(A1));
  delay(100);
}