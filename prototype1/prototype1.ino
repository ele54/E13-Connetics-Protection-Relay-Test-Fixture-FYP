/*
Prototype 1:

  CB status trips based on signal.
  CB recloses based on auto reclose signal.
*/

// I/O pins
// can use digital pins 2 to 19
#define trip_input 2
#define CB_output 3
#define spring_charged_contact_input 4
#define spring_charged_contact_output 5
// #define auxiliary_1_output 6
// #define auxiliary_2_output 6
// #define CB_control_sw 6
// auxiliary outputs

// status variables
bool CB_status = 1;  //0 for open/tripped (1 for closed/untripped)
bool spring_charged_switch = 1; //1 for charged (switch closed), 0 for not charged (switch open). !!NEED SOMETHING TO TOGGLE THIS
// auxiliary contacts

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
}

void loop() {
  int trip_signal = digitalRead(trip_input);
  
  if (trip_signal == HIGH) {
    CB_status = 0;
  }

  if (spring_charged_switch == 1) {
    int spring_charge_status = digitalRead(spring_charged_contact_input);
    digitalWrite(spring_charged_contact_output, spring_charge_status);
  } else {
    digitalWrite(spring_charged_contact_output, 0);   // pulls spring-charged switch low if it is open
  }

  digitalWrite(CB_output, CB_status);
}
