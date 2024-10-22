/* sender_test
 *  
 * This code tests that the AC Current Source can receieve an ADC code to change its current output
 * Also sends CB_ctatus0 to check that the AC current source can stop
 * 
 * Connections:
 * - Pin 11 (MOSI) on this board to MISO on the AC Current Source
 * - Pin 12 (MISO) on this board to MOSI on the AC current source
 * - Connect ground pin on Arduino to ground in on the AC current source
 * - MISO, MOSI and ground pins can be accessed from the ICSP header, please see test setup diagram for more information
 * 
 * How to use:
 * - Open serial monitor and enter a value between 0-1023. 
 * - The program will then repeat your value and the bytes it sent to the AC current source
 */



#include <SoftwareSerial.h>

SoftwareSerial mySerial(11,12);

uint16_t pot_value = 512;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  delay(100);
  mySerial.begin(4800);
}

void loop() {
  // put your main code here, to run repeatedly:

    if(Serial.available() > 0) {
      String inputString = Serial.readStringUntil('\n'); // Read until newline
      uint16_t pot_value = inputString.toInt(); // Convert the string to an integer
      // From Eshan's Code. Sends 10-bit ADC code MSB first
      uint8_t msg_AC[2];
      uint8_t CB_status0 = 0;
      msg_AC[0] = pot_value >> 2;
      msg_AC[1] = (pot_value << 6) | (CB_status0 << 5);
      // End of Eshan's code
      
      Serial.print("Pot Value:");
      Serial.println(pot_value);
      Serial.print("msg0:");
      Serial.println(msg_AC[0]);
      Serial.print("msg1:");
      Serial.println(msg_AC[1]);
      delay(100);
      
      mySerial.write(msg_AC, 2);
    }
    

}
