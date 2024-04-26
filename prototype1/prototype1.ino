/*
Prototype 1:

  CB status trips based on signal.
  CB recloses based on auto reclose signal.
*/

// I/O pins
// can use digital pins 2 to 19
#define trip_input 2
#define CB_output 3
#define auto_reclose_signal 4
#define spring_charged_state_output 5
// #define CB_control_sw 6
// auxiliary outputs

// status variables
bool CB_status = 1;  //0 for open/tripped (1 for closed/untripped)
bool spring_charged_state = 1; //1 for charged, 0 for not charged. !!NEED SOMETHING TO TOGGLE THIS
// auxiliary contacts

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //input pins
  pinMode(trip_input, INPUT);   
  pinMode(auto_reclose_input, INPUT);   
  // pinMode(CB_control_sw, INPUT);   

  //output pins
  pinMode(CB_output, OUTPUT);
  pinMode(spring_charged_state_output, OUTPUT);
}

void loop() {
  trip_signal = digitalRead(trip_input);
  auto_reclose_signal = digitalRead(auto_reclose_input);
  
  if (trip_signal == HIGH) {
    CB_status = 0;
  }

  if (auto_reclose_signal == HIGH) {
    CB_status = 1;
  }

  digitalWrite(CB_output, CB_status);
  digitalWrite(spring_charged_state, spring_charged_state_output);
}
