#define latchPin 4
#define clockPin 5
#define dataPin 3

void setup()
{
  Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
  digitalWrite(latchPin, LOW);
  digitalWrite(clockPin, LOW);
}

uint16_t read_switches(void)
{
  uint16_t data;

  /* first clock pulse gets parallel load */
  digitalWrite(latchPin, HIGH);

  data=0;
  for(int i=0;i<16;i++)
  {
    /* clock low-high-low */
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);

    /* if this is the first bit, then we're done with the parallel load */
    if (i==0) digitalWrite(latchPin, LOW);

    /* shift the new bit in */
    data<<=1;
    if (digitalRead(dataPin)) data|=1;
  }
  return(data);
}

void loop() {
  Serial.println("1st byte is...");
  uint16_t data = read_switches();
  Serial.println(data, BIN);
  delay(400);
}