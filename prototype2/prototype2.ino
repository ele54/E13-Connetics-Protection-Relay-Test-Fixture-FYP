/*
Prototype 2:

  CB status trips based on signal and manual button inputs.
  Out shift register code modified from https://www.instructables.com/3-Arduino-pins-to-24-output-pins/
  In shift register code modified from https://wiki-content.arduino.cc/en/Tutorial/ShftIn14
*/
#include <ezAnalogKeypad.h>

// Defines to make code more readable
enum State {OPEN, CLOSED};
bool timer_running = 0; // Timer for spring charged status re-charging
int start_time;

// digital pins are 2 to 13
#define trip_input 2

// PISO shift register pins and variables
#define in_data_pin 3 // pin 3 serial out
#define in_latch_pin 4  // pin 9 PE
#define in_clock_pin 5  // pin 10 CP
byte ref_inputs = B11111111;

byte auxiliary_ref_input = 2;
byte gas_pressure_ref_input = 3;
byte earth_switch_ref_input = 4;
byte supervision_ref_input = 5;
byte service_position_ref_input = 6;
byte spring_ref_input = 7;
// byte generic_ref_input1 = 8;
// byte generic_ref_input2 = 9;
// byte generic_ref_input3 = 10;
// byte generic_ref_input4 = 11;
// byte generic_ref_input5 = 12;

// SIPO shift register pins and variables
#define out_data_pin 11  //pin 14 DS
#define out_latch_pin 12 //pin 12 ST_CP
#define out_clock_pin 13  //pin 11 SH_CP
#define number_of_74hc595s 2
#define numOfRegisterPins number_of_74hc595s * 8
boolean registers[numOfRegisterPins];

enum State {OPEN, CLOSED};
bool spring_charge_timer_running = 0; // Timer for spring charged status re-charging
unsigned long spring_charge_start_time;

#define trip_input_pin 2  // Digital pin used for CB trip
#define close_input_pin 12
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

// 74HC595 shift register pins and variables for output signals
#define out_data_pin 6  //pin 14 DS
#define out_latch_pin 7 //pin 12 ST_CP
#define out_clock_pin 8  //pin 11 SH_CP
#define number_of_74hc595s 2
#define numOfRegisterPins number_of_74hc595s * 8
boolean registers[numOfRegisterPins];
#define auxiliary_52A_output 1
#define auxiliary_52B_output 2

// 74HC595 shift register pins and variables for LEDs
#define LED_data_pin 9  //pin 14 DS
#define LED_latch_pin 10 //pin 12 ST_CP
#define LED_clock_pin 11  //pin 11 SH_CP
#define numOfLEDRegisters 3
#define numOfLEDRegisterPins numOfLEDRegisters * 8
boolean LEDregisters[numOfLEDRegisterPins];
// Positions of the output LEDs connected to each bit of the 74HC595 shift registers
#define CB_status_LEDg 2
#define CB_status_LEDr 1
#define gas_pressure_status_LEDg 3
#define gas_pressure_status_LEDr 4
#define earth_switch_status_LEDg 5
#define earth_switch_status_LEDr 7
#define generic_status_LED1g 6

#define generic_status_LED1r 8
#define generic_status_LED2g 17
#define generic_status_LED2r 16

#define service_position_status_LEDg 0
#define service_position_status_LEDr 15
#define spring_charge_status_LEDg 14
#define spring_charge_status_LEDr 13
#define circuit_supervision_status_LEDg 12
#define circuit_supervision_status_LEDr 11
#define generic_status_LED3g 9
#define generic_status_LED3r 10

#define generic_status_LED4g 21
#define generic_status_LED4r 22

// analog pins are A0(14) to A5(19)
ezAnalogKeypad buttonSet1(A0);   // Preset CB status buttons
ezAnalogKeypad buttonSet2(A1);  // Currently used as generic statuses' buttons

State CB_status = CLOSED;  
State prev_CB_status = CLOSED;  
State gas_pressure_switch = OPEN;   // closed for gas low, open for gas normal
State earth_switch = CLOSED;  // closed for earthed, open for not earthed
State circuit_supervision_status_switch = CLOSED;   // closed for normal, open for fault
State service_position_switch = CLOSED;   // closed for racked in, open for racked out 
State spring_status_switch = CLOSED;   // closed for charged, open for discharged
State generic_status_switch1 = CLOSED;
State generic_status_switch2 = CLOSED;
State generic_status_switch3 = CLOSED;
State generic_status_switch4 = CLOSED;

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

