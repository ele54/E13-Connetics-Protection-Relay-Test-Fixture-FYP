/* ac_current_source_spwm
 * 
 * This code accepts a 10-bit ADC code and uses it to configure the output current.
 * 
 * It uses a cooperative timer-scheduled schedule and goes through the following tasks:
 * - Communication tasks - Reads serial communication
 * - Digipot task - Adjusts digial potentioemter to adjust output current
 * - Output disable task - Uses the CB_status read from serial to determine if the current should stop
 * - SPWM task - Varies the output PWM sinusoidally so it can be filtered into a smooth sinusoidal voltage
 * 
 * Currently, this code is setup to work with sender_test, where sender_test allows another Arduino to send ADC codes
 * based on the Serial monitor input on the Arduino IDE.
 * 
 */

const int pwmPin = 9;         // PWM output pin
const int pwmFreq = 20000;    // PWM frequency (20 kHz)
const int sineFreq = 50;      // Sinusoidal frequency (50 Hz)
const int resolution = 100;   // Resolution for sine wave (number of steps per sine wave)
#define DEBUG 1

#include <Wire.h>
#include "MCP4551.h"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(11, 12);



static int index = 0;
const uint16_t sineLookupTable[] = {
50, 53, 56, 59, 62, 65, 68, 71,
74, 77, 79, 82, 84, 86, 89, 90,
92, 94, 95, 96, 98, 98, 99, 100,
100, 100, 100, 100, 99, 98, 98, 96,
95, 94, 92, 90, 89, 86, 84, 82,
79, 77, 74, 71, 68, 65, 62, 59,
56, 53, 50, 47, 44, 41, 38, 35,
32, 29, 26, 23, 21, 18, 16, 14,
11, 10, 8, 6, 5, 4, 2, 2,
1, 0, 0, 0, 0, 0, 1, 2,
2, 4, 5, 6, 8, 10, 11, 14,
16, 18, 21, 23, 26, 29, 32, 35,
38, 41, 44, 47};


// Scheduling
 unsigned long present_us = 0;

unsigned long last_sine_us = 0;
unsigned const long sine_us = 1e6 / (sineFreq * resolution);

unsigned long last_digipot_us = 0;
unsigned const long digipot_us = 700000;

unsigned long last_comms_us = 0;
unsigned const long comms_us = 70000;

unsigned long last_es_us = 0;
unsigned const long es_us = 1000;

// Digipot
const uint8_t digipot_a0_pin = 16; // PC2 = pin 16 = A2
uint8_t digipot_position = 192;
MCP4551 myMCP4551(MCP4551_ADDRESS_A0_VCC);

// Output Disable
const uint8_t es_pin = 17; // PC3 = pin 17 = A3
uint8_t CB_status = 0;

void setup() {
  // Set the PWM pin as output
  pinMode(pwmPin, OUTPUT);
  TCCR1A = 0x00;   //reset
  TCCR1B = 0x00;   //TC1 reset and OFF
  //fOC1A = clckSys/(N*(1+ICR1)); Mode-14 FPWM; OCR1A controls duty cycle
  // 20 kHz = 16000000/(8*(1+ICR1)) ==> ICR1 = 99, N = 8

  TCCR1A |= (1 << COM1A1) | (0 << COM1A0);  //Ch-A non-inverting, Mode-14
  TCCR1A |= (1 << COM1B1) | (1 << COM1B0); //Ch-B inverting
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12); //Mode-14 Fast PWM
  ICR1 = 99;  //changes frequency as ICR changes
  OCR1A = 50;   //~= 50% duty cycle
  OCR1B = 50;
  TCCR1B |= (1 << CS11); //N = 8

  // Setup communication
  Serial.begin(9600);
  mySerial.begin(4800);

  // Setup digipot stuff
  pinMode(digipot_a0_pin, OUTPUT);
  digitalWrite(digipot_a0_pin, HIGH);
  Wire.begin();
  Wire.setClock(400000); //fast mode

  myMCP4551.begin();

  // Initialize output disable pin
  pinMode(es_pin, OUTPUT);
  digitalWrite(es_pin, HIGH);
  
}

void loop() {
  // Calculate time between sine updates
  present_us = micros();


  // sine_us
  if (present_us - last_sine_us >= sine_us) {
    last_sine_us = present_us;

    // Update PWM duty cycle with next value in sine table
    OCR1A = sineLookupTable[index];   //~= 50% duty cycle
    OCR1B = sineLookupTable[index];

    // Increment the index for sine table, wrap around if needed
    index = (index + 1) % resolution;
  }

  
  // Digipot Task
  if ((present_us - last_digipot_us) > digipot_us) {
    last_digipot_us = present_us;
    myMCP4551.setWiper(digipot_position);
  }

  // Output disable task
  if ((present_us - last_es_us) > es_us) {
    last_es_us = present_us;
    digitalWrite(es_pin, CB_status);
  }
  

  // Communication Task
  
  if ((present_us - last_comms_us) > comms_us) {
    last_comms_us = present_us;
    if (mySerial.available() > 1) {
      uint8_t incoming[2] = {0};
      incoming[0] = mySerial.read();
      incoming[1] = mySerial.read();
  
      uint16_t dac_code = ((incoming[1] & 0xC0) >> 6) | ((uint16_t)(incoming[0] << 2));
      digipot_position = dac_code >> 2;
      // convert 10 bit (1023) dac code to 8-bit (255) pot code by dividing by 4 which is same as bitshifting left 2 bits
      CB_status = (incoming[1] & 0x20) >> 5;
  
      #ifdef DEBUG
      Serial.print("Incoming0:");
      Serial.print(incoming[0]);
      Serial.print("\n");
      Serial.print("Incoming1:");
      Serial.print(incoming[1]);
      Serial.print("\n");
  
      Serial.print("DAC Code:");
      Serial.print(dac_code);
      Serial.print("\n");
      Serial.print("CB Status:");
      Serial.print(CB_status);
      Serial.print("\n");

      Serial.print("digipot:");
      Serial.print(digipot_position);
      Serial.print("\n");
      #endif
    }
  }
}
