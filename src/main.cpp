#include <Arduino.h>
#include <string>
#include <sstream>
#include "SSD1306Wire.h"
#include "PCF8574.h"

constexpr int SDA_pin = 14, SCL_pin = 12; //SDA = D5; SCL = D6
constexpr int screen_refresh_time = 25, key_refresh_time = 12;

volatile struct KeyButtons {
    bool button_1 = true;
    bool button_2 = true;
    bool button_3 = true;
    bool button_4 = true;
} key_buttons;

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

void loop() {

    auto last_expander_refresh_time = millis();
    auto last_screen_refresh_time = millis();

    int counter = 0;

    while (true) {
        if (millis() - last_expander_refresh_time >= key_refresh_time) {
            PCF8574::DigitalInput val = expander.digitalReadAll();

            key_buttons.button_1 = val.p0;
            key_buttons.button_2 = val.p1;
            key_buttons.button_3 = val.p2;
            key_buttons.button_4 = val.p3;

            counter++;

            last_expander_refresh_time = millis();
        }

        if (millis() - last_screen_refresh_time >= screen_refresh_time) {
            std::stringstream ss;
            ss << "Button 1: " << !key_buttons.button_1 << " -> " << counter
               << "\nButton 2: " << !key_buttons.button_2
               << "\nButton 3: " << !key_buttons.button_3
               << "\nButton 4: " << !key_buttons.button_4;
            draw_text(ss.str());
            last_screen_refresh_time = millis();
        }

        EspClass::wdtFeed();
    }
}