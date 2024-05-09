/*
Prototype 1:

  CB status trips based on signal.
  Spring-charge contact outputs based on whether if switch is closed.
*/
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
#define CB_open_button 8
#define CB_close_button 9

// status variables
State CB_status = CLOSED;  //0 for open/tripped (1 for closed/untripped) !! need to add states failed and unknown (would be manually toggled)
State spring_charged_switch = CLOSED; //1 for charged (switch closed), 0 for not charged (switch open). !!NEED SOMETHING TO TOGGLE THIS
// auxiliary contacts
State auxiliary_52A_status = OPEN;
State auxiliary_52B_status = CLOSED;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //input pins
  pinMode(trip_input, INPUT);   
  pinMode(spring_charged_contact_input, INPUT);   
  pinMode(CB_open_button, INPUT);   
  pinMode(CB_close_button, INPUT);   

  //output pins
  pinMode(CB_output, OUTPUT);
  pinMode(spring_charged_contact_output, OUTPUT);
  pinMode(auxiliary_52A_output, OUTPUT);
  pinMode(auxiliary_52B_output, OUTPUT);
}

void loop() {
  int trip_signal = digitalRead(trip_input);
  
  if (trip_signal == HIGH) {
    CB_status = OPEN;
    digitalWrite(auxiliary_52A_output, OPEN);
    digitalWrite(auxiliary_52B_output, CLOSED);
  }

  int manual_CB_open = digitalRead(CB_open_button);
  int manual_CB_close = digitalRead(CB_close_button);

  if (manual_CB_open == HIGH) {
    CB_status = OPEN;
  }
  if (manual_CB_close == HIGH) {
    CB_status = CLOSED;
  }

  if (spring_charged_switch == CLOSED) {
    int spring_charge_status = digitalRead(spring_charged_contact_input);
    digitalWrite(spring_charged_contact_output, spring_charge_status);
  } else {
    digitalWrite(spring_charged_contact_output, 0);   // pulls spring-charged switch low if it is open
  }

  // Auxiliary Contact outputs for CB status
  switch (CB_status) {
    case OPEN:
      digitalWrite(auxiliary_52A_output, OPEN);
      digitalWrite(auxiliary_52B_output, CLOSED);
      break;
    case CLOSED:
      digitalWrite(auxiliary_52A_output, CLOSED);
      digitalWrite(auxiliary_52B_output, OPEN);
      break;
    case FAILURE:
      digitalWrite(auxiliary_52A_output, CLOSED);
      digitalWrite(auxiliary_52B_output, CLOSED);
      break;
    case UNKNOWN:
      digitalWrite(auxiliary_52A_output, OPEN);
      digitalWrite(auxiliary_52B_output, OPEN);
      break;
  }
  
}
