#include <SHTSensor.h>

#include "expander_buttons.h"
#include "periodic_execution.h"
#include "oled.h"

#include <array>
#include <sstream>

constexpr int SDA_pin = 14, SCL_pin = 12;// SDA = D5; SCL = D6
constexpr int screen_refresh_time = 50, key_refresh_time = 5, long_key_press_add_time = 200, temperature_refresh_time = 500;

std::array<int, 4> button_counter;
double SHT3X_temperature = 0, SHT3X_humidity = 0;

SHTSensor sht30;
SSD1306Wire display(0x3c, SDA_pin, SCL_pin);
PCF8574 expander(0x20, SDA_pin, SCL_pin);

std::vector<Expander::Button> expander_buttons = {{0},
                                                  {1},
                                                  {2},
                                                  {3}};

std::vector<PeriodicExecution::Routine> routines;

void init_expander() {
    for (int i = 0; i < 4; i++) {
        expander.pinMode(i, OUTPUT, HIGH);
    }

    delay(5);

    for (int i = 0; i < 4; i++) {
        expander.pinMode(i, INPUT);
    }

    expander.begin();
}

void setup() {
    Wire.begin(SDA_pin,SCL_pin);
    Serial.begin(9600);

    OLED::init_OLED(display);
    init_expander();

    sht30.init();
}

void refresh_keys() {
    Expander::updateAll(expander_buttons, expander);

    for (int i = 0; i < 4; i++) {
        if (expander_buttons[i].has_changed && expander_buttons[i].is_activated) {
            button_counter[i]++;
            expander_buttons[i].has_changed = false;
        }
    }
}

void increase_when_long_button_is_pressed() {
    for (int i = 0; i < 4; i++) {
        if (expander_buttons[i].is_long_press) {
            button_counter[i] += 1 + (expander_buttons[i].long_press_score / 200 * 3);
        }
    }
}

void refresh_temperature() {
    auto result = sht30.readSample();
    if(result) {
        SHT3X_temperature = sht30.getTemperature();
        SHT3X_humidity = sht30.getHumidity();
    } else {
        SHT3X_temperature = sht30.getTemperature() + 100;
        SHT3X_humidity = sht30.getHumidity();
    }
}

void refresh_screen() {
    std::stringstream ss;
    for (int i = 0; i < 4; i++) {
        ss << button_counter[i] << " ";
    }

    ss.precision(2);
    ss << std::fixed << "\nSHT3X T: " << SHT3X_temperature << " H: "<< SHT3X_humidity;

    OLED::draw_text(display, ss.str());
}

void loop() {
    routines.push_back({key_refresh_time, refresh_keys});
    routines.push_back({temperature_refresh_time, refresh_temperature});
    routines.push_back({long_key_press_add_time, increase_when_long_button_is_pressed});
    routines.push_back({screen_refresh_time, refresh_screen});

    while (true) {
        PeriodicExecution::updateExecutions(routines);

        EspClass::wdtFeed();
    }
}