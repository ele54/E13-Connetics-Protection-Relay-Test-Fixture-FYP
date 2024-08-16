/*
Prototype 2:

  CB status trips based on signal and manual button inputs.
  Out shift register code modified from https://www.instructables.com/3-Arduino-pins-to-24-output-pins/
*/
#include <ezAnalogKeypad.h>

// Defines to make code more readable
bool SPRING_CHARGE_TIMER_RUNNING = 0; // Timer for spring charged status re-charging
unsigned long SPRING_CHARGE_START_TIME;

#define TRIP_INPUT_PIN 2  // Digital pin used for CB trip
#define CLOSE_INPUT_PIN 3

// 74HC595 shift register pins and variables for status LEDs
#define STATUS_DATA_PIN 4  //pin 14 DS on shift register
#define STATUS_LATCH_PIN 5 //pin 12 ST_CP on shift register
#define STATUS_CLOCK_PIN 6  //pin 11 SH_CP on shift register

#define num_status_registers 3
#define num_status_register_pins num_status_registers * 8
boolean LEDregisters[num_status_register_pins];

struct Status {
  int green_LED;  // shift register pins that the LEDs are connected to  
  int red_LED;
  int green_button;   // position of buttons 
  int red_button;
  boolean state;
};

#define NUM_STATUSES 10

Status statuses_array[NUM_STATUSES] = {
  {2, 1, 1, 10, LOW}, // pin mapping for each status group of LEDs and buttons
  {3, 4, 2, 9, HIGH}, 
  {5, 7, 3, 8, HIGH}, 
  {6, 8, 4, 7, LOW},
  {17, 16, 5, 6, HIGH},
  {0, 15, 11, 20, HIGH},
  {14, 13, 12, 19, HIGH},
  {12, 11, 13, 18, HIGH},
  {9, 10, 14, 17, HIGH},
  {21, 22, 15, 16, HIGH},
};

// analog pins are A0(14) to A5(19)
ezAnalogKeypad buttonSet1(A0);   
ezAnalogKeypad buttonSet2(A1);  

// array index of each status 
// CHANGE THIS TO MOVE THINGS AROUND ON THE USER INTERFACE (0 being the top status on the user interface)
#define CB_status 0  
#define spring_charge_status 1  
#define gas_pressure_status 2 
#define earth_switch_status 3  
#define trip_circuit_supervision_status 4  
#define service_position_status 5
#define generic_status1 6
#define generic_status2 7
#define generic_status3 8
#define generic_status4 9

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

// Set cb status to high and set spring charge status to discharged
void closeCB() {
  prev_CB_status = statuses_array[CB_status].state;
  statuses_array[CB_status].state = LOW;  
  if (prev_CB_status == HIGH) {
    statuses_array[spring_charge_status].state = LOW;
    SPRING_CHARGE_START_TIME = millis();    // start timer
    SPRING_CHARGE_TIMER_RUNNING = 1;
  }
}

// Set cb status to low and saves previous cb status
void openCB() {
    prev_CB_status = statuses_array[CB_status].state;
    statuses_array[CB_status].state = HIGH; 
}

// Change status LEDs depending on button presses, including spring charge behaviour on CB close
void processButton(unsigned char key) {
  if (key == statuses_array[CB_status].green_button) {
    openCB();
  } else if (key == statuses_array[CB_status].red_button) {
    closeCB();
  } else {    // ADD ELSE IF STATEMENTS HERE TO ASSIGN SPECIAL BEHAVOUR TO BUTTONS
    if (key == statuses_array[spring_charge_status].green_button || key == statuses_array[spring_charge_status].red_button) {
      SPRING_CHARGE_TIMER_RUNNING = 0;  // stop auto timer
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
  // Serial.begin(9600);  // for debugging only
  pinMode(TRIP_INPUT_PIN, INPUT);
  pinMode(CLOSE_INPUT_PIN, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // Status LED shift registers
  pinMode(STATUS_LATCH_PIN, OUTPUT);
  pinMode(STATUS_CLOCK_PIN, OUTPUT);
  pinMode(STATUS_DATA_PIN, OUTPUT);

  // clear LED pins
  for (int i = 0; i< num_status_register_pins; i++) {
    LEDregisters[i] = LOW;
  }
  outputLEDs();

  // Left hand side buttons
  buttonSet1.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet1.registerKey(1, 0); 
  buttonSet1.registerKey(2, 100); 
  buttonSet1.registerKey(3, 200); 
  buttonSet1.registerKey(4, 300); 
  buttonSet1.registerKey(5, 400); 
  buttonSet1.registerKey(6, 500);
  buttonSet1.registerKey(7, 600); 
  buttonSet1.registerKey(8, 700);
  buttonSet1.registerKey(9, 800); 
  buttonSet1.registerKey(10, 900);

  // Right hand side buttons 
  buttonSet2.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet2.registerKey(20, 0);  
  buttonSet2.registerKey(19, 100); 
  buttonSet2.registerKey(18, 200); 
  buttonSet2.registerKey(17, 300); 
  buttonSet2.registerKey(16, 400); 
  buttonSet2.registerKey(15, 500); 
  buttonSet2.registerKey(14, 600); 
  buttonSet2.registerKey(13, 700); 
  buttonSet2.registerKey(12, 800); 
  buttonSet2.registerKey(11, 900); 
}

void loop() {
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
  if (close_signal == HIGH) {
    closeCB();
  }

  if ((statuses_array[spring_charge_status].state == LOW) && (SPRING_CHARGE_TIMER_RUNNING)) {
      if ((millis() - SPRING_CHARGE_START_TIME) >= 4000) {
        statuses_array[spring_charge_status].state = HIGH;  // if 4 seconds have passed since CB HIGH, spring finishes charging
        SPRING_CHARGE_TIMER_RUNNING = 0;
      }
  }
  writeLEDRegister();
}
