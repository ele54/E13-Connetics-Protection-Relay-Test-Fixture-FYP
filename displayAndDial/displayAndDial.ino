/*
User interface for the analogue current function:
Reads a dial and outputs the value to a 3 digit seven segment display

*/
#include <ShiftDisplay.h>

#define POT_PIN A2
#define SEG_LATCH_PIN 7
#define SEG_CLOCK_PIN 8
#define SEG_DATA_PIN 9
int pot_value;
const int DISPLAY_TYPE = COMMON_CATHODE;
#define DISPLAY_SIZE 3

ShiftDisplay display(SEG_LATCH_PIN, SEG_CLOCK_PIN, SEG_DATA_PIN, DISPLAY_TYPE, DISPLAY_SIZE);

void setup() {
    pinMode(POT_PIN, INPUT);
}

void loop() {
    pot_value = analogRead(POT_PIN);
    pot_value = map(value, 0, 1023, 0, 200);//map value
    display.set(pot_value);
    display.setDot(1, true);
    display.show();
}