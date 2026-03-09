#include "wokwi-api.h"
#include <stdio.h>

static pin_t ao_pin;
static uint32_t ph_attr;

void chip_init() {
  // Initialize analog output pin
  ao_pin = pin_init("AO", ANALOG);

  // Initialize attribute with default value (integer)
  ph_attr = attr_init("ph", 7);
}

void chip_loop() {
  // Read attribute (returns int)
  uint32_t ph_value = attr_read(ph_attr);

  float ph = (float)ph_value;

  // Convert pH to voltage
  float voltage = 2.5f + (7.0f - ph) * 0.059f;

  pin_dac_write(ao_pin, voltage);
}