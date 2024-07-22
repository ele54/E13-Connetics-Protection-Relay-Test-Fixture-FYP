
//define where your pins are
#define LED_data_pin 9  //pin 14 DS
#define LED_latch_pin 10 //pin 12 ST_CP
#define LED_clock_pin 11  //pin 11 SH_CP
#define numOfLEDRegisters 2
#define numOfLEDRegisterPins numOfLEDRegisters * 8
boolean LEDregisters[numOfLEDRegisterPins];

enum State {OPEN, CLOSED};
State CB_status = CLOSED;  
State prev_CB_status = CLOSED;  
State prev_spring_status = CLOSED;  
State gas_pressure_switch = CLOSED;   // closed for gas low, open for gas normal
State earth_switch = CLOSED;  // closed for earthed, open for not earthed
State circuit_supervision_status_switch = CLOSED;   // closed for normal, open for fault
State service_position_switch = CLOSED;   // closed for racked in, open for racked out 
State spring_status_switch = CLOSED;   // closed for charged, open for discharged
State generic_status_switch1 = CLOSED;
State generic_status_switch2 = CLOSED;
State generic_status_switch3 = CLOSED;
State generic_status_switch4 = CLOSED;

#define CB_status_LED 10
#define gas_pressure_status_LED 11
#define earth_switch_status_LED 12
#define generic_status_LED1 13
#define generic_status_LED2 14
#define service_position_status_LED 6
#define spring_charge_status_LED 4
#define circuit_supervision_status_LED 3
#define generic_status_LED3 2
#define generic_status_LED4 1

void writeLEDOutputs() {
  LEDregisters[CB_status_LED] = CB_status;
  LEDregisters[gas_pressure_status_LED] = gas_pressure_switch;
  LEDregisters[earth_switch_status_LED] = !earth_switch;
  LEDregisters[generic_status_LED1] = generic_status_switch1;
  LEDregisters[generic_status_LED2] = generic_status_switch2;
  LEDregisters[service_position_status_LED] = service_position_switch;
  LEDregisters[spring_charge_status_LED] = spring_status_switch;
  LEDregisters[circuit_supervision_status_LED] = circuit_supervision_status_switch;
  LEDregisters[generic_status_LED3] = generic_status_switch3;
  LEDregisters[generic_status_LED4] = generic_status_switch4;
  Serial.println("led register");
  for(int i = 0; i < 16; i++)
  {
    Serial.println(LEDregisters[i]);
  }
  writeRegisters(LED_latch_pin, LED_clock_pin, LED_data_pin);
  delay(1000);
}

//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfLEDRegisterPins - 1; i >=  0; i--){
     LEDregisters[i] = LOW;
  }
} 

//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(int latch_pin, int clock_pin, int data_pin) {

  digitalWrite(latch_pin, LOW);

  for(int i = numOfLEDRegisterPins - 1; i >=  0; i--){
    digitalWrite(clock_pin, LOW);

    int val = LEDregisters[i];

    digitalWrite(data_pin, val);
    digitalWrite(clock_pin, HIGH);

  }
  digitalWrite(latch_pin, HIGH);

}

void setup() {
  //start serial
  Serial.begin(9600);
  //define pin modes
  pinMode(LED_latch_pin, OUTPUT);
  pinMode(LED_clock_pin, OUTPUT);
  pinMode(LED_data_pin, OUTPUT);
}

void loop() {
  for(int i = numOfLEDRegisterPins - 1; i >=  0; i--){
     LEDregisters[i] = LOW;
  }
  writeRegisters(LED_latch_pin, LED_clock_pin, LED_data_pin);
  Serial.println("low");
  delay(1000);
  for(int i = numOfLEDRegisterPins - 1; i >=  0; i--){
     LEDregisters[i] = HIGH;
  }
  LEDregisters[15] = HIGH;
  writeRegisters(LED_latch_pin, LED_clock_pin, LED_data_pin);
  Serial.println("High");
  delay(1000);
}
