/*
Prototype 2:

  CB status trips based on signal and manual button inputs.
  Out shift register code modified from https://www.instructables.com/3-Arduino-pins-to-24-output-pins/
*/
#include <ezAnalogKeypad.h>

// Defines to make code more readable
bool SPRING_CHARGE_TIMER_RUNNING = 0; // Timer for spring charged status re-charging
unsigned long SPRING_CHARGE_START_TIME;

#define TRIP_INPUT_PIN 9  // Digital pin used for CB trip
#define CLOSE_INPUT_PIN 10

// 74HC595 shift register pins and variables for LEDs
#define STATUS_DATA_PIN 2  //pin 14 DS
#define STATUS_LATCH_PIN 3 //pin 12 ST_CP
#define STATUS_CLOCK_PIN 4  //pin 11 SH_CP

#define num_status_registers 3
#define num_status_register_pins num_status_registers * 8
boolean LEDregisters[num_status_register_pins];

struct Status {
  int green_LED;  // shift register pin that the LED are connected to  
  int red_LED;    // shift register pin that the LED are connected to  
  int green_button;   // position of button (as mapped in setup() function)
  int red_button;     // position of button (as mapped in setup() function)
  boolean state;      // output logic state
};

#define NUM_STATUSES 11

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
ezAnalogKeypad buttonSet1(A1);   
ezAnalogKeypad buttonSet2(A2);  
ezAnalogKeypad buttonSet3(A5);  

// array index of each status 
// CHANGE THIS TO MOVE THINGS AROUND ON THE USER INTERFACE (0 being the top status on the user interface)
#define CB_status1 0  // CB position 
#define CB_status2 1  // spring charge 
#define CB_status3 2  // blank status
#define CB_status4 3  // blank status
#define CB_status5 4  // blank status
#define CB_status6 5  // blank status
#define CB_status7 6  // blank status
#define CB_status8 7  // blank status
#define CB_status9 8  // blank status
#define CB_status10 9 // blank status
#define CB_status11 10  // blank status

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
  prev_CB_status = statuses_array[CB_status1].state;
  statuses_array[CB_status1].state = LOW;  
  if (prev_CB_status == HIGH) {
    statuses_array[CB_status2].state = LOW;
    SPRING_CHARGE_START_TIME = millis();    // start timer
    SPRING_CHARGE_TIMER_RUNNING = 1;
  }
}

// Set cb status to low and saves previous cb status
void openCB() {
    prev_CB_status = statuses_array[CB_status1].state;
    statuses_array[CB_status1].state = HIGH; 
}

// Change status LEDs depending on button presses, including spring charge behaviour on CB close
void processButton(unsigned char key) {
  if (key == statuses_array[CB_status1].green_button) {
    openCB();
  } else if (key == statuses_array[CB_status1].red_button) {
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
  pinMode(A1, INPUT);   // Button Set 1
  pinMode(A2, INPUT);   // Button Set 2
  pinMode(A5, INPUT);   // Button Set 3

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

  if ((statuses_array[CB_status2].state == LOW) && (SPRING_CHARGE_TIMER_RUNNING)) {
      if ((millis() - SPRING_CHARGE_START_TIME) >= 4000) {
        statuses_array[CB_status2].state = HIGH;  // if 4 seconds have passed since CB HIGH, spring finishes charging
        SPRING_CHARGE_TIMER_RUNNING = 0;
      }
  }
  writeLEDRegister();
}
