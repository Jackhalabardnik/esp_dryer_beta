#include "expander_buttons.h"

namespace Expander {
    namespace {
        void updateButton(Button &button, bool pinValue) {
            bool is_pin_active = (pinValue == activation_level);

            if (is_pin_active && button.score < button_max_score) {
                button.score++;
            }
            if (!is_pin_active && button.score > 0) {
                button.score--;
            }

            if (button.is_activated) {
                if (button.score < button_score_threshold) {
                    button.is_activated = false;
                    button.long_press_score = 0;
                    button.is_long_press = false;
                    button.has_changed = true;
                } else if (button.score == button_max_score) {
                    if (button.long_press_score < button_max_long_press_score) {
                        button.long_press_score++;
                    }
                    if (button.long_press_score > button_long_press_threshold_score && !button.is_long_press) {
                        button.is_long_press = true;
                    }
                }
            } else {
                if (button.score == button_max_score) {
                    button.is_activated = true;
                    button.has_changed = true;
                }
            }

        }

        bool get_pin_state(int pin_number, const PCF8574::DigitalInput &values) {
            switch (pin_number) {
                case 0:
                    return values.p0;
                case 1:
                    return values.p1;
                case 2:
                    return values.p2;
                case 3:
                    return values.p3;
                case 4:
                    return values.p4;
                case 5:
                    return values.p5;
                case 6:
                    return values.p6;
                case 7:
                    return values.p7;
                default:
                    return false;
            }
        }
    }


    void updateAll(std::vector<Button> &buttons, PCF8574 &expander) {
        PCF8574::DigitalInput pin_values = expander.digitalReadAll();

        std::for_each(buttons.begin(), buttons.end(), [&pin_values](auto &button) {
            updateButton(button, get_pin_state(button.expander_pin, pin_values));
        });
    }
}

