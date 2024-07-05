//**************************************************************//
//  Name    : shiftIn Example 2.1                               //
//  Author  : Carlyn Maw                                        //
//  Date    : 25 Jan, 2007                                      //
//  Version : 1.0                                               //
//  Notes   : Code for using a CD4021B Shift Register       //
//          :                                                   //
//****************************************************************

//define where your pins are
int latchPin = 3;
int dataPin = 5;
int clockPin = 4;

//Define variables to hold the data
//for each shift register.
//starting with non-zero numbers can help
//troubleshoot
byte switchVar1 = 72;  //01001000
byte switchVar2 = 159; //10011111

void setup() {
  //start serial
  Serial.begin(9600);
  //define pin modes
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
}

void loop() {
  //Pulse the latch pin:
  //set it to 1 to collect parallel data
  digitalWrite(latchPin,1);
  //set it to 1 to collect parallel data, wait
  delay(800);
  //set it to 0 to transmit data serially
  digitalWrite(latchPin,0);
  //while the shift register is in serial mode
  //collect each shift register into a byte
  //the register attached to the chip comes in first
  switchVar1 = shiftIn(dataPin, clockPin);
  switchVar2 = shiftIn(dataPin, clockPin);
  //Print out the results.
  //leading 0's at the top of the byte
  //(7, 6, 5, etc) will be dropped before
  //the first pin that has a high input
  //reading
  Serial.println("switchVar1: ");
  Serial.println(switchVar1, BIN);
  Serial.println("switchVar2: ");
  Serial.println(switchVar2, BIN);
  //white space
  Serial.println("-------------------");
  //delay so all these print satements can keep up.
  // int pin2 = digitalRead(2);
  // Serial.println(pin2, BIN);
  delay(500);
}

//------------------------------------------------end main loop
////// ----------------------------------------shiftIn function
///// just needs the location of the data pin and the clock pin
///// it returns a byte with each bit in the byte corresponding
///// to a pin on the shift register. leftBit 7 = Pin 7 / Bit 0= Pin 0

byte shiftIn(int myDataPin, int myClockPin) {
  int i = 0;
  int temp = 0;
  int pinState;
  byte myDataIn = 0;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, INPUT);
//we will be holding the clock pin high 8 times (0,..,7) at the
//end of each time through the for loop
//at the begining of each loop when we set the clock low, it will
//be doing the necessary low to high drop to cause the shift
//register's DataPin to change state based on the value
//of the next bit in its serial information flow.
//The register transmits the information about the pins from pin 7 to pin 0
//so that is why our function counts down
  for (i=7; i>=0; i--)
  {
    digitalWrite(myClockPin, 0);
    delay(800);
    temp = digitalRead(myDataPin);
    if (temp) {
      pinState = 1;
      //set the bit to 0 no matter what
      myDataIn = myDataIn | (1 << i);
    }
    else {
      //turn it off -- only necessary for debuging
     //print statement since myDataIn starts as 0
      pinState = 0;
    }

    //Debuging print statements
    Serial.print("pinState: ");
    Serial.print(pinState);
    Serial.print("     pin: ");
    Serial.println(myDataPin);
    digitalWrite(myClockPin, 1);
  }
  //debuging print statements whitespace
  Serial.println("shift in data: ");
  Serial.println(myDataIn, BIN);
  return myDataIn;
}
