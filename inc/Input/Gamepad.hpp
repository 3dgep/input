#pragma once

#include "ButtonState.hpp"

namespace input
{
// Source (September 15, 2025): https://github.com/microsoft/DirectXTK/blob/main/Inc/GamePad.h
struct Gamepad
{
    static constexpr int MAX_PLAYER_COUNT   = 8;   ///< Maximum number of connected gamepads.
    static constexpr int MOST_RECENT_PLAYER = -1;  ///< Use this player ID to get the most recently plugged-in gamepad.

    /// <summary>
    /// Dead-zone mode.
    /// </summary>
    enum class DeadZone
    {
        IndependentAxis = 0,
        Circular,
        None,
    };

    struct Buttons
    {
        union
        {
            bool a;
            bool cross;
        };
        union
        {
            bool b;
            bool circle;
        };

        union
        {
            bool x;
            bool square;
        };

        union
        {
            bool y;
            bool triangle;
        };

        bool leftStick;
        bool rightStick;
        bool leftShoulder;
        bool rightShoulder;

        union
        {
            bool back;
            bool view;
        };

        union
        {
            bool start;
            bool menu;
        };
    };

    struct DPad
    {
        bool up;
        bool down;
        bool right;
        bool left;
    };

    struct ThumbSticks
    {
        float leftX;
        float leftY;
        float rightX;
        float rightY;
    };

    struct Triggers
    {
        float left;
        float right;
    };

    struct State
    {
        bool        connected;
        uint64_t    packet;
        Buttons     buttons;
        DPad        dPad;
        ThumbSticks thumbSticks;
        Triggers    triggers;

        bool isConnected() const noexcept
        {
            return connected;
        }

        bool isAPressed() const noexcept
        {
            return buttons.a;
        }

        bool isBPressed() const noexcept
        {
            return buttons.b;
        }

        bool isXPressed() const noexcept
        {
            return buttons.x;
        }

        bool isYPressed() const noexcept
        {
            return buttons.y;
        }

        bool isLeftStickPressed() const noexcept
        {
            return buttons.leftStick;
        }

        bool isRightStickPressed() const noexcept
        {
            return buttons.rightStick;
        }

        bool isLeftShoulderPressed() const noexcept
        {
            return buttons.leftShoulder;
        }

        bool isRightShoulderPressed() const noexcept
        {
            return buttons.rightShoulder;
        }

        bool isBackPressed() const noexcept
        {
            return buttons.back;
        }

        bool isViewPressed() const noexcept
        {
            return buttons.view;
        }

        bool isStartPressed() const noexcept
        {
            return buttons.start;
        }

        bool isMenuPressed() const noexcept
        {
            return buttons.menu;
        }

        bool isDPadDownPressed() const noexcept
        {
            return dPad.down;
        }

        bool isDPadUpPressed() const noexcept
        {
            return dPad.up;
        }

        bool isDPadLeftPressed() const noexcept
        {
            return dPad.left;
        }

        bool isDPadRightPressed() const noexcept
        {
            return dPad.right;
        }

        bool isLeftThumbStickUp() const noexcept
        {
            return thumbSticks.leftY > 0.5f;
        }

        bool isLeftThumbStickDown() const noexcept
        {
            return thumbSticks.leftY < -0.5f;
        }

        bool isLeftThumbStickLeft() const noexcept
        {
            return thumbSticks.leftX < -0.5f;
        }

        bool isLeftThumbStickRight() const noexcept
        {
            return thumbSticks.leftX > 0.5f;
        }

        bool isRightThumbStickUp() const noexcept
        {
            return thumbSticks.rightY > 0.5f;
        }

        bool isRightThumbStickDown() const noexcept
        {
            return thumbSticks.rightY < -0.5f;
        }

        bool isRightThumbStickLeft() const noexcept
        {
            return thumbSticks.rightX < -0.5f;
        }

        bool isRightThumbStickRight() const noexcept
        {
            return thumbSticks.rightX > 0.5f;
        }

        bool isLeftTriggerPressed() const noexcept
        {
            return triggers.left > 0.5f;
        }

        bool isRightTriggerPressed() const noexcept
        {
            return triggers.right > 0.5f;
        }

        bool operator==( const State& ) const noexcept;
        bool operator!=( const State& ) const noexcept = default;
    };

    Gamepad() = default;
    Gamepad( int playerIndex );

    State getState( DeadZone deadZoneMode = DeadZone::IndependentAxis ) const;
    bool  setVibration( float leftMotor, float rightMotor, float leftTrigger = 0.0f, float rightTrigger = 0.0f );

