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

// CD4014 shift register pins and variables
#define in_data_pin 3 // pin 3 Q8
#define in_latch_pin 4  // pin 9 PE
#define in_clock_pin 5  // pin 10 CP

uint16_t ref_inputs =0;

// these are what which bit of the CD4014 shift register is connected to
// #define auxiliary_ref_input = 2;
#define gas_pressure_ref_input 6
#define earth_switch_ref_input 7
#define supervision_ref_input 12
#define service_position_ref_input 14

// 74HC595 shift register pins and variables
#define out_data_pin 6  //pin 14 DS
#define out_latch_pin 7 //pin 12 ST_CP
#define out_clock_pin 8  //pin 11 SH_CP

#define number_of_74hc595s 2
#define numOfRegisterPins number_of_74hc595s * 8
boolean registers[numOfRegisterPins];

// what each bit of the 74HC595 shift register is (output signals)
#define gas_pressure_status_output 4
#define earth_switch_status_output 3
#define supervision_status_output 2
#define service_position_status_output 1

// analog pins are A0(14) to A5(19)
ezAnalogKeypad buttonSet1(A0);   // Preset CB status buttons
ezAnalogKeypad buttonSet2(A1);  // Currently used as generic statuses' buttons

State gas_pressure_switch = CLOSED;   // closed for normal, open for low
State earth_switch = CLOSED;  
State supervision_status_switch = CLOSED;   // closed for normal, open for fault
State service_position_switch = CLOSED;   // closed for racked in, open for racked out 

// Load byte in from shift register
// Code adapted from https://forums.adafruit.com/viewtopic.php?t=41906
uint16_t shiftIn(int latch_pin, int clock_pin, int data_pin) {
  // Pulse to load data
  digitalWrite(latch_pin, HIGH);

  int pin_state = 0;
  uint16_t data_in = 0;

  for (int i = 0; i < 16; i++) {
    digitalWrite(clock_pin, HIGH);
    digitalWrite(clock_pin, LOW);

    if (i == 0) digitalWrite(latch_pin, LOW);

    pin_state = digitalRead(data_pin);
    if (pin_state == HIGH) {
      data_in = data_in | (1 << i);
    }
    digitalWrite(clock_pin, HIGH);
  }
  return data_in;
}

// Get reference signal bit from shift register input
boolean getBit(byte desired_bit) {
  boolean bit_state;
  bit_state = ref_inputs & (1 << desired_bit);
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
  buttonSet1.registerKey(1, 50); // button for gas pressure normal
  buttonSet1.registerKey(2, 456); // button for gas pressure low
  buttonSet1.registerKey(3, 619); // button for earth switch closed
  buttonSet1.registerKey(4, 704); // button for earth switch open

  buttonSet2.setNoPressValue(1000);  // analog value when no button is pressed
  buttonSet1.registerKey(1, 42); // button for trip circuit supervision normal
  buttonSet1.registerKey(2, 518); // button for trip circuit supervision fault
  buttonSet1.registerKey(3, 680); // button for racked in
  buttonSet1.registerKey(4, 772); // button for racked out
}

void loop() {
  // Process buttons
  unsigned char key1 = buttonSet1.getKey();
  switch (key1) {
    case 1:
      gas_pressure_switch = OPEN;   // Gas normal
      break;
    case 2:
      gas_pressure_switch = CLOSED;   // Gas low
      break;
    case 3:
      earth_switch = CLOSED;  
      break;
    case 4:
      earth_switch = OPEN;
      break;
  }

  // Buttons for generic statuses
  unsigned char key2 = buttonSet2.getKey();
  switch (key2) {
    case 1:
      supervision_status_switch = CLOSED;
      break;
    case 2:
      supervision_status_switch = OPEN;
      break;
    case 3:
      service_position_switch = CLOSED;
      break;
    case 4:
      service_position_switch = OPEN;
      break;
  }

  ref_inputs = shiftIn(in_latch_pin, in_clock_pin, in_data_pin);

  setStatusOutput(gas_pressure_ref_input, gas_pressure_switch, gas_pressure_status_output);
  setStatusOutput(earth_switch_ref_input, earth_switch, earth_switch_status_output);
  setStatusOutput(supervision_ref_input, supervision_status_switch, supervision_status_output);
  setStatusOutput(service_position_ref_input, service_position_switch, service_position_status_output);

  writeRegisters(out_latch_pin, out_clock_pin, out_data_pin);
  
  Serial.println("ref inputs: ");
  Serial.println(ref_inputs);
}
