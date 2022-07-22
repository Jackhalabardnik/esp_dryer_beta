#pragma once
#include <vector>
#include "PCF8574.h"

namespace Expander {
    struct Button {
        int expander_pin;
        int score = 0;
        int long_press_score = 0;
        bool is_activated = false;
        bool is_long_press = false;
    };

    void updateAll(std::vector<Button> &buttons, PCF8574 &expander);

    //activation level: false - button is pressed when logic 0, true - button is pressed when logic 1
    extern bool activation_level;

    constexpr int button_max_score = 10, button_max_long_press_score = 50, button_score_threshold = button_max_score / 2;
}
