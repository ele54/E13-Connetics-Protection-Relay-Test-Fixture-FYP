/*
Prototype 1:

  CB status trips based on signal
*/

#define trip_signal 2
#define CB_output 3

bool CB_status = 1;  //0 for open/tripped (1 for closed/untripped)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //input pins
  pinMode(trip_signal, INPUT);   

  //output pins
  pinMode(CB_output, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (trip_signal == HIGH) {
    CB_status = 0;
  }

  digitalWrite(CB_output, CB_status);
}
