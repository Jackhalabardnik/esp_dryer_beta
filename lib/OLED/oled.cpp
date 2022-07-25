#include "oled.h"

namespace OLED {
    void init_OLED(SSD1306Wire & display) {
        display.init();
        display.flipScreenVertically();
        display.setFont(ArialMT_Plain_10);
    }

    void draw_text(SSD1306Wire & display, const std::string &text) {
        display.clear();
        display.drawString(1, 1, text.c_str());
        display.display();
    }
}