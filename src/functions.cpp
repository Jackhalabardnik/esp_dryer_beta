#include "functions.h"

namespace {
    // structures
    enum class BoxState {
        STANDBY = 0,
        HEATING = 1
    };

    enum MenuButton {
        UP = 0,
        DOWN = 1,
        ENTER = 2,
        ESCAPE = 3
    };

    // static stuff
    constexpr int SDA_pin = 14, SCL_pin = 12;// SDA = D5; SCL = D6
    constexpr int temperature_buffer = 5;
    constexpr int max_buffer_size = 128;

    const char *status_standby_text = "STANDBY", *status_heating_test = "HEATING";
    const char *menu_text = "H: %.1f T: %.1f H_T: %.1f\nST: %d MNH: %d MXH: %d\nT: %d ETA: %d EL: %d\n HT: %d MF: %d HD: %d";

    // main logic variables
    BoxState box_state = BoxState::STANDBY;
    double box_temperature = 0, box_humidity = 0, heating_pad_temperature = 0;
    int set_temperature = 40, min_humidity = 70, max_humidity = 90, set_time = 60, current_time = 0;
    bool is_heating_pad_on = false, is_humid_deleter_on = false, is_mixing_fan_on = false;

    // misc variables
    bool is_preheat_on = false;
    char display_text[max_buffer_size];

    SHTSensor sht30;
    SSD1306Wire display(0x3c, SDA_pin, SCL_pin);
    PCF8574 expander(0x20, SDA_pin, SCL_pin);

    std::vector<Expander::Button> expander_buttons = {{0},
                                                      {1},
                                                      {2},
                                                      {3}};

    // functions
    void init_expander() {
        for (int i = 0; i < 4; i++) {
            expander.pinMode(i, OUTPUT, HIGH);
        }

        for (int i = 4; i < 8; i++) {
            expander.pinMode(i, OUTPUT, LOW);
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

    void turn_heating_pad_on() {
        expander.digitalWrite(4, 1);
        is_heating_pad_on = true;
    }

    void turn_heating_pad_off() {
        expander.digitalWrite(4, 0);
        is_heating_pad_on = false;
    }

    void turn_mixing_fan_on() {
        expander.digitalWrite(5, 0);
        is_mixing_fan_on = true;
    }

    void turn_mixing_fan_off() {
        expander.digitalWrite(5, 0);
        is_mixing_fan_on = false;
    }

    void turn_humid_deleter_on() {
        expander.digitalWrite(6, 0);
        is_humid_deleter_on = true;
    }

    void turn_humid_deleter_off() {
        expander.digitalWrite(6, 0);
        is_humid_deleter_on = false;
    }

    void set_state_to_standby() {
        box_state = BoxState::STANDBY;
        is_preheat_on = false;
        turn_heating_pad_off();
        turn_mixing_fan_off();
        turn_humid_deleter_off();
    }

    void set_state_to_heating() {
        box_state = BoxState::HEATING;
        is_preheat_on = true;
        turn_heating_pad_on();
        turn_mixing_fan_on();
    }

    void refresh_menu(MenuButton menuButton) {
        switch (menuButton) {
            case UP:
                set_state_to_standby();
                break;
            case DOWN:
                set_state_to_heating();
                break;
            case ENTER:
                box_state == BoxState::STANDBY ? set_state_to_heating() : set_state_to_standby();
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
        box_temperature = sht30.getTemperature();
        box_humidity = sht30.getHumidity();
    } else {
        box_temperature = sht30.getTemperature() + 100;
        box_humidity = sht30.getHumidity();
    }
}

void refresh_screen() {
    std::snprintf(display_text, max_buffer_size, menu_text,
                  box_humidity, box_temperature, heating_pad_temperature,
                  set_temperature, min_humidity, max_humidity,
                  set_time, set_time - current_time, current_time - set_time,
                  is_heating_pad_on, is_mixing_fan_on, is_humid_deleter_on);

    display.clear();

    display.drawString(1, 1, box_state == BoxState::STANDBY ? status_standby_text : status_heating_test);

    display.drawHorizontalLine(1, 12, 32);

    display.drawString(1, 14, display_text);

    display.display();
}

void do_logic() {
    if (box_state == BoxState::HEATING) {
        if (is_preheat_on) {
            if (heating_pad_temperature >= set_temperature) {
                turn_heating_pad_off();
                is_preheat_on = false;
            }
        } else {
            if (is_heating_pad_on && heating_pad_temperature >= set_temperature) {
                turn_heating_pad_off();
            } else if (!is_heating_pad_on && heating_pad_temperature < set_temperature - temperature_buffer) {
                turn_heating_pad_on();
            }
        }

        if (is_humid_deleter_on && box_humidity < min_humidity) {
            turn_humid_deleter_off();
        } else if (!is_humid_deleter_on && box_humidity > max_humidity) {
            turn_humid_deleter_on();
        }
    }
}

void safe_box() {
    if (box_state == BoxState::STANDBY) {
        turn_heating_pad_off();
        turn_humid_deleter_off();
        turn_mixing_fan_off();
    }
}
