#include <Arduino.h>
#include <string>
#include <sstream>
#include "SSD1306Wire.h"
#include "PCF8574.h"

constexpr int SDA_pin = 13, SCL_pin = 14, interrupt_pin = 12;
constexpr int debounce_delay = 200, screen_refresh_time = 50, key_refresh_time = 25;

volatile int int_counter = 0;
volatile bool is_expander_interrupt = false;

volatile struct KeyButtons {
    bool button_1 = true;
    bool button_2 = true;
    bool button_3 = true;
    bool button_4 = true;
} key_buttons;

IRAM_ATTR void expander_interrupt_func() {
    int_counter++;
    is_expander_interrupt = true;
}

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA_pin, SCL_pin);
PCF8574 expander(0x20, SDA_pin, SCL_pin, interrupt_pin, expander_interrupt_func);

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
        expander.pinMode(i, INPUT, HIGH);
    }
    expander.begin();
}

void setup() {
    Serial.begin(9600);

    init_OLED();
    init_expander();

    draw_text("Ready");
}

void loop() {

    auto last_expander_interrupt_time = millis();
    auto last_screen_refresh_time = millis();

    while (true) {
        if (is_expander_interrupt && millis() - last_expander_interrupt_time >= debounce_delay) {
            PCF8574::DigitalInput val = expander.digitalReadAll();

            key_buttons.button_1 = val.p0;
            key_buttons.button_2 = val.p1;
            key_buttons.button_3 = val.p2;
            key_buttons.button_4 = val.p3;
            last_expander_interrupt_time = millis();
            is_expander_interrupt = false;
        }

        if (millis() - last_screen_refresh_time >= refresh_time) {
            std::stringstream ss;
            ss << "Button 1: " << !key_buttons.button_1 << " ints = " << int_counter
               << "\nButton 2: " << !key_buttons.button_2
               << "\nButton 3: " << !key_buttons.button_3
               << "\nButton 4: " << !key_buttons.button_4;
            draw_text(ss.str());
            last_screen_refresh_time = millis();
        }

        EspClass::wdtFeed();
    }
}