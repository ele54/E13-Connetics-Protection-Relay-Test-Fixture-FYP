/*
Prototype 2:

  CB status trips based on signal and manual button inputs.
  shift register code taken from https://www.instructables.com/3-Arduino-pins-to-24-output-pins/
  
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

// define shift register pins and variables
#define data_pin 11  //pin 14 DS
#define latch_pin 12 //pin 12 SHCP
#define clock_pin 13  //pin 11 SH_CP
#define number_of_74hc595s 2
#define numOfRegisterPins number_of_74hc595s * 8
boolean registers[numOfRegisterPins];

// analog pins are A0(14) to A5(19)
ezAnalogKeypad buttonSet1(A0);   // Generic name can be changed

// CB status 
State CB_status = CLOSED;  
// CB internal status switches - need to be checked against schematic
State gas_pressure_switch = CLOSED;   // closed for normal, open for low
State earth_switch = CLOSED;  
State supervision_status_switch = CLOSED;   // closed for normal, open for fault
State service_position_switch = CLOSED;   // closed for racked in, open for racked out 
  
// Define output signals
int auxiliary_52A_output;
int auxiliary_52B_output;
int service_position_status_output;
int gas_pressure_status_output;
int earth_switch_status_output;
int supervision_status_output;

// Set status output value based on reference input, switch position
void setStatusOutput(int ref_input, State switch_position, int* output_addr) {
  int ref_signal = digitalRead(ref_input);
  switch (switch_position) {
    case CLOSED:
      *output_addr = ref_signal;
      break;
    case OPEN:
      *output_addr = !ref_signal;
      break;
  }
}


//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers[i] = LOW;
  }
  writeRegisters();
} 

//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(){

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

  //input pins
  pinMode(trip_input, INPUT);   
  pinMode(auxiliary_ref_input, INPUT);
  pinMode(gas_pressure_ref_input, INPUT);
  pinMode(earth_switch_ref_input, INPUT);
  pinMode(supervision_ref_input, INPUT);
  pinMode(service_position_ref_input, INPUT);

  // Shift register
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  //reset all register pins
  clearRegisters();
  writeRegisters();

  // Buttons
  buttonSet1.setNoPressValue(1023);  // analog value when no button is pressed
  // Below values need to be recalibrated for different prototypes
  buttonSet1.registerKey(1, 0); // button for CB manual close
  buttonSet1.registerKey(2, 288); // button for CB manual open
  buttonSet1.registerKey(3, 563); // button for racked in
  buttonSet1.registerKey(4, 688); // button for racked out
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
      service_position_switch = CLOSED;
      break;
    case 4:
      service_position_switch = OPEN;
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
  int auxiliary_signal = digitalRead(auxiliary_ref_input);
  if (trip_signal == HIGH) {  
    CB_status = OPEN; // would this conflict with manual control
    auxiliary_52A_output = !auxiliary_signal;
    auxiliary_52B_output = auxiliary_signal;
  }

  switch (CB_status) {
    case OPEN:
      auxiliary_52A_output = !auxiliary_signal; // open = output opposite of input signal
      auxiliary_52B_output = auxiliary_signal; // closed = connect output to the input signal
      break;
    case CLOSED:
      auxiliary_52A_output = auxiliary_signal;
      auxiliary_52B_output = !auxiliary_signal;
      break;
  }

  setStatusOutput(gas_pressure_ref_input, gas_pressure_switch, &gas_pressure_status_output);
  setStatusOutput(earth_switch_ref_input, earth_switch, &earth_switch_status_output);
  setStatusOutput(supervision_ref_input, supervision_status_switch, &supervision_status_output);
  setStatusOutput(service_position_ref_input, service_position_switch, &service_position_status_output);

  // outputs into shift register
  setRegisterPin(0, LOW);
  setRegisterPin(1, LOW);
  setRegisterPin(2, supervision_status_output);
  setRegisterPin(3, earth_switch_status_output);
  setRegisterPin(4, gas_pressure_status_output);
  setRegisterPin(5, service_position_status_output);
  setRegisterPin(6, auxiliary_52B_output);
  setRegisterPin(7, auxiliary_52A_output);
  setRegisterPin(8, LOW);
  setRegisterPin(9, LOW);
  setRegisterPin(10, LOW);
  writeRegisters();
  
}
