/*
Prototype 2:

  CB status trips based on signal and manual button inputs.
  Out shift register code modified from https://www.instructables.com/3-Arduino-pins-to-24-output-pins/
*/
#include <ezAnalogKeypad.h>

// Defines to make code more readable
bool spring_charge_timer_running = 0; // Timer for spring charged status re-charging
unsigned long spring_charge_start_time;

#define trip_input_pin 2  // Digital pin used for CB trip
#define close_input_pin 12

// 74HC595 shift register pins and variables for LEDs
#define LED_data_pin 9  //pin 14 DS
#define LED_latch_pin 10 //pin 12 ST_CP
#define LED_clock_pin 11  //pin 11 SH_CP
#define numOfLEDRegisters 3
#define numOfLEDRegisterPins numOfLEDRegisters * 8
boolean LEDregisters[numOfLEDRegisterPins];

struct Status {
  int green_LED;  // shift register pin that the LEDs are connected to  
  int red_LED;
  int green_button;   // position of button 
  int red_button;
  boolean state;
};

#define NUM_STATUSES 10

Status statuses_array[NUM_STATUSES] = {
  {2, 1, 1, 10, LOW}, // breaker position
  {3, 4, 2, 9, HIGH}, // spring charge status
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

// write outputs to shift register data pin
void outputLEDs() {
  digitalWrite(LED_latch_pin, LOW);
  for(int i = numOfLEDRegisterPins - 1; i >=  0; i--){
    digitalWrite(LED_clock_pin, LOW);

    int val = LEDregisters[i];

    digitalWrite(LED_data_pin, val);
    digitalWrite(LED_clock_pin, HIGH);

  }
  digitalWrite(LED_latch_pin, HIGH);
}

// write LED shift registers according to each status
void writeLEDRegister() {
  for (int i = 0; i < NUM_STATUSES; i++) {
    LEDregisters[statuses_array[i].green_LED] = statuses_array[i].state;
    LEDregisters[statuses_array[i].red_LED] = !statuses_array[i].state;
  }
  outputLEDs();
}

// set cb status to high and set spring charge status to discharged
void closeCB() {
  prev_CB_status = statuses_array[CB_status].state;
  statuses_array[CB_status].state = LOW;  
  if (prev_CB_status == HIGH) {
    statuses_array[spring_charge_status].state = LOW;
    spring_charge_start_time = millis();    // start timer
    spring_charge_timer_running = 1;
  }
}

// set cb status to low and saves previous cb status
void openCB() {
    prev_CB_status = statuses_array[CB_status].state;
    statuses_array[CB_status].state = HIGH; 
}

void processButton(unsigned char key) {
  if (key == statuses_array[CB_status].green_button) {
    openCB();
  } else if (key == statuses_array[CB_status].red_button) {
    closeCB();
  } else {
    if (key == statuses_array[spring_charge_status].green_button || key == statuses_array[spring_charge_status].red_button) {
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
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trip_input_pin, INPUT);
  pinMode(close_input_pin, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // LED shift registers
  pinMode(LED_latch_pin, OUTPUT);
  pinMode(LED_clock_pin, OUTPUT);
  pinMode(LED_data_pin, OUTPUT);

  // clear LED pins
  for (int i = 0; i< numOfLEDRegisterPins; i++) {
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

  int trip_signal = digitalRead(trip_input_pin);
  if (trip_signal == HIGH) {  
    openCB();
  }

  int close_signal = digitalRead(close_input_pin);
  if (close_signal == HIGH) {
    closeCB();
  }

  if ((statuses_array[spring_charge_status].state == LOW) && (spring_charge_timer_running)) {
      if ((millis() - spring_charge_start_time) >= 4000) {
        statuses_array[spring_charge_status].state = HIGH;  // if 4 seconds have passed since CB HIGH, spring finishes charging
        spring_charge_timer_running = 0;
      }
  }
  writeLEDRegister();
}
