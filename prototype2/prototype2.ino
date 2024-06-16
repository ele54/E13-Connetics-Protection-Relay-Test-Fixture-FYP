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

// define shift register pins
#define data_pin 8
#define latch_pin 9
#define clock_pin 10
// #define auxiliary_52A_output 8
// #define auxiliary_52B_output 9
// #define gas_pressure_status_output 10
// #define earth_switch_status_output 11
// #define supervision_status_output 12
// #define service_position_status_output 13

// analog pins are A0(14) to A5(19)
ezAnalogKeypad buttonSet1(A0);   // Generic name can be changed

// CB status 
State CB_status = CLOSED;  
// CB internal status switches
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

byte out_data = 0;

void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // Function taken from Arduino Docs Tutorial 
  // "Serial to Parallel Shifting-Out with a 74HC595"
  // (Carlyn Maw & Tom Igoe, 26/01/2022)

  int i=0;
  int pinState;

  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {
      pinState= 0;
    }
    digitalWrite(myDataPin, pinState);
    digitalWrite(myClockPin, 1);
    digitalWrite(myDataPin, 0);
  }

  digitalWrite(myClockPin, 0);
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

  //output pins
  // pinMode(auxiliary_52A_output, OUTPUT);
  // pinMode(auxiliary_52B_output, OUTPUT);
  // pinMode(gas_pressure_status_output, OUTPUT);
  // pinMode(earth_switch_status_output, OUTPUT);
  // pinMode(supervision_status_output, OUTPUT);
  // pinMode(service_position_status_output, OUTPUT);

  // Shift register
  pinMode(latch_pin, OUTPUT);

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
      auxiliary_52A_output = !auxiliary_signal;             // open = output opposite of input signal
      auxiliary_52B_output = auxiliary_signal; // closed = connect output to the input signal
      break;
    case CLOSED:
      auxiliary_52A_output = auxiliary_signal;
      auxiliary_52B_output = !auxiliary_signal;
      break;
  }

  int gas_pressure_ref_signal = digitalRead(gas_pressure_ref_input);
  switch (gas_pressure_switch) {
    case CLOSED:
      gas_pressure_status_output = gas_pressure_ref_signal;
      break;
    case OPEN:
      gas_pressure_status_output = !gas_pressure_ref_signal;
  }

  int earth_switch_ref_signal = digitalRead(earth_switch_ref_input);
  switch (earth_switch) {
    case CLOSED:
      earth_switch_status_output = earth_switch_ref_signal;
      break;
    case OPEN:
      earth_switch_status_output = !earth_switch_ref_signal;
  }

  int supervision_ref_signal = digitalRead(supervision_ref_input);
  switch (supervision_status_switch) {
    case CLOSED:
      supervision_status_output = supervision_ref_signal;
      break;
    case OPEN:
      supervision_status_output = !supervision_ref_signal;
  }

  int service_position_ref_signal = digitalRead(service_position_ref_input);
  switch (service_position_switch) {
    case CLOSED:
      service_position_status_output = service_position_ref_input;
      break;
    case OPEN:
      service_position_status_output = !service_position_ref_input;
  }

  // outputs into byte for shift register
  bitWrite(out_data, 7, auxiliary_52A_output);
  bitWrite(out_data, 6, auxiliary_52B_output);
  bitWrite(out_data, 5, service_position_status_output);
  bitWrite(out_data, 4, gas_pressure_status_output);
  bitWrite(out_data, 3, earth_switch_status_output);
  bitWrite(out_data, 2, supervision_status_output);
  bitWrite(out_data, 1, 0);

  digitalWrite(latch_pin, 0);
  shiftOut(data_pin, clock_pin, out_data);
  digitalWrite(latch_pin, 1);
  
}
