#include "esphome.h"
#include <FastLED.h>

void color_wheel(esphome::light::LightState *light)
{
    static uint8_t hue;
    hue++;

    static int state = 0;
    auto call = light->turn_on();
    // Transtion of 1000ms = 1s
    call.set_transition_length(0);

    const CRGB rgb = CHSV(hue, 255, 255);

    call.set_rgb(rgb.r / 255.0, rgb.g / 255.0, rgb.b / 255.0);

    call.perform();
}