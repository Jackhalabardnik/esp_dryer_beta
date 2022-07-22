#include <Arduino.h>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include "SSD1306Wire.h"
#include "PCF8574.h"

constexpr int SDA_pin = 14, SCL_pin = 12; // SDA = D5; SCL = D6
constexpr int screen_refresh_time = 50, key_refresh_time = 5;
constexpr int button_max_score = 20, button_max_long_press_score = 100, button_score_treshold = button_max_score / 2;

struct Button
{
    bool is_activated = false;
    int score = 0;
    int long_press_score = 0;
};

volatile std::array<Button, 4> buttons;

struct PeriodicExecution {
    unsigned long last_execution_time = 0;
    void (*function)();
};

std::vector<PeriodicExecution> periodic_executions;

volatile struct KeyButtons
{
    int button_1 = 0;
    int button_2 = 0;
    int button_3 = 0;
    int button_4 = 0;
} key_buttons;

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA_pin, SCL_pin);
PCF8574 expander(0x20, SDA_pin, SCL_pin);

void draw_text(const std::string &text)
{
    display.clear();
    display.drawString(1, 1, text.c_str());
    display.display();
}

void init_OLED()
{
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void init_expander()
{
    for (int i = 0; i < 4; i++)
    {
        expander.pinMode(i, OUTPUT, HIGH);
    }

    delay(5);

    for (int i = 0; i < 4; i++)
    {
        expander.pinMode(i, INPUT);
    }

    expander.begin();
}

void setup()
{
    Serial.begin(9600);

    init_OLED();
    init_expander();

    draw_text("Ready");
}

void refresh_keys() {

}

void update_buttons() {

}

void refresh_screen() {

}


void loop()
{
    periodic_executions.push_back({key_refresh_time, refresh_keys});

    auto last_expander_refresh_time = millis();
    auto last_screen_refresh_time = millis();

    int counter = 0;

    while (true)
    {
        if (millis() - last_expander_refresh_time >= key_refresh_time)
        {
            PCF8574::DigitalInput val = expander.digitalReadAll();

            if (val.p0 && key_buttons.button_1 > 0)
            {
                key_buttons.button_1--;
            }
            else if (!val.p0 && key_buttons.button_1 < button_score_treshold)
            {
                key_buttons.button_1++;
            }

            if (val.p1 && key_buttons.button_2 > 0)
            {
                key_buttons.button_2--;
            }
            else if (!val.p1 && key_buttons.button_2 < button_score_treshold)
            {
                key_buttons.button_2++;
            }

            if (val.p2 && key_buttons.button_3 > 0)
            {
                key_buttons.button_3--;
            }
            else if (!val.p2 && key_buttons.button_3 < button_score_treshold)
            {
                key_buttons.button_3++;
            }

            if (val.p3 && key_buttons.button_4 > 0)
            {
                key_buttons.button_4--;
            }
            else if (!val.p3 && key_buttons.button_4 < button_score_treshold)
            {
                key_buttons.button_4++;
            }

            counter++;

            last_expander_refresh_time = millis();
        }

        if (millis() - last_screen_refresh_time >= screen_refresh_time)
        {
            std::stringstream ss;
            ss << "Button 1: " << key_buttons.button_1 << " = " << (key_buttons.button_1 > button_score_treshold)
               << "\nButton 2: " << key_buttons.button_2 << " = " << (key_buttons.button_2 > button_score_treshold)
               << "\nButton 3: " << key_buttons.button_3 << " = " << (key_buttons.button_3 > button_score_treshold)
               << "\nButton 4: " << key_buttons.button_4 << " = " << (key_buttons.button_4 > button_score_treshold);
            draw_text(ss.str());
            last_screen_refresh_time = millis();
        }

        EspClass::wdtFeed();
    }
}