#pragma once

#include <cstdint>

namespace input
{

// Source (September 15, 2025): https://github.com/microsoft/DirectXTK/blob/main/Inc/Mouse.h
class Mouse
{
public:
    static Mouse& get()
    {
        static Mouse mouse;
        return mouse;
    }

    enum class Mode : uint8_t
    {
        Absolute, ///< Mouse position is reported based on its location in the window.
        Relative  ///< Mouse movement is reported as a series of delta values, rather than an absolute position.
    };

    /// <summary>
    /// The state of the mouse.
    /// Use Mouse::getState() to query the current state of the mouse.
    /// </summary>
    struct State
    {
        /// <summary>
        /// The state of the left mouse button.
        /// `true` if the left mouse button is held, `false` otherwise.
        /// </summary>
        bool leftButton;

        /// <summary>
        /// The state of the middle mouse button.
        /// `true` if the middle mouse button is held, `false` otherwise.
        /// </summary>
        bool middleButton;

        /// <summary>
        /// State of the right mouse button.
        /// `true` if the right mouse button is held, `false` otherwise.
        /// </summary>
        bool rightButton;

        /// <summary>
        /// State of the first extra mouse button.
        /// `true` if the first extra mouse button is held, `false` otherwise.
        /// </summary>
        bool xButton1;

        /// <summary>
        /// State of the second extra mouse button.
        /// `true` if the second extra mouse button is held, `false` otherwise.
        /// </summary>
        bool xButton2;

        /// <summary>
        /// The x-coordinate of the mouse cursor relative to the top-left corner of the window that received the mouse event.
        /// </summary>
        /// <remarks>
        /// If the mouse cursor is locked to a window, then this is the relative x mouse
        /// motion, otherwise it is the position of the mouse cursor relative to the window that
        /// sent the event.
        /// </remarks>
        /// <see cref="Mouse::setWindow" />
        float x;

        /// <summary>
        /// The y-coordinate of the mouse cursor relative to the top-left corner of the window that received the mouse event.
        /// </summary>
        /// <remarks>
        /// If the mouse cursor is locked to a window, then this is the relative y mouse
        /// motion, otherwise it is the position of the mouse cursor relative to the window that
        /// sent the event.
        /// </remarks>
        /// <see cref="Mouse::setWindow" />
        float y;

        /// <summary>
        /// The value of the mouse's scroll wheel.
        /// </summary>
        int scrollWheelValue;

        /// <summary>
        /// Whether the mouse's x and y coordinates are absolute (relative to the top-left corner of the window)
        /// or 
        /// </summary>
        Mode positionMode;

        /// <summary>
        /// Used to check if this MouseState is equivalent to another.
        /// </summary>
        /// <remarks>
        /// Used to check if the mouse state has changed since the last time it was queried.
        /// </remarks>
        /// <returns>`true` if they are equal, `false` otherwise.</returns>
        bool operator==( const State& ) const = default;

        /// <summary>
        /// Used to check if this MouseState is different to another.
        /// </summary>
        /// <remarks>
        /// Used to check if the mouse state has changed since the last time it was queried.
        /// </remarks>
        /// <returns>`true` if they are not equal, `false` otherwise.</returns>
        bool operator!=( const State& ) const = default;
    };

    State getState() const;

    void resetScrollWheelValue() noexcept;

    void setMode( Mode mode );

    void resetRelativeMotion() noexcept;

    bool isConnected() const;

    bool isVisible() const noexcept;
    void setVisible( bool visible );

    void setWindow( void* window );

    Mouse( const Mouse& )     = delete;
    Mouse( Mouse&& ) noexcept = delete;

    Mouse& operator=( const Mouse& ) = delete;
    Mouse& operator=( Mouse&& )      = delete;

private:
    Mouse() = default;
    ~Mouse() = default;
};

class MouseStateTracker
{
public:
    enum class ButtonState : uint8_t
    {
        Up       = 0,  // Button is up
        Held     = 1,  // Button is held down
        Released = 2,  // Button was just released
        Pressed  = 3,  // Buton was just pressed
    };

    ButtonState leftButton;
    ButtonState middleButton;
    ButtonState rightButton;
    ButtonState xButton1;
    ButtonState xButton2;

    int scrollWheelDelta;

    MouseStateTracker() noexcept
    {
        reset();
    }

    void update( const Mouse::State& state ) noexcept;

    void reset() noexcept;

    Mouse::State getLastState() const noexcept
    {
        return lastState;
    }

private:
    Mouse::State lastState;
};

}  // namespace input
