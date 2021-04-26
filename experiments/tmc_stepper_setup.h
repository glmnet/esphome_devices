
#include <TMCStepper.h>

#include "esphome.h"

class TMCStepperComponent : public Component, public UARTDevice
{
public:
    TMCStepperComponent(UARTComponent *parent) : UARTDevice(parent) {}

    void setup() override
    {
        TMC2209Stepper stepper_driver(this, 0.15f, 0b00);
        stepper_driver.pdn_disable(true);
        stepper_driver.begin();
        stepper_driver.microsteps(64);
        stepper_driver.TCOOLTHRS(910);
        stepper_driver.rms_current(600);
        stepper_driver.SGTHRS(20);

        stepper_driver.en_spreadCycle(false);

        //DRIVER SETUP

        //stepper_driver.TPWMTHRS(0);
        /*stepper_driver.semin(0);
        stepper_driver.semax(2);
        stepper_driver.sedn(0b00);
        stepper_driver.toff(4);
        stepper_driver.blank_time(24);

        */
    }
    void loop() override
    {
    }
};