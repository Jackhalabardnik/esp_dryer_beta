#include "functions.h"
#include "periodic_execution.h"

std::vector<PeriodicExecution::Routine> routines;
constexpr int screen_refresh_time = 50,
              key_refresh_time = 5,
              temperature_refresh_time = 100,
              do_logic_time = 10,
              safe_box_time = 1000;


void setup() {
    Serial.begin(9600);

    init_drivers();

    routines.push_back({key_refresh_time, refresh_keys});
    routines.push_back({temperature_refresh_time, refresh_temperature});
    routines.push_back({screen_refresh_time, refresh_screen});
    routines.push_back({do_logic_time, do_logic});
    routines.push_back({safe_box_time, safe_box});
}

void loop() {
    while(true) {
        PeriodicExecution::updateExecutions(routines);
        EspClass::wdtFeed();
    }
}