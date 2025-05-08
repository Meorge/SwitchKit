#ifndef COLORS_H
#define COLORS_H

#include <cstdint>

struct Color24 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct SwitchControllerColors {
    Color24 body_color;
    Color24 button_color;
    Color24 left_grip_color;
    Color24 right_grip_color;
};

#endif // COLORS_H
