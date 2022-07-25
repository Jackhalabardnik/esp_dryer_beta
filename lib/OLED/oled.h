#pragma once
#include <string>
#include "SSD1306Wire.h"

namespace OLED {
    void init_OLED(SSD1306Wire & display);

    void draw_text(SSD1306Wire & display, const std::string &text);
}