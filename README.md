# esphome_devices

![CI](https://github.com/glmnet/esphome_devices/workflows/CI/badge.svg)

This is my house full of ESPHome devices... I really don't know how many are there already, but around 15.

There are 4 outside, two in the living room, 1 in the kitchen, upstairs there is 1 on the hall and 1 on each room + 1 for every AC unit.

Many of them are connected to Arduinos via I2C and the Arduino Port Expander (https://esphome.io/cookbook/arduino_port_extender.html) thinghy, those controls relays boards (the common one which comes with 4 relays)

Everything is DIY hardware too, no sonoff here.

I do have to admit I have a few H801 RGBWW Led controllers, which uses the ESP8266 inside.

I not only use ESPHome but I also develop and contribute for ESPHome for more than a year already, helping Otto as much as I can. So you will find here many test yaml programs which I write to test code I write or maintain.

I've created a few components
* DF Player
* SIM800L Sms Sender
* TM1637 display
* Coolix climate (contributed a bit on the climate stuff)
* AC Dimmer, this one started by Otto but really put some effort there, seems to be working

I also maintain the VSCode plugin

Hope this helps!