    static State getState( int playerIndex, DeadZone deadZoneMode = DeadZone::IndependentAxis );
    static bool  setVibration( int playerIndex, float leftMotor, float rightMotor, float leftTrigger = 0.0f, float rightTrigger = 0.0f );

    static void suspend() noexcept;
    static void resume() noexcept;

private:
    int playerIndex = MOST_RECENT_PLAYER;
};

class GamepadStateTracker
{
public:
    union
    {
        ButtonState a;
        ButtonState cross;
    };
    union
    {
        ButtonState b;
        ButtonState circle;
    };
    union
    {
        ButtonState x;
        ButtonState square;
    };

    union
    {
        ButtonState y;
        ButtonState triangle;
    };

    ButtonState leftStick;
    ButtonState rightStick;

    ButtonState leftShoulder;
    ButtonState rightShoulder;

    union
    {
        ButtonState back;
        ButtonState view;
    };

    union
    {
        ButtonState start;
        ButtonState menu;
    };

    ButtonState dPadUp;
    ButtonState dPadDown;
    ButtonState dPadLeft;
    ButtonState dPadRight;

    ButtonState leftStickUp;
    ButtonState leftStickDown;
    ButtonState leftStickLeft;
    ButtonState leftStickRight;

    ButtonState rightStickUp;
    ButtonState rightStickDown;
    ButtonState rightStickLeft;
    ButtonState rightStickRight;

    ButtonState leftTrigger;
    ButtonState rightTrigger;

    GamepadStateTracker() noexcept
    {
        reset();
    }

    void update( const Gamepad::State& state );

    void reset() noexcept;

    Gamepad::State getLastState() const noexcept
    {
        return lastState;
    }

private:
    Gamepad::State lastState;
};

float ApplyLinearDeadZone( float value, float maxValue, float deadZoneSize ) noexcept;                                                                      ///< Defined in Gamepad.cpp
void  ApplyStickDeadZone( float x, float y, Gamepad::DeadZone deadZoneMode, float maxValue, float deadZoneSize, float& resultX, float& resultY ) noexcept;  ///< Defined in Gamepad.cpp

}  // namespace input

inline bool operator==( const input::Gamepad::Buttons& lhs, const input::Gamepad::Buttons& rhs )
{
    return lhs.a == rhs.a &&
           lhs.b == rhs.b &&
           lhs.x == rhs.x &&
           lhs.y == rhs.y &&
           lhs.leftStick == rhs.leftStick &&
           lhs.rightStick == rhs.rightStick &&
           lhs.leftShoulder == rhs.leftShoulder &&
           lhs.rightShoulder == rhs.rightShoulder &&
           lhs.back == rhs.back &&
           lhs.start == rhs.start;
}

inline bool operator!=( const input::Gamepad::Buttons& lhs, const input::Gamepad::Buttons& rhs )
{
    return !( lhs == rhs );
}

inline bool operator==( const input::Gamepad::DPad& lhs, const input::Gamepad::DPad& rhs )
{
    return lhs.left == rhs.left &&
           lhs.right == rhs.right &&
           lhs.up == rhs.up &&
           lhs.down == rhs.down;
}

inline bool operator!=( const input::Gamepad::DPad& lhs, const input::Gamepad::DPad& rhs )
{
    return !( lhs == rhs );
}

inline bool operator==( const input::Gamepad::ThumbSticks& lhs,
                        const input::Gamepad::ThumbSticks& rhs )
{
    return lhs.leftX == rhs.leftX &&
           lhs.leftY == rhs.leftY &&
           lhs.rightX == rhs.rightX &&
           lhs.rightY == rhs.rightY;
}

inline bool operator!=( const input::Gamepad::ThumbSticks& lhs,
                        const input::Gamepad::ThumbSticks& rhs )
{
    return !( lhs == rhs );
}

inline bool operator==( const input::Gamepad::Triggers& lhs,
                        const input::Gamepad::Triggers& rhs )
{
    return lhs.left == rhs.left &&
           lhs.right == rhs.right;
}

inline bool operator!=( const input::Gamepad::Triggers& lhs,
                        const input::Gamepad::Triggers& rhs )
{
    return !( lhs == rhs );
}

inline bool operator==( const input::Gamepad::State& lhs, const input::Gamepad::State& rhs )
{
    return lhs.connected == rhs.connected &&
           lhs.buttons == rhs.buttons &&
           lhs.dPad == rhs.dPad &&
           lhs.thumbSticks == rhs.thumbSticks &&
           lhs.triggers == rhs.triggers;
}
