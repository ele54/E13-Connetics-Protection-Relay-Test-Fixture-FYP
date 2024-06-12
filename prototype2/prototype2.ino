/*
Prototype 2:

  CB status trips based on signal and manual button inputs.
  
*/
#include <ezAnalogKeypad.h>

// Defines to make code more readable
enum State {OPEN, CLOSED};

// I/O pins
// can use digital pins 2 to 13
#define trip_input 2
#define auxiliary_ref_input 3
#define gas_pressure_ref_input 4
#define earth_switch_ref_input 5
#define supervision_ref_input 6
#define service_position_ref_input 7  

#define auxiliary_52A_output 8
#define auxiliary_52B_output 9
#define gas_pressure_status_output 10
#define earth_switch_status_output 11
#define supervision_status_output 12
#define service_position_status_output 13
// analog pins are A0(14) to A5(19)
ezAnalogKeypad buttonSet1(A0);   // Generic name can be changed

// CB status 
State CB_status = CLOSED;  
// CB internal status switches
State gas_pressure_switch = CLOSED;   // closed for normal, open for low
State earth_switch = CLOSED;  
State supervision_status_switch = CLOSED;   // closed for normal, open for fault
State service_position_switch = CLOSED;   // closed for racked in, open for racked out (not attached to buttons yet)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //input pins
  pinMode(trip_input, INPUT);   
  pinMode(auxiliary_ref_input, INPUT);
  pinMode(gas_pressure_ref_input, INPUT);
  pinMode(earth_switch_ref_input, INPUT);
  pinMode(supervision_ref_input, INPUT);
  pinMode(service_position_ref_input, INPUT);

  //output pins
  pinMode(auxiliary_52A_output, OUTPUT);
  pinMode(auxiliary_52B_output, OUTPUT);
  pinMode(gas_pressure_status_output, OUTPUT);
  pinMode(earth_switch_status_output, OUTPUT);
  pinMode(supervision_status_output, OUTPUT);
  pinMode(service_position_status_output, OUTPUT);

  // Buttons
  // CB_close_button.setDebounceTime(50);
  // CB_open_button.setDebounceTime(50);
  buttonSet1.setNoPressValue(1023);  // analog value when no button is pressed
  // Below values need to be recalibrated for different prototypes
  buttonSet1.registerKey(1, 0); // button for CB manual close
  buttonSet1.registerKey(2, 288); // button for CB manual open
  // buttonSet1.registerKey(3, 563); // button for 
  // buttonSet1.registerKey(4, 688); // button for 
  buttonSet1.registerKey(5, 760); // button for gas pressure normal
  buttonSet1.registerKey(6, 807); // button for gas pressure low
  buttonSet1.registerKey(7, 845); // button for earth switch closed
  buttonSet1.registerKey(8, 882); // button for earth switch open
  buttonSet1.registerKey(9, 910); // button for trip circuit supervision normal
  buttonSet1.registerKey(10, 944); // button for trip circuit supervision fault
}

void loop() {
  // Process buttons
  unsigned char button1 = buttonSet1.getKey();
  switch (button1) {
    case 1:
      CB_status = CLOSED;
      break;
    case 2:
      CB_status = OPEN;
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      gas_pressure_switch = CLOSED;
      break;
    case 6:
      gas_pressure_switch = OPEN;
      break;      
    case 7:
      earth_switch = CLOSED;
      break;
    case 8:
      earth_switch = OPEN;
      break;      
    case 9:
      supervision_status_switch = CLOSED;
      break;
    case 10:
      supervision_status_switch = OPEN;
      break;   
  }

  int trip_signal = digitalRead(trip_input);
  
  if (trip_signal == HIGH) {  
    CB_status = OPEN; // would this conflict with manual control
    digitalWrite(auxiliary_52A_output, OPEN);
    digitalWrite(auxiliary_52B_output, CLOSED);
  }

  // Auxiliary Contact outputs for CB status
  int auxiliary_signal = digitalRead(auxiliary_ref_input);
  switch (CB_status) {
    case OPEN:
      digitalWrite(auxiliary_52A_output, !auxiliary_signal);             // open = output opposite of input signal
      digitalWrite(auxiliary_52B_output, auxiliary_signal); // closed = connect output to the input signal
      break;
    case CLOSED:
      digitalWrite(auxiliary_52A_output, auxiliary_signal);
      digitalWrite(auxiliary_52B_output, !auxiliary_signal);
      break;
  }

  int gas_pressure_ref_signal = digitalRead(gas_pressure_ref_input);
  switch (gas_pressure_switch) {
    case CLOSED:
      digitalWrite(gas_pressure_status_output, gas_pressure_ref_signal);
      break;
    case OPEN:
      digitalWrite(gas_pressure_status_output, !gas_pressure_ref_signal);
  }

  int earth_switch_ref_signal = digitalRead(earth_switch_ref_input);
  switch (earth_switch) {
    case CLOSED:
      digitalWrite(earth_switch_status_output, earth_switch_ref_signal);
      break;
    case OPEN:
      digitalWrite(earth_switch_status_output, !earth_switch_ref_signal);
  }

  int supervision_ref_signal = digitalRead(supervision_ref_input);
  switch (supervision_status_switch) {
    case CLOSED:
      digitalWrite(supervision_status_output, supervision_ref_signal);
      break;
    case OPEN:
      digitalWrite(supervision_status_output, !supervision_ref_signal);
  }

  int service_position_ref_signal = digitalRead(service_position_ref_input);
  switch (service_position_switch) {
    case CLOSED:
      digitalWrite(service_position_status_output, service_position_ref_input);
      break;
    case OPEN:
      digitalWrite(service_position_status_output, !service_position_ref_input);
  }

}
