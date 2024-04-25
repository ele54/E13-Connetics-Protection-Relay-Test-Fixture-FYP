/*
Prototype 1:

  CB status trips based on signal
  CB recloses based on auto reclose signal
*/

#define trip_input 2
#define CB_output 3
#define auto_reclose_signal 4

bool CB_status = 1;  //0 for open/tripped (1 for closed/untripped)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //input pins
  pinMode(trip_input, INPUT);   
  pinMode(auto_reclose_input, INPUT);   

  //output pins
  pinMode(CB_output, OUTPUT);
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
}
