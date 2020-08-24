// This is the Code for a CL Speed Timer
// Written in Noember 2012 by Sebastian Kunzke, used a lot of the Example Library
// you need 3 Potis on A0,A1,A2 and Two LED`s on D9 and D10
// also a LED for Main Power and a Switch.
// Controller Pulse gets to D3
// Arduino gets Power from the BEC of the Controller
// Connect Minus to GND and Plus to 5V
// i did build the switch after the Power LED and before the Arduino, that means you "Arm" the plane by plugging in the Akku
// this is a free code and the writer is not responsible for any damage or injury that relates to the usage of this code

#include <Servo.h>

Servo myservo;

int ledState = LOW;
long previousMillis = 0;
long interval = 333;

int pos = 0;  // variable to store the servo position
int zeit = 0; // variable für speedflug

int dauer = 10; // wertungsfluglänge / main flight time in s

int led1 = 10; // statusLEDs
int led2 = 9;

int maxspeed = 170;       //oberer Wert mit dem der regler funktioniert  / upper Controller Value
int zerospeed = 15;       // minimaler Reglerwert zum initialisieren (std. 15)  / lower Controller Value
int plateauspeedsens = 1; // geschwindigkeit vorbereitungsphase poti
int plateauspeed = 0;     // Plateauspeedvariable
int plateautimesens = 2;  //  zeit Vorbereitungsphase poti
int plateautime = 0;      // Plateauzeitvariable
int accsensor = 0;        // beschleunigung poti
int brightness1 = 255;    //grün
int brightness2 = 255;    // blau
int speedup = 0;          // beschleunigungsfaktor

void setup()
{
    myservo.attach(3); // attaches the servo on pin 3 to the servo object

    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
}

void loop() // Hauptprogramm  / main Programm
{
    analogWrite(led1, brightness1);

    for (pos = 0; pos < 100; pos += 1) // Regler initialisieren
    {

        myservo.write(zerospeed);

        unsigned long currentMillis = millis(); // LED blinker
        if (currentMillis - previousMillis > interval)
        {
            // save the last time you blinked the LED
            previousMillis = currentMillis;
            // if the LED is off turn it on and vice-versa:
            if (ledState == LOW)
                ledState = HIGH;
            else
                ledState = LOW;

            // set the LED with the ledState of the variable:
            digitalWrite(led1, ledState);
        }

        delay(50);
    }

    plateauspeed = 1023 - analogRead(plateauspeedsens);
    plateauspeed = map(plateauspeed, 0, 1023, zerospeed, maxspeed);
    for (pos = zerospeed + 10; pos < plateauspeed; pos += 1) // gasweg null bis Plateau
    {
        myservo.write(pos); // regler ansteuern
        digitalWrite(led1, HIGH);
        speedup = analogRead(accsensor) / 5 + 21; // Speedupsensor auslesen
        delay(speedup);
        plateauspeed = 1023 - analogRead(plateauspeedsens);
        plateauspeed = map(plateauspeed, 0, 1023, zerospeed, maxspeed);
    }

    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);

    plateautime = 1023 - analogRead(plateautimesens);

    for (zeit = 0; zeit < plateautime; zeit += 1) // Plateauphase
    {
        myservo.write(plateauspeed);
        plateauspeed = 1023 - analogRead(plateauspeedsens);
        plateauspeed = map(plateauspeed, 0, 1023, zerospeed, maxspeed);
        plateautime = 1023 - analogRead(plateautimesens);
        delay(30);
    }

    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    plateauspeed = 1023 - analogRead(plateauspeedsens);
    plateauspeed = map(plateauspeed, 0, 1023, zerospeed, maxspeed);

    for (pos = plateauspeed; pos < maxspeed; pos += 1) // gasweg bis vollgas
    {
        myservo.write(pos);                       // regler ansteuern
        speedup = analogRead(accsensor) / 5 + 21; // Speedupsensor auslesen
        delay(speedup);
    }

    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    for (zeit = 0; zeit < dauer * 50; zeit += 1) // Wertungsgeschwindigkeit, zeit in s
    {
        myservo.write(maxspeed);

        delay(20);
    }

    analogWrite(led1, brightness1);

    for (pos = 180; pos >= 1; pos -= 1) // slow shut off
    {
        myservo.write(pos);
        analogWrite(led2, brightness2);
        delay(20);
    }
    analogWrite(led2, 0);
    zeit = 0;
    for (zeit = 0; zeit < 1000; zeit -= 1)
    {
        if (zeit == 100)
            zeit = 0;
        else
            zeit = 0;
        digitalWrite(led2, HIGH);
        delay(1000);
        digitalWrite(led2, LOW);
        delay(1000);
    }
}