// write LED shift registers according to each status
void writeLEDOutputs() {
  LEDregisters[CB_status_LEDg] = CB_status;
  LEDregisters[CB_status_LEDr] = !CB_status;
  LEDregisters[gas_pressure_status_LEDg] = !gas_pressure_switch;
  LEDregisters[gas_pressure_status_LEDr] = gas_pressure_switch;
  LEDregisters[earth_switch_status_LEDg] = !earth_switch;
  LEDregisters[earth_switch_status_LEDr] = earth_switch;
  LEDregisters[generic_status_LED1g] = generic_status_switch1;
  LEDregisters[generic_status_LED1r] = !generic_status_switch1;
  LEDregisters[generic_status_LED2g] = generic_status_switch2;
  LEDregisters[generic_status_LED2r] = !generic_status_switch2;
  LEDregisters[service_position_status_LEDg] = service_position_switch;
  LEDregisters[service_position_status_LEDr] = !service_position_switch;
  LEDregisters[spring_charge_status_LEDg] = spring_status_switch;
  LEDregisters[spring_charge_status_LEDr] = !spring_status_switch;
  LEDregisters[circuit_supervision_status_LEDg] = circuit_supervision_status_switch;
  LEDregisters[circuit_supervision_status_LEDr] = !circuit_supervision_status_switch;
  LEDregisters[generic_status_LED3g] = generic_status_switch3;
  LEDregisters[generic_status_LED3r] = !generic_status_switch3;

  LEDregisters[generic_status_LED4g] = generic_status_switch4;
  LEDregisters[generic_status_LED4r] = !generic_status_switch4;

  digitalWrite(LED_latch_pin, LOW);

  for(int i = numOfLEDRegisterPins - 1; i >=  0; i--){
    digitalWrite(LED_clock_pin, LOW);

    int val = LEDregisters[i];

    digitalWrite(LED_data_pin, val);
    digitalWrite(LED_clock_pin, HIGH);

  }
  digitalWrite(LED_latch_pin, HIGH);

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trip_input_pin, INPUT);
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
  
  // LED shift registers
  pinMode(LED_latch_pin, OUTPUT);
  pinMode(LED_clock_pin, OUTPUT);
  pinMode(LED_data_pin, OUTPUT);

  pinMode(A0, INPUT);   
  pinMode(A1, INPUT);   

  // Left hand side buttons
  buttonSet1.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet1.registerKey(1, 10); // button for CB manual close
  buttonSet1.registerKey(2, 100); // button for gas pressure normal
  buttonSet1.registerKey(3, 200); // button for earth switch not earthed
  buttonSet1.registerKey(4, 300); // button for generic status1 closed
  buttonSet1.registerKey(5, 400); // button for generic status2 closed
  buttonSet1.registerKey(6, 500); // button for generic status2 open
  buttonSet1.registerKey(7, 600); // button for generic status1 open
  buttonSet1.registerKey(8, 700); // button for earth switch closed
  buttonSet1.registerKey(9, 800); // button for gas pressure low
  buttonSet1.registerKey(10, 900); // button for CB manual open

  // Right hand side buttons 
  buttonSet2.setNoPressValue(1000);  // analog value when no button is pressed
  buttonSet2.registerKey(1, 10);  // service position status racked in
  buttonSet2.registerKey(2, 100); // spring charge status charged
  buttonSet2.registerKey(3, 200); // trip circuit supervision status normal
  buttonSet2.registerKey(4, 300); // generic status3 closed
  buttonSet2.registerKey(5, 400); // generic status4 closed
  buttonSet2.registerKey(6, 500); // generic status4 open
  buttonSet2.registerKey(7, 600); // generic status3 open
  buttonSet2.registerKey(8, 700); // trip circuit supervision status fault
  buttonSet2.registerKey(9, 800); // spring charge status discharged
  buttonSet2.registerKey(10, 900);  // service position status racked out

}

