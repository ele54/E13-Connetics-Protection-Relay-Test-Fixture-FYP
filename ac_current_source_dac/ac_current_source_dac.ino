/* ac_current_source_dac
 *  
 *  Author: Daniel Duan
 *  
 *  DAC version. This version is untested. Also it does not work with sender_test as the software serial
 *  uses the SPI pins, which the DAC needs.
 * 
 */

#define DEBUG 1

#include <MCP_DAC.h>
#include <Wire.h>
#include "MCP4551.h"


const uint16_t sineLookupTable[] = {
2048, 2176, 2304, 2431, 2557, 2680, 2801, 2919,
3034, 3145, 3251, 3353, 3449, 3540, 3625, 3704,
3776, 3842, 3900, 3951, 3995, 4031, 4059, 4079,
4091, 4095, 4091, 4079, 4059, 4031, 3995, 3951,
3900, 3842, 3776, 3704, 3625, 3540, 3449, 3353,
3251, 3145, 3034, 2919, 2801, 2680, 2557, 2431,
2304, 2176, 2048, 1919, 1791, 1664, 1538, 1415,
1294, 1176, 1061, 950, 844, 742, 646, 555,
470, 391, 319, 253, 195, 144, 100, 64,
36, 16, 4, 0, 4, 16, 36, 64,
100, 144, 195, 253, 319, 391, 470, 555,
646, 742, 844, 950, 1061, 1176, 1294, 1415,
1538, 1664, 1791, 1919};

// DAC
MCP4921 MCP;

static int index = 0;
const int sineFreq = 50;      // Sinusoidal frequency (50 Hz)
const int resolution = 100;   // Resolution for sine wave (number of steps per sine wave)
const uint8_t dac_pin = 14; // PC0 = pin 14 = A0


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


  // Setup communication
  Serial.begin(9600);

  // Setup digipot stuff
  pinMode(digipot_a0_pin, OUTPUT);
  digitalWrite(digipot_a0_pin, HIGH);
  Wire.begin();
  Wire.setClock(400000); //fast mode

  myMCP4551.begin();

  // Initialize output disable pin
  pinMode(es_pin, OUTPUT);
  digitalWrite(es_pin, HIGH);

  // Setup DAC
  SPI.begin();
  MCP.begin(dac_pin);
  // fastWrite just writes, skips flags and return values
  MCP.fastWriteA(0);
  
}

void loop() {
  // Calculate time between sine updates
  present_us = micros();


  // sine_us
  if (present_us - last_sine_us >= sine_us) {
    last_sine_us = present_us;

    MCP.fastWriteA(sineLookupTable[index]);
    
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
    if (Serial.available() > 1) {
      uint8_t incoming[2] = {0};
      incoming[0] = Serial.read();
      incoming[1] = Serial.read();
  
      uint16_t dac_code = ((incoming[1] & 0xC0) >> 6) | ((uint16_t)(incoming[0] << 2));
      digipot_position = dac_code >> 2;
      // convert 10 bit (1023) dac code to 8-bit (255) pot code by dividing by 4 which is same as bitshifting left 2 bits
      CB_status = (incoming[1] & 0x20) >> 5;
    }
  }
}
