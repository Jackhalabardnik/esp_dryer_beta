#pragma once

#include <SHTSensor.h>
#include <SSD1306Wire.h>

#include "expander_buttons.h"

#include <array>
#include <string>

void init_drivers();

void refresh_keys();

void refresh_long_press();

void refresh_temperature();

void refresh_screen();

void drive_box();

void safe_box();