void loop() {
  // Process buttons
  unsigned char key1 = buttonSet1.getKey();
  switch (key1) {
    case 1: // CB status: (manual) close
    Serial.println("close cb");
      prev_CB_status = CB_status;
      CB_status = CLOSED;  
      if (prev_CB_status == OPEN) {
        spring_status_switch = OPEN;
        spring_charge_start_time = millis();    // start timer
        spring_charge_timer_running = 1;
        Serial.println("Restart timer: ");
        Serial.println(spring_charge_start_time);
        Serial.print("spring_status_switch after cb close: ");
        Serial.println(spring_status_switch);
      }
      break;
    case 2: // gas pressure: normal
      gas_pressure_switch = OPEN;
      break;
    case 3: // earth switch: not earthed
      earth_switch = OPEN;
      break;
    case 4:
      generic_status_switch1 = CLOSED;
      break;
    case 5:
      generic_status_switch2 = CLOSED;
      break;
    case 6:
      generic_status_switch2 = OPEN;
      break;
    case 7:
      generic_status_switch1 = OPEN;
      break;
    case 8: // earth switch: earthed
      earth_switch = CLOSED;
      break;
    case 9: // gas pressure: low
      gas_pressure_switch = CLOSED;
      break;
    case 10:  // CB status: (manual) open
      if (CB_status != OPEN) {
        prev_CB_status = CB_status;
        CB_status = OPEN;  
      }    
      break;
  }

  // Buttons for more statuses
  unsigned char key2 = buttonSet2.getKey();
  switch (key2) {
    case 1:
      spring_status_switch = CLOSED;
      break;
    case 2:
      spring_status_switch = OPEN;
      break;
    // case 3:
    //   generic_status_switch2 = CLOSED;
    //   break;
    // case 4:
    //   generic_status_switch2 = OPEN;
    //   break;
    // case 5:
    //   generic_status_switch3 = CLOSED;
    //   break;
    // case 6:
    //   generic_status_switch3 = OPEN;
    //   break;      
    // case 7:
    //   generic_status_switch4 = CLOSED;
    //   break;
    // case 8:
    //   generic_status_switch4 = OPEN;
    //   break;      
    // case 9:
    //   generic_status_switch5 = CLOSED;
    //   break;
    // case 10:
    //   generic_status_switch5 = OPEN;
    //   break;   
  }
  
  // // Buttons for generic statuses
  // unsigned char key3 = buttonSet3.getKey();
  // switch (key2) {
  //   case 1:
  //     generic_status_switch1 = CLOSED;
  //     break;
  //   case 2:
  //     generic_status_switch1 = OPEN;
  //     break;
  //   case 3:
  //     generic_status_switch2 = CLOSED;
  //     break;
  //   case 4:
  //     generic_status_switch2 = OPEN;
  //     break;
  //   case 5:
  //     generic_status_switch3 = CLOSED;
  //     break;
  //   case 6:
  //     generic_status_switch3 = OPEN;
  //     break;      
  //   case 7:
  //     generic_status_switch4 = CLOSED;
  //     break;
  //   case 8:
  //     generic_status_switch4 = OPEN;
  //     break;      
  //   case 9:
  //     generic_status_switch5 = CLOSED;
  //     break;
  //   case 10:
  //     generic_status_switch5 = OPEN;
  //     break;   
  // }

  ref_inputs = shiftIn(in_latch_pin, in_clock_pin, in_data_pin);

  // Buttons for generic statuses
  unsigned char key2 = buttonSet2.getKey();
  switch (key2) {
    case 1: // service position status: racked out
      service_position_switch = OPEN;
      break;
    case 2:  // spring charge discharged (switch open)
      spring_status_switch = OPEN;   // Discharged
      spring_charge_timer_running = 0;  // Stop auto timer
      break;
    case 3: // trip circuit supervision status: fault
      circuit_supervision_status_switch = OPEN;
      break; 
    case 4: 
      generic_status_switch3 = OPEN;
      break;
    case 5:
      generic_status_switch4 = OPEN;
      break;
    case 6:
      generic_status_switch4 = CLOSED;
      break;
    case 7:
      generic_status_switch3 = CLOSED;
      break;
    case 8:// trip circuit supervision status: normal
      circuit_supervision_status_switch = CLOSED;
      break;
    case 9:   // spring charge status: charged (switch closed)
      spring_status_switch = CLOSED;   // Charged
      spring_charge_timer_running = 0;  // Stop auto timer
      break;
    case 10:  // service position status: racked in
      service_position_switch = CLOSED;
      break;
  }

  ref_inputs = shiftIn(in_latch_pin, in_clock_pin, in_data_pin);

  int trip_signal = digitalRead(trip_input_pin);
  int auxiliary_signal = getBit(auxiliary_ref_input);
  if (trip_signal == HIGH) {  
    prev_CB_status = CB_status;
    CB_status = OPEN; 
  }

  int close_signal = digitalRead(close_input_pin);
  if (close_signal == HIGH) {
    prev_CB_status = CB_status;
    CB_status = CLOSED;  
    if (prev_CB_status == OPEN) {
      spring_status_switch = OPEN;
      spring_charge_start_time = millis();    // start timer
      spring_charge_timer_running = 1;
      Serial.println("Restart timer: ");
      Serial.println(spring_charge_start_time);
    }
  }

  switch (CB_status) {
    case OPEN:
      setRegisterPin(auxiliary_52A_output, !auxiliary_signal); // open = output opposite of input signal
      setRegisterPin(auxiliary_52B_output, auxiliary_signal);  // closed = connect output to the input signal
      break;
    case CLOSED:
      setRegisterPin(auxiliary_52A_output, auxiliary_signal);
      setRegisterPin(auxiliary_52B_output, !auxiliary_signal);
      if ((spring_status_switch == OPEN) && (spring_charge_timer_running)) {
        if ((millis() - spring_charge_start_time) >= 4000) {
          spring_status_switch = CLOSED;  // if 4 seconds have passed since CB closed, spring finishes charging
          spring_charge_timer_running = 0;
        }
      }
      break;
  }
  writeLEDOutputs();
}
