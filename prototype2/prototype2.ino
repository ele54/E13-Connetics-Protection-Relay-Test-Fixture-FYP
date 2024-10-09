/*
Final Prototype:

  CB status opens and closes based on relay trip/close and manual button inputs.
  Out shift register code modified from https://www.instructables.com/3-Arduino-pins-to-24-output-pins/
  Reads a dial and outputs the value to a 3 digit seven segment display.

*/
#include <ezAnalogKeypad.h>
#include <ShiftDisplay.h>

float pot_value;
const int DISPLAY_TYPE = COMMON_CATHODE;

bool spring_charge_timer_running = 0; // Timer for spring charged status re-charging
unsigned long spring_charge_start_time;
#define SPRING_CHARGE_DURATION 4000 // Delay duration to emulate spring charging time in milliseconds

#define POT_PIN A2
#define SEG_DATA_PIN 6
#define SEG_LATCH_PIN 7
#define SEG_CLOCK_PIN 8
#define DISPLAY_SIZE 3

ezAnalogKeypad buttonSet1(A1);   
ezAnalogKeypad buttonSet2(A2);  
ezAnalogKeypad buttonSet3(A5);  
ShiftDisplay display(SEG_LATCH_PIN, SEG_CLOCK_PIN, SEG_DATA_PIN, DISPLAY_TYPE, DISPLAY_SIZE);

#define TRIP_INPUT_PIN 9  // Digital pin used for CB trip
#define CLOSE_INPUT_PIN 10

// 74HC595 shift register pins and variables for LEDs
#define STATUS_CLOCK_PIN 2  //pin 11 SH_CP
#define STATUS_LATCH_PIN 3 //pin 12 ST_CP
#define STATUS_DATA_PIN 4  //pin 14 DS

#define num_status_registers 3
#define num_status_register_pins num_status_registers * 8
boolean LEDregisters[num_status_register_pins];

// array index of each status 
// CHANGE THIS TO MOVE THINGS AROUND ON THE USER INTERFACE (0 being the top left status on the user interface)
#define CB_status0 0  // CB position 
#define CB_status1 1  // spring charged status
#define CB_status2 2  // blank status
#define CB_status3 3  // blank status
#define CB_status4 4  // blank status
#define CB_status5 5  // blank status
#define CB_status6 6  // blank status
#define CB_status7 7  // blank status
#define CB_status8 8  // blank status
#define CB_status9 9  // blank status
#define CB_status10 10  // blank status
#define NUM_STATUSES 11

struct Status {
  int green_LED;      // shift register pin that the LED are connected to  
  int red_LED;        // shift register pin that the LED are connected to  
  int green_button;   // position of button (as mapped in setup() function)
  int red_button;     // position of button (as mapped in setup() function)
  boolean state;      // output logic state
};

Status statuses_array[NUM_STATUSES] = {
  // pin mapping for each status' associated group of LEDs and buttons 
  {7, 6, 1, 10, LOW},     // CB_status0 (CB position)
  {5, 4, 2, 9, HIGH},     // CB_status1 (spring charged)
  {3, 2, 3, 8, HIGH},     // CB_status2
  {1, 15, 4, 7, LOW},     // CB_status3
  {16, 17, 5, 6, HIGH},   // CB_status4
  {22, 23, 11, 20, HIGH}, // CB_status5
  {12, 13, 12, 19, HIGH}, // CB_status6
  {14, 0, 13, 18, HIGH},  // CB_status7
  {18, 19, 14, 17, HIGH}, // CB_status8
  {20, 21, 15, 16, HIGH}, // CB_status9
  {10, 11, 15, 16, HIGH}, // CB_status10
};

boolean prev_CB_status = HIGH;  

// Update status LEDs by writing to the status LEDs shift register data pin
void outputLEDs() {
  digitalWrite(STATUS_LATCH_PIN, LOW);
  for(int i = num_status_register_pins - 1; i >=  0; i--){
    digitalWrite(STATUS_CLOCK_PIN, LOW);

    int val = LEDregisters[i];

    digitalWrite(STATUS_DATA_PIN, val);
    digitalWrite(STATUS_CLOCK_PIN, HIGH);

  }
  digitalWrite(STATUS_LATCH_PIN, HIGH);
}

// Write status LED shift registers according to each status
void writeLEDRegister() {
  for (int i = 0; i < NUM_STATUSES; i++) {
    LEDregisters[statuses_array[i].green_LED] = statuses_array[i].state;
    LEDregisters[statuses_array[i].red_LED] = !statuses_array[i].state;
  }
  outputLEDs();
}

// Set cb status to high and set spring charged status to discharged
void closeCB() {
  prev_CB_status = statuses_array[CB_status0].state;
  statuses_array[CB_status0].state = LOW;  
  if (prev_CB_status == HIGH) {
    statuses_array[CB_status1].state = LOW;
    spring_charge_start_time = millis();    // start timer
    spring_charge_timer_running = 1;
  }
}

