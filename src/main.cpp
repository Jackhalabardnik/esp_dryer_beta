#include <Arduino.h>
#include <string>
#include <sstream>
#include <array>
#include "SSD1306Wire.h"
#include "expander_buttons.h"

constexpr int SDA_pin = 14, SCL_pin = 12; // SDA = D5; SCL = D6
constexpr int screen_refresh_time = 50, key_refresh_time = 5, long_key_press_add_time = 1000;

std::vector<Expander::Button> expander_buttons = {{0},
                                                  {1},
                                                  {2},
                                                  {3}};
bool Expander::activation_level = false;

std::array<int, 4> button_counter;

struct PeriodicExecution {
    unsigned long period;

    void (*function)();

    unsigned long last_execution_time = 0;
};

std::vector<PeriodicExecution> periodic_executions;

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA_pin, SCL_pin);
PCF8574 expander(0x20, SDA_pin, SCL_pin);

void draw_text(const std::string &text) {
    display.clear();
    display.drawString(1, 1, text.c_str());
    display.display();
}

void init_OLED() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

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
    Serial.begin(9600);

    init_OLED();
    init_expander();

    draw_text("Ready");
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
        if (expander_buttons[i].has_changed && expander_buttons[i].is_activated) {
            if (expander_buttons[i].is_long_press) {
                button_counter[i] += 1 + ((expander_buttons[i].long_press_score / 200) * 5);
            }
        }
    }
}

void refresh_screen() {
    std::stringstream ss;
    for (int i = 0; i < 4; i++) {
        ss << "p " << expander_buttons[i].is_activated << " lp " << expander_buttons[i].is_long_press << " count "
           << button_counter[i] << "\n";
    }
    draw_text(ss.str());
}

void loop() {
    periodic_executions.push_back({key_refresh_time, refresh_keys});
    periodic_executions.push_back({screen_refresh_time, refresh_screen});
    periodic_executions.push_back({long_key_press_add_time, increase_when_long_button_is_pressed});

    while (true) {
        std::for_each(periodic_executions.begin(), periodic_executions.end(), [](auto &pe) {
            if (pe.last_execution_time - millis() > pe.period) {
                pe.function();
                pe.last_execution_time = millis();
            }
        });

        EspClass::wdtFeed();
    }
}