/*
Prototype 1:

  CB status trips based on signal and manual button inputs.
  Spring-charge contact outputs based on whether if switch is closed.
*/
#include <ezAnalogKeypad.h>

// Defines to make code more readable
enum State {OPEN, CLOSED, FAILURE, UNKNOWN};

// I/O pins
// can use digital pins 2 to 9, 14 to 19
#define trip_input 2
#define CB_output 3
#define spring_charged_contact_input 4
#define spring_charged_contact_output 5
#define auxiliary_52A_output 6
#define auxiliary_52B_output 7
#define auxiliary_input 8

ezAnalogKeypad buttonSet1(A0);   // Generic name can be changed

// CB status variables
State CB_status = CLOSED;  
State spring_charged_switch = CLOSED; //1 for charged (switch closed), 0 for not charged (switch open). !!NEED SOMETHING TO TOGGLE THIS

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //input pins
  pinMode(trip_input, INPUT);   
  pinMode(spring_charged_contact_input, INPUT);   
  pinMode(auxiliary_input, INPUT);

  //output pins
  pinMode(CB_output, OUTPUT);
  pinMode(spring_charged_contact_output, OUTPUT);
  pinMode(auxiliary_52A_output, OUTPUT);
  pinMode(auxiliary_52B_output, OUTPUT);

  // Buttons
  // CB_close_button.setDebounceTime(50);
  // CB_open_button.setDebounceTime(50);
  buttonSet1.setNoPressValue(1023);  // analog value when no button is pressed
  // Below values need to be recalibrated for different prototypes
  buttonSet1.registerKey(1, 0); // button for CB manual close
  buttonSet1.registerKey(2, 288); // button for CB manual open
  buttonSet1.registerKey(3, 563); // button for CB failure status
  buttonSet1.registerKey(4, 688); // button for CB unknown status
  // buttonSet1.registerKey(5, 760); // button for 
  // buttonSet1.registerKey(6, 807); // button for 
  // buttonSet1.registerKey(7, 845); // button for 
  // buttonSet1.registerKey(8, 882); // button for 
  // buttonSet1.registerKey(9, 910); // button for 
  // buttonSet1.registerKey(10, 944); // button for 
}

void loop() {
  // Process buttons
  unsigned char button1 = buttonSet1.getKey();
  if (button1) {
    Serial.print("The button ");
    Serial.print((int)button1);
    Serial.println(" is pressed");
  }
  switch (button1) {
    case 1:
      CB_status = CLOSED;
      break;
    case 2:
      CB_status = OPEN;
      break;
    case 3:
      CB_status = FAILURE;
      break;
    case 4:
      CB_status = UNKNOWN;
      break;
  }

  int trip_signal = digitalRead(trip_input);
  
  if (trip_signal == HIGH) {  
    CB_status = OPEN; // would this conflict with manual control
    digitalWrite(auxiliary_52A_output, OPEN);
    digitalWrite(auxiliary_52B_output, CLOSED);
  }

  if (spring_charged_switch == CLOSED) {
    int spring_charge_signal = digitalRead(spring_charged_contact_input);
    digitalWrite(spring_charged_contact_output, spring_charge_signal);
  } else {
    digitalWrite(spring_charged_contact_output, 0);   // pulls spring-charged switch low if it is open
  }

  // Auxiliary Contact outputs for CB status
  int auxiliary_signal = digitalRead(auxiliary_input);
  switch (CB_status) {
    case OPEN:
      digitalWrite(auxiliary_52A_output, !auxiliary_signal);             // open = output opposite of input signal
      digitalWrite(auxiliary_52B_output, auxiliary_signal); // closed = connect output to the input signal
      break;
    case CLOSED:
      digitalWrite(auxiliary_52A_output, auxiliary_signal);
      digitalWrite(auxiliary_52B_output, !auxiliary_signal);
      break;
    case FAILURE:
      digitalWrite(auxiliary_52A_output, auxiliary_signal);
      digitalWrite(auxiliary_52B_output, auxiliary_signal);
      break;
    case UNKNOWN:
      digitalWrite(auxiliary_52A_output, !auxiliary_signal);
      digitalWrite(auxiliary_52B_output, !auxiliary_signal);
      break;
  }

}
