/*
Prototype 2:

  Code for testing prototype with:
    two resistor ladders with four buttons each
    one SIPO 75HC595
    one PISO CD4014
  SIPO shift register code adapted from https://www.instructables.com/3-Arduino-pins-to-24-output-pins/
  PISO CD4014 code adapted from https://forums.adafruit.com/viewtopic.php?t=41906
*/
#include <ezAnalogKeypad.h>

// Defines to make code more readable
enum State {OPEN, CLOSED};
bool timer_running = 0; // Timer for spring charged status re-charging
unsigned long start_time;
unsigned char prev_key1 = 0;

#define trip_input 2  // Digital pin used for CB trip

// CD4014 shift register pins and variables
#define in_data_pin 3 // pin 3 Q8
#define in_latch_pin 4  // pin 9 PE
#define in_clock_pin 5  // pin 10 CP

uint16_t ref_inputs = 1;

// these are what which bit of the CD4014 shift register is connected to
#define spring_charge_ref_input 7
#define auxiliary_ref_input 6
#define supervision_ref_input 15
#define service_position_ref_input 14

// 74HC595 shift register pins and variables
#define out_data_pin 6  //pin 14 DS
#define out_latch_pin 7 //pin 12 ST_CP
#define out_clock_pin 8  //pin 11 SH_CP

#define number_of_74hc595s 2
#define numOfRegisterPins number_of_74hc595s * 8
boolean registers[numOfRegisterPins];

// what each bit of the 74HC595 shift register is (output signals)
#define spring_charge_status_output 5
#define auxiliary_52A_output 4
#define auxiliary_52B_output 3
#define supervision_status_output 2
#define service_position_status_output 1

// analog pins are A0(14) to A5(19)
ezAnalogKeypad buttonSet1(A0);   // Preset CB status buttons
ezAnalogKeypad buttonSet2(A1);  // Currently used as generic statuses' buttons

State CB_status = CLOSED;  
State prev_CB_status = CLOSED;  
State prev_spring_status = CLOSED;  
State spring_status_switch = CLOSED;   // closed for charged, open for discharged
State supervision_status_switch = CLOSED;   // closed for normal, open for fault
State service_position_switch = CLOSED;   // closed for racked in, open for racked out 

// Load byte in from shift register
// Code adapted from https://forums.adafruit.com/viewtopic.php?t=41906
uint16_t shiftIn(int latch_pin, int clock_pin, int data_pin) {
  // Pulse to load data
  digitalWrite(latch_pin, HIGH);

  int pin_state = 0;
  uint16_t data_in = 0;

  for(int i=0;i<16;i++) {
    /* clock low-high-low */
    digitalWrite(clock_pin, HIGH);
    digitalWrite(clock_pin, LOW);

    /* if this is the first bit, then we're done with the parallel load */
    if (i==0) digitalWrite(latch_pin, LOW);

    /* shift the new bit in */
    data_in<<=1;
    if (digitalRead(data_pin)) data_in|=1;
  }
  return data_in;
}

// Get reference signal bit from shift register input
boolean getBit(byte desired_bit) {
  boolean bit_state;
  bit_state = ref_inputs & (1 << (desired_bit - 1));
  return bit_state;
}

// Set status output value based on reference input, switch position
void setStatusOutput(byte ref_bit, State switch_position, byte output_bit) {
  byte ref_signal = getBit(ref_bit);
  switch (switch_position) {
    case CLOSED:
      setRegisterPin(output_bit, ref_signal);
      break;
    case OPEN:
      setRegisterPin(output_bit, !ref_signal);
      break;
  }
}

//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers[i] = LOW;
  }
} 

