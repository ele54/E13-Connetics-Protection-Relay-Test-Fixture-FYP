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
ezAnalogKeypad buttonSet1(A0);   
ezAnalogKeypad buttonSet2(A1);  

boolean CB_status = HIGH;  
boolean prev_CB_status = HIGH;  
boolean gas_pressure_switch = LOW;   // HIGH for gas low, LOW for gas normal
boolean earth_switch = HIGH;  // HIGH for earthed, LOW for not earthed
boolean circuit_supervision_status_switch = HIGH;   // HIGH for normal, LOW for fault
boolean service_position_switch = HIGH;   // HIGH for racked in, LOW for racked out 
boolean spring_status_switch = HIGH;   // HIGH for charged, LOW for discharged
boolean generic_status_switch1 = HIGH;
boolean generic_status_switch2 = HIGH;
boolean generic_status_switch3 = HIGH;
boolean generic_status_switch4 = HIGH;

// write LED shift registers according to each status
void writeLEDOutputs() {
  LEDregisters[CB_status_LEDg] = !CB_status;
  LEDregisters[CB_status_LEDr] = CB_status;
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
  // write outputs to shift register data pin
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
  pinMode(close_input_pin, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // LED shift registers
  pinMode(LED_latch_pin, OUTPUT);
  pinMode(LED_clock_pin, OUTPUT);
  pinMode(LED_data_pin, OUTPUT);

  // Left hand side buttons
  buttonSet1.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet1.registerKey(1, 0); // button for CB manual close
  buttonSet1.registerKey(2, 100); // button for gas pressure normal
  buttonSet1.registerKey(3, 200); // button for earth switch not earthed
  buttonSet1.registerKey(4, 300); // button for generic status1 HIGH
  buttonSet1.registerKey(5, 400); // button for generic status2 HIGH
  buttonSet1.registerKey(6, 500); // button for generic status2 LOW
  buttonSet1.registerKey(7, 600); // button for generic status1 LOW
  buttonSet1.registerKey(8, 700); // button for earth switch HIGH
  buttonSet1.registerKey(9, 800); // button for gas pressure low
  buttonSet1.registerKey(10, 900); // button for CB manual LOW

  // Right hand side buttons 
  buttonSet2.setNoPressValue(1023);  // analog value when no button is pressed
  buttonSet2.registerKey(1, 0);  // service position status racked in
  buttonSet2.registerKey(2, 100); // spring charge status charged
  buttonSet2.registerKey(3, 200); // trip circuit supervision status normal
  buttonSet2.registerKey(4, 300); // generic status3 HIGH
  buttonSet2.registerKey(5, 400); // generic status4 HIGH
  buttonSet2.registerKey(6, 500); // generic status4 LOW
  buttonSet2.registerKey(7, 600); // generic status3 LOW
  buttonSet2.registerKey(8, 700); // trip circuit supervision status fault
  buttonSet2.registerKey(9, 800); // spring charge status discharged
  buttonSet2.registerKey(10, 900);  // service position status racked out

}

void loop() {
  // Left set of buttons
  unsigned char key1 = buttonSet1.getKey();
  switch (key1) {
    case 1: // CB status: (manual) LOW
      if (CB_status != LOW) {
        prev_CB_status = CB_status;
        CB_status = LOW;  
      }    
      break;
    case 2: // gas pressure: normal
      gas_pressure_switch = LOW;
      break;
    case 3: // earth switch: not earthed
      earth_switch = LOW;
      break;
    case 4:
      generic_status_switch1 = HIGH;
      break;
    case 5:
      generic_status_switch2 = HIGH;
      break;
    case 6:
      generic_status_switch2 = LOW;
      break;
    case 7:
      generic_status_switch1 = LOW;
      break;
    case 8: // earth switch: earthed
      earth_switch = HIGH;
      break;
    case 9: // gas pressure: low
      gas_pressure_switch = HIGH;
      break;
    case 10:  // CB status: (manual) close
      prev_CB_status = CB_status;
      CB_status = HIGH;  
      if (prev_CB_status == LOW) {
        spring_status_switch = LOW;
        spring_charge_start_time = millis();    // start timer
        spring_charge_timer_running = 1;
      }
      break;
  }

  // Right set of buttons
  unsigned char key2 = buttonSet2.getKey();
  Serial.println(analogRead(A1));
  switch (key2) {
    case 1: // service position status: racked out
      service_position_switch = LOW;
      break;
    case 2:  // spring charge discharged (switch LOW)
      spring_status_switch = LOW;   // Discharged
      spring_charge_timer_running = 0;  // Stop auto timer
      break;
    case 3: // trip circuit supervision status: fault
      circuit_supervision_status_switch = LOW;
      break; 
    case 4: 
      generic_status_switch3 = LOW;
      break;
    case 5:
      generic_status_switch4 = LOW;
      break;
    case 6:
      generic_status_switch4 = HIGH;
      break;
    case 7:
      generic_status_switch3 = HIGH;
      break;
    case 8:// trip circuit supervision status: normal
      circuit_supervision_status_switch = HIGH;
      break;
    case 9:   // spring charge status: charged (switch HIGH)
      spring_status_switch = HIGH;   // Charged
      spring_charge_timer_running = 0;  // Stop auto timer
      break;
    case 10:  // service position status: racked in
      service_position_switch = HIGH;
      break;
  }

  int trip_signal = digitalRead(trip_input_pin);
  if (trip_signal == HIGH) {  
    prev_CB_status = CB_status;
    CB_status = LOW; 
  }

  int close_signal = digitalRead(close_input_pin);
  if (close_signal == HIGH) {
    prev_CB_status = CB_status;
    CB_status = HIGH;  
    if (prev_CB_status == LOW) {
      spring_status_switch = LOW;
      spring_charge_start_time = millis();    // start timer
      spring_charge_timer_running = 1;
    }
  }

  if (CB_status == HIGH) {
    if ((spring_status_switch == LOW) && (spring_charge_timer_running)) {
        if ((millis() - spring_charge_start_time) >= 4000) {
          spring_status_switch = HIGH;  // if 4 seconds have passed since CB HIGH, spring finishes charging
          spring_charge_timer_running = 0;
        }
    }
  }
  writeLEDOutputs();
}
