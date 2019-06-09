/*
Ports: 
	0 0 .. 13 13
	A0: 14, A1: 15, A2: 16, A3: 17: A4: 18: A5: 19: A6: 20, A7: 21
	port bits: 5 ... 0..32
	0:   0: 00000
	1:	 1:	00001	
	A7: 21: 10101


*/

#include <Arduino.h>
#include <Wire.h>

//#define DEBUG // remove debug so pin 0 and 1 can be used for IO

#define I2C_ADDRESS 8

void onRequest();
void onReceive(int);

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println(F("Init "));
#endif

  analogReference(INTERNAL);

  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(onRequest);
  Wire.onReceive(onReceive);

#ifdef DEBUG
  Serial.println(F("Wire ok"));
#endif
}

void loop()
{
  //int temp = analogRead(A1);
  //Serial.println(temp);
}

volatile byte buffer[3];
volatile byte len = 1;

#define DIGITAL_READ(b, pin, mask) \
  if (digitalRead(pin))            \
    buffer[b] |= mask;

void readDigital()
{
  len = 3;
  buffer[0] = 0;
  DIGITAL_READ(0, 0, 1);
  DIGITAL_READ(0, 1, 2);
  DIGITAL_READ(0, 2, 4);
  DIGITAL_READ(0, 3, 8);
  DIGITAL_READ(0, 4, 16);
  DIGITAL_READ(0, 5, 32);
  DIGITAL_READ(0, 6, 64);
  DIGITAL_READ(0, 7, 128);

  buffer[1] = 0;
  DIGITAL_READ(1, 8, 1);
  DIGITAL_READ(1, 9, 2);
  DIGITAL_READ(1, 10, 4);
  DIGITAL_READ(1, 11, 8);
  DIGITAL_READ(1, 12, 16);
  DIGITAL_READ(1, 13, 32);
  DIGITAL_READ(1, A0, 64);
  DIGITAL_READ(1, A1, 128);

  buffer[2] = 0;
  DIGITAL_READ(2, A2, 1);
  DIGITAL_READ(2, A3, 2);

// I2C
//DIGITAL_READ(2, A4, 4);
//DIGITAL_READ(2, A5, 8);

// DIGITAL READ not supports on A3 .. A7
#ifdef DEBUG_READ
  Serial.print(F("Read 3 bytes: "));
  Serial.print(buffer[0]);
  Serial.print(' ');
  Serial.print(buffer[1]);
  Serial.print(' ');
  Serial.println(buffer[2]);

#endif
}
void readAnalog(int pin)
{
  int val = analogRead(A0 + pin);
  len = 2;
  buffer[0] = val & 0xFF;
  buffer[1] = (val >> 8) & 0b11;
#ifdef DEBUG_READ
  Serial.print(F("Read analog pin "));
  Serial.println(pin);
#endif
}

void onRequest()
{
  Wire.write(const_cast<uint8_t *>(buffer), len);
}

#define CMD_DIGITAL_READ 0x0

#define CMD_WRITE_ANALOG 0x2
#define CMD_WRITE_DIGITAL_HIGH 0x3
#define CMD_WRITE_DIGITAL_LOW 0x4

#define CMD_SETUP_PIN_OUTPUT 0x5
#define CMD_SETUP_PIN_INPUT_PULLUP 0x6
#define CMD_SETUP_PIN_INPUT 0x7

// 8 analog registers.. A0 to A7
// A4 and A5 not supported due to I2C
#define CMD_ANALOG_READ_A0 0b1000 // 0x8
// ....
#define CMD_ANALOG_READ_A7 0b1111 // 0xF

#define CMD_SETUP_ANALOG_INTERNAL 0x10
#define CMD_SETUP_ANALOG_DEFAULT 0x11

void onReceive(int numBytes)
{
#ifdef DEBUG_READ
  Serial.print("Received bytes: ");
  Serial.println(numBytes);
#endif
  int cmd = Wire.read();

  switch (cmd)
  {
  case CMD_DIGITAL_READ:
    readDigital();
    break;
  }

  if (cmd >= CMD_ANALOG_READ_A0 && cmd <= CMD_ANALOG_READ_A7)
  {
    readAnalog(cmd & 0b111);
    return;
  }

  int pin = Wire.read();

  switch (cmd)
  {
  case CMD_WRITE_DIGITAL_HIGH:
  case CMD_WRITE_DIGITAL_LOW:
  {
    bool output = cmd == CMD_WRITE_DIGITAL_HIGH;
    digitalWrite(pin, output);
#ifdef DEBUG
    Serial.print(F("Pin "));
    Serial.print(pin);
    Serial.println(output ? F(" HIGH") : F(" LOW"));
#endif
    break;
  }
  case CMD_WRITE_ANALOG:
  {
    int val = Wire.read() & (Wire.read() << 8);
    analogWrite(pin, val);
#ifdef DEBUG
    Serial.print(F("Pin "));
    Serial.print(pin);
    Serial.print(F(" Analog write "));
    Serial.println(val);
#endif
    break;
  }
  case CMD_SETUP_PIN_OUTPUT:
    pinMode(pin, OUTPUT);
#ifdef DEBUG
    Serial.print(F("Pin "));
    Serial.print(pin);
    Serial.println(F(" OUTPUT"));
#endif
    break;
  case CMD_SETUP_PIN_INPUT:
    pinMode(pin, INPUT);
#ifdef DEBUG
    Serial.print(F("Pin "));
    Serial.print(pin);
    Serial.println(F("INPUT"));
#endif
    break;
  case CMD_SETUP_PIN_INPUT_PULLUP:
    pinMode(pin, INPUT_PULLUP);
#ifdef DEBUG
    Serial.print(F("Pin "));
    Serial.print(pin);
    Serial.println(F("INPUT PULLUP"));
#endif
    break;
  case CMD_SETUP_ANALOG_INTERNAL:
    analogReference(INTERNAL);
#ifdef DEBUG
    Serial.println(F("Analog reference INTERNAL"));
#endif
    break;
  case CMD_SETUP_ANALOG_DEFAULT:
    analogReference(DEFAULT);
#ifdef DEBUG
    Serial.println(F("Analog reference DEFAULT"));
#endif
    break;
  }
}
