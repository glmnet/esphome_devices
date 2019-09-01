#include "esphome.h"

void zero_cross() {
digitalWrite(14, LOW);
}

class TSADimmer : public Component {
 public:
  void setup() override {
    attachInterrupt(4, &zero_cross, RISING);
    pinMode(14, OUTPUT); //nodemcu D5 PWM
    //pinMode(4, INPUT); //nodemcu D2 ZC
  }

  void loop() override {
    digitalWrite(14, HIGH);
    delay(1);
    }

};