//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(int latch_pin, int clock_pin, int data_pin) {

  digitalWrite(latch_pin, LOW);

  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    digitalWrite(clock_pin, LOW);

    int val = registers[i];

    digitalWrite(data_pin, val);
    digitalWrite(clock_pin, HIGH);

  }
  digitalWrite(latch_pin, HIGH);

}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // In shift register
  pinMode(in_latch_pin, OUTPUT);
  pinMode(in_clock_pin, OUTPUT);
  pinMode(in_data_pin, INPUT);
  digitalWrite(in_latch_pin, LOW);
  digitalWrite(in_clock_pin, LOW);

  // Out shift register
  pinMode(out_latch_pin, OUTPUT);
  pinMode(out_clock_pin, OUTPUT);
  pinMode(out_data_pin, OUTPUT);
  //reset all register pins
  clearRegisters();
  writeRegisters(out_latch_pin, out_clock_pin, out_data_pin);

  pinMode(A0, INPUT);   
  pinMode(A1, INPUT);   

  buttonSet1.setNoPressValue(1000);  // analog value when no button is pressed
  buttonSet1.registerKey(1, 5); // button for gas pressure normal
  buttonSet1.registerKey(2, 456); // button for gas pressure low
  buttonSet1.registerKey(3, 619); // button for earth switch closed
  buttonSet1.registerKey(4, 704); // button for earth switch open

  buttonSet2.setNoPressValue(1000);  // analog value when no button is pressed
  buttonSet2.registerKey(5, 42); // button for trip circuit supervision normal
  buttonSet2.registerKey(6, 518); // button for trip circuit supervision fault
  buttonSet2.registerKey(7, 680); // button for racked in
  buttonSet2.registerKey(8, 772); // button for racked out
}

void loop() {
  // Process buttons
  unsigned char key1 = buttonSet1.getKey();
  if (key1 != prev_key1) {
    switch (key1) {
      case 1:
        if (spring_status_switch != CLOSED) {
          prev_spring_status = spring_status_switch;
          spring_status_switch = CLOSED;   // Charged
        }
        timer_running = 0;  // Stop auto timer
        break;
      case 2:
        if (spring_status_switch != OPEN) {
          prev_spring_status = spring_status_switch;
          spring_status_switch = OPEN;   // Discharged
        }
        timer_running = 0;  // Stop auto timer
        break;
      case 3:
        if (CB_status != CLOSED) {
          prev_CB_status = CB_status;
          CB_status = CLOSED;  
        }
        if ((prev_CB_status == OPEN) && (spring_status_switch == OPEN)) {
          start_time = millis();    // start timer
          timer_running = 1;
          Serial.println("Restart timer: ");
          Serial.println(start_time);
        }
        break;
      case 4:
        if (CB_status != OPEN) {
          prev_CB_status = CB_status;
          CB_status = OPEN;  
          prev_spring_status = spring_status_switch;
          spring_status_switch = OPEN;
        }      
        break;
    }
    unsigned char prev_key1 = key1;
  }

  // Buttons for generic statuses
  unsigned char key2 = buttonSet2.getKey();
  switch (key2) {
    case 5:
      supervision_status_switch = CLOSED;
      break;
    case 6:
      supervision_status_switch = OPEN;
      break;
    case 7:
      service_position_switch = CLOSED;
      break;
    case 8:
      service_position_switch = OPEN;
      break;
  }

  ref_inputs = shiftIn(in_latch_pin, in_clock_pin, in_data_pin);

  int trip_signal = digitalRead(trip_input);
  int auxiliary_signal = getBit(auxiliary_ref_input);
  if (trip_signal == HIGH) {  
    prev_CB_status = CB_status;
    CB_status = OPEN; 
    prev_spring_status = spring_status_switch;
    spring_status_switch = OPEN;
    setRegisterPin(auxiliary_52A_output, !auxiliary_signal);
    setRegisterPin(auxiliary_52B_output, auxiliary_signal);
  }

  switch (CB_status) {
    case OPEN:
      setRegisterPin(auxiliary_52A_output, !auxiliary_signal); // open = output opposite of input signal
      setRegisterPin(auxiliary_52B_output, auxiliary_signal);  // closed = connect output to the input signal
      break;
    case CLOSED:
      setRegisterPin(auxiliary_52A_output, auxiliary_signal);
      setRegisterPin(auxiliary_52B_output, !auxiliary_signal);
      if ((spring_status_switch == OPEN) && (timer_running)) {
        // Serial.println("timer ran for: ");
        // Serial.println((millis() - start_time));
        if ((millis() - start_time) >= 4000) {
          spring_status_switch = CLOSED;  // if 4 seconds have passed since CB closed, spring finishes charging
          timer_running = 0;
        }
      }
      break;
  }

  setStatusOutput(spring_charge_ref_input, spring_status_switch, spring_charge_status_output);
  setStatusOutput(supervision_ref_input, supervision_status_switch, supervision_status_output);
  setStatusOutput(service_position_ref_input, service_position_switch, service_position_status_output);

  writeRegisters(out_latch_pin, out_clock_pin, out_data_pin);
  
}
