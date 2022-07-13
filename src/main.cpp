#include <Arduino.h>
#include <string>
#include "SSD1306Wire.h"
#include "PCF8574.h"

const int SDA_pin = 13;
const int SCL_pin = 14;
const int interrupt_pin = 12;

volatile int int_counter = 0;
volatile bool expander_interrupt = false;

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA_pin, SCL_pin);
PCF8574 expander(0x20, SDA_pin, SCL_pin);

void draw_center_text(const std::string &text) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 22, text.c_str());
    display.display();
}

void init_OLED() {
    display.init();

    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

IRAM_ATTR void keyPressedOnPCF8574() {
    expander_interrupt = true;
    int_counter++;
}

void init_expander() {
    pinMode(interrupt_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interrupt_pin), keyPressedOnPCF8574, FALLING);

    for (int i = 0; i < 4; i++) {
        expander.pinMode(i, INPUT, HIGH);
    }

    if (expander.begin()) {
        draw_center_text("Expander init complete");
    } else {
        draw_center_text("Expander init error");
    }
}

void setup() {
    Serial.begin(9600);

    init_OLED();
    init_expander();
}

void loop() {
    while (true) {
        if(expander_interrupt) {
            PCF8574::DigitalInput val = expander.digitalReadAll();
            auto text = std::to_string(val.p0) + std::to_string(val.p1) + std::to_string(val.p2) + std::to_string(val.p3);
            text += " : ";
            text += std::to_string(int_counter);
            text += " times";
            draw_center_text(text);
            expander_interrupt = false;
            delay(100);
        }
        ESP.wdtFeed();
    }
}