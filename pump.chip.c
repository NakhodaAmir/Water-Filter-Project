#include "wokwi-api.h"
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

static pin_t vcc;
static pin_t gnd;
static timer_t timer;
static bool was_running = false;

// This function runs every 100ms
static void timer_callback(void *user_data) {
    bool vcc_state = pin_read(vcc);
    bool gnd_state = pin_read(gnd);

    // Pump runs only if VCC HIGH and GND LOW
    bool running = (vcc_state == HIGH) && (gnd_state == LOW);

    if (running && !was_running) {
        printf("Pump started!\n");
    } 
    else if (!running && was_running) {
        printf("Pump stopped!\n");
    }

    was_running = running;
}

void chip_init() {
    // Initialize pins
    vcc = pin_init("VCC", INPUT);
    gnd = pin_init("GND", INPUT);

    // Configure timer
    const timer_config_t config = {
        .callback = timer_callback,
        .user_data = NULL
    };

    // Start timer, repeats every 100ms
    timer = timer_init(&config);
    timer_start(timer, 100000, true);

    printf("Pump initialized\n");
}