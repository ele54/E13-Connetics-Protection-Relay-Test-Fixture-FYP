/*
   Created by ArduinoGetStarted.com

   This example code is in the public domain

   Tutorial page: https://arduinogetstarted.com/library/arduino-analog-button-array-example

   This example reads the pressed button from an array of buttons connected to single analog pin and prints it to Serial Monitor.
*/

#include <ezAnalogKeypad.h>

ezAnalogKeypad buttonArray(A0);  // create ezAnalogKeypad object that attach to pin A0

void setup() {
  Serial.begin(9600);

  // MUST READ: You MUST run the calibration example, press button one-by-one to get the analog values
  // The below values is just an example, your button's value may be different
  buttonArray.setNoPressValue(1023);  // analog value when no button is pressed
  buttonArray.registerKey(1, 0); // analog value when the button 1 is pressed
  buttonArray.registerKey(2, 288); // analog value when the button 1 is pressed
  buttonArray.registerKey(3, 563); // analog value when the button 2 is pressed
  buttonArray.registerKey(4, 688); // analog value when the button 3 is pressed
  buttonArray.registerKey(5, 760); // analog value when the button 4 is pressed
  buttonArray.registerKey(6, 807); // analog value when the button 5 is pressed
  buttonArray.registerKey(7, 845); // analog value when the button 5 is pressed
  buttonArray.registerKey(8, 882); // analog value when the button 5 is pressed
  buttonArray.registerKey(9, 910); // analog value when the button 5 is pressed
  buttonArray.registerKey(10, 944); // analog value when the button 5 is pressed

  // ADD MORE IF HAS MORE
}

void loop() {
  unsigned char button = buttonArray.getKey();
  if (button) {
    Serial.print("The button ");
    Serial.print((int)button);
    Serial.println(" is pressed");
  }
  
  switch (button) {
    case 1:
      Serial.print(1);
      break;
    case 2:
      Serial.print(2);
      break;
  }
}
