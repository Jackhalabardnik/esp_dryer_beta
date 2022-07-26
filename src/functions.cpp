#include "functions.h"

namespace {
    // static stuff
    constexpr int SDA_pin = 14, SCL_pin = 12;// SDA = D5; SCL = D6
    constexpr int max_buffer_size = 128;

    const char *status_standby_text = "STANDBY", *status_heating_test = "HEATING";
    const char *menu_text = "Hum:%.2f, Temp=%.2f";

    // dynamic stuff
    double SHT3X_temperature = 0, SHT3X_humidity = 0;
    char display_text[max_buffer_size];

    SHTSensor sht30;
    SSD1306Wire display(0x3c, SDA_pin, SCL_pin);
    PCF8574 expander(0x20, SDA_pin, SCL_pin);

    std::vector<Expander::Button> expander_buttons = {{0},
                                                      {1},
                                                      {2},
                                                      {3}};

    // structures
    enum class BoxState {
        STANDBY = 0,
        HEATING = 1
    } boxState = BoxState::STANDBY;

    enum MenuButton {
        UP = 0,
        DOWN = 1,
        ENTER = 2,
        ESCAPE = 3
    };

    // functions
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

    void init_OLED() {
        display.init();
        display.flipScreenVertically();
        display.setFont(ArialMT_Plain_10);
    }

    void refresh_menu(MenuButton menuButton) {
        switch (menuButton) {
            case UP:
                boxState = BoxState::STANDBY;
                break;
            case DOWN:
                boxState = BoxState::HEATING;
                break;
            case ENTER:
                boxState = boxState == BoxState::STANDBY ? BoxState::HEATING : BoxState::STANDBY;
                break;
            case ESCAPE:
                break;
        }
    }

}// namespace

void init_drivers() {
    Wire.begin(SDA_pin, SCL_pin);
    init_expander();
    init_OLED();
    sht30.init();
}

void refresh_keys() {
    Expander::updateAll(expander_buttons, expander);

    for (int i = 0; i < 4; i++) {
        if (expander_buttons[i].has_changed && expander_buttons[i].is_activated) {
            refresh_menu(MenuButton(i));
            expander_buttons[i].has_changed = false;
        }
    }
}

void refresh_temperature() {
    auto result = sht30.readSample();
    if (result) {
        SHT3X_temperature = sht30.getTemperature();
        SHT3X_humidity = sht30.getHumidity();
    } else {
        SHT3X_temperature = sht30.getTemperature() + 100;
        SHT3X_humidity = sht30.getHumidity();
    }
}

void refresh_screen() {
    std::snprintf(display_text, max_buffer_size, menu_text, SHT3X_humidity, SHT3X_temperature);

    display.clear();

    display.drawString(1, 1, boxState == BoxState::STANDBY ? status_standby_text : status_heating_test);

    display.drawHorizontalLine(1, 12, 32);

    display.drawString(1, 14, display_text);

    display.display();
}

void drive_box() {
    if (boxState == BoxState::STANDBY) {

    } else {
    }
}

void safe_box() {
    if (boxState == BoxState::STANDBY) {
    }
}
