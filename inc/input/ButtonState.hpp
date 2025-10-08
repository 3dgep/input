#pragma once

#include <cstdint>

namespace input
{
enum class ButtonState : uint8_t
{
    Up       = 0,  // Button is up
    Held     = 1,  // Button is held down
    Released = 2,  // Button was just released
    Pressed  = 3,  // Buton was just pressed
};
}