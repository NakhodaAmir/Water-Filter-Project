#include "wokwi-api.h"
#include <stddef.h>

static pin_t vplus;
static pin_t gnd;
static timer_t timer;

static void timer_callback(void *user_data) {
  pin_write(vplus, HIGH);
  pin_write(gnd, LOW);
}

void chip_init() {
  vplus = pin_init("V+", OUTPUT);
  gnd   = pin_init("GND", OUTPUT);

  const timer_config_t config = {
    .callback = timer_callback,
    .user_data = NULL,
  };

  timer = timer_init(&config);
  timer_start(timer, 100000, true);
}