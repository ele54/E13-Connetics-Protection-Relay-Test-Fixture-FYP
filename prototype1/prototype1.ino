/*
Prototype 1:

  CB status trips based on signal.
  Spring-charge contact outputs based on whether if switch is closed.
*/
// Defines to make code more readable
#define open 0
#define closed 1

// I/O pins
// can use digital pins 2 to 19
#define trip_input 2
#define CB_output 3
#define spring_charged_contact_input 4
#define spring_charged_contact_output 5
#define auxiliary_52A_output 6
#define auxiliary_52B_output 6
// #define CB_control_sw 6
// auxiliary outputs

// status variables
int CB_status = closed;  //0 for open/tripped (1 for closed/untripped) !! need to add states failed and unknown (would be manually toggled)
bool spring_charged_switch = closed; //1 for charged (switch closed), 0 for not charged (switch open). !!NEED SOMETHING TO TOGGLE THIS
// auxiliary contacts
bool auxiliary_52A_status = open;
bool auxiliary_52B_status = closed;

// struct auxiliary {
//   int output_pin;
//   bool status;
// }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //input pins
  pinMode(trip_input, INPUT);   
  pinMode(spring_charged_contact_input, INPUT);   
  // pinMode(CB_control_sw, INPUT);   

  //output pins
  pinMode(CB_output, OUTPUT);
  pinMode(spring_charged_contact_output, OUTPUT);
  pinMode(auxiliary_52A_output, OUTPUT);
  pinMode(auxiliary_52B_output, OUTPUT);
}

void loop() {
  // if manual control switch is on
  // read input switches
  // spring-charge_switch = read(spring_charge_manual_switch)

  int trip_signal = digitalRead(trip_input);
  
  if (trip_signal == HIGH) {
    CB_status = open;
  }

  if (spring_charged_switch == 1) {
    int spring_charge_status = digitalRead(spring_charged_contact_input);
    digitalWrite(spring_charged_contact_output, spring_charge_status);
  } else {
    digitalWrite(spring_charged_contact_output, 0);   // pulls spring-charged switch low if it is open
  }

  // Auxiliary Contact outputs
  if (CB_status == open) {
    auxiliary_52A_status = open;
    auxiliary_52B_status = closed;
  }

  digitalWrite(CB_output, CB_status);    // don't need this if circuit breaker status is only read through auxiliary contact position
}