// Set cb status to low and saves previous cb status
void openCB() {
    prev_CB_status = statuses_array[CB_status0].state;
    statuses_array[CB_status0].state = HIGH; 
}

// Change status LEDs depending on button presses, including spring charge behaviour on CB close
void processButton(unsigned char key) {
  if (key == statuses_array[CB_status0].green_button) {
    openCB();
  } else if (key == statuses_array[CB_status0].red_button) {
    closeCB();
  } else {    // ADD ELSE IF STATEMENTS HERE TO ASSIGN SPECIAL BEHAVOUR TO BUTTONS
    if (key == statuses_array[CB_status1].green_button || key == statuses_array[CB_status1].red_button) {
      spring_charge_timer_running = 0;  // stop auto timer
    }
    for (int i = 0; i < NUM_STATUSES; i++) {
      if (key == statuses_array[i].green_button) {
        statuses_array[i].state = HIGH;
      }
      if (key == statuses_array[i].red_button) {
        statuses_array[i].state = LOW;
      }
    }
  }
}

void setup() {
  // Serial.begin(9600);  // for debugging 
  pinMode(TRIP_INPUT_PIN, INPUT);
  pinMode(CLOSE_INPUT_PIN, INPUT);
  pinMode(A1, INPUT);   // Button Set 1
  pinMode(A2, INPUT);   // Button Set 2
  pinMode(A5, INPUT);   // Button Set 3
  pinMode(POT_PIN, INPUT);

  // Status LED shift registers
  pinMode(STATUS_LATCH_PIN, OUTPUT);
  pinMode(STATUS_CLOCK_PIN, OUTPUT);
  pinMode(STATUS_DATA_PIN, OUTPUT);

  // clear LED pins
  for (int i = 0; i< num_status_register_pins; i++) {
    LEDregisters[i] = LOW;
  }
  outputLEDs();

  // Below registerKey parameters are (SW value in schematic, adc reading)
  // Button Set 1 values
  buttonSet1.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet1.registerKey(1, 0);   // Status 4 RedButton (SW1 in schematic)
  buttonSet1.registerKey(2, 129); // Status 4 GreenButton 
  buttonSet1.registerKey(3, 253); // Status 3 RedButton 
  buttonSet1.registerKey(4, 378); // Status 3 GreenButton 
  buttonSet1.registerKey(5, 502); // Status 2 RedButton 
  buttonSet1.registerKey(6, 634); // Status 2 GreenButton 
  buttonSet1.registerKey(7, 758); // Status 1 RedButton 
  buttonSet1.registerKey(8, 890); // Status 1 GreenButton 

  // Button Set 2 values 
  buttonSet2.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet2.registerKey(9, 0);    // Status 8 RedButton 
  buttonSet2.registerKey(10, 155); // Status 8 GreenButton 
  buttonSet2.registerKey(11, 311); // Status 7 RedButton 
  buttonSet2.registerKey(12, 459); // Status 7 GreenButton 
  buttonSet2.registerKey(13, 591); // Status 6 RedButton 
  buttonSet2.registerKey(14, 740); // Status 6 GreenButton 
  buttonSet2.registerKey(15, 885); // Status 5 RedButton 

  // Button Set 3 values 
  buttonSet3.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet3.registerKey(16, 0);    // Status 11 RedButton 
  buttonSet3.registerKey(17, 155);  // Status 11 GreenButton 
  buttonSet3.registerKey(18, 311);  // Status 10 RedButton 
  buttonSet3.registerKey(19, 459);  // Status 10 GreenButton 
  buttonSet3.registerKey(20, 591);  // Status 9 RedButton 
  buttonSet3.registerKey(21, 740);  // Status 9 GreenButton 
  buttonSet3.registerKey(22, 885);  // Status 5 RedButton 

}

void loop() {
  // Analogue current source potentiometer and display
  pot_value = analogRead(POT_PIN);
  pot_value = pot_value/1023 * 2;   //map adc value to 0-2A
  display.set(pot_value, 2, 0);
  display.show();

  // Left set of buttons
  unsigned char key1 = buttonSet1.getKey();
  processButton(key1);

  // Right set of buttons
  unsigned char key2 = buttonSet2.getKey();
  processButton(key2);

  int trip_signal = digitalRead(TRIP_INPUT_PIN);
  if (trip_signal == HIGH) {  
    openCB();
  }

  int close_signal = digitalRead(CLOSE_INPUT_PIN);
  // reclose CB from relay signal if spring is charged
  if (close_signal == HIGH && CB_status1 == HIGH) {
    closeCB();
  }

  if ((statuses_array[CB_status1].state == LOW) && (spring_charge_timer_running)) {
      if ((millis() - spring_charge_start_time) >= SPRING_CHARGE_DURATION) {
        statuses_array[CB_status1].state = HIGH;  // if delay has passed, spring finishes charging
        spring_charge_timer_running = 0;
      }
  }
  writeLEDRegister();
}
