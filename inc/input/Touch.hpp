#pragma once

#include "ButtonState.hpp"

#include <cstdint>
#include <vector>

namespace input
{

namespace Touch
{

/// <summary>
/// The maximum number of simultaneous touch points supported.
/// </summary>
static constexpr size_t MAX_TOUCH_COUNT = 10;

/// <summary>
/// Touch phase state.
/// </summary>
enum class Phase : uint8_t
{
    Began,       ///< Touch point has just been placed on the touch surface.
    Moved,       ///< Touch point has moved on the touch surface.
    Stationary,  ///< Touch point is on the surface but has not moved since last event.
    Ended,       ///< Touch point has been removed from the touch surface.
    Cancelled    ///< System cancelled tracking for the touch point.
};

/// <summary>
/// Represents a single touch point.
/// </summary>
struct TouchPoint
{
    /// <summary>
    /// The unique identifier for this touch point.
    /// </summary>
    /// <remarks>
    /// A finger touching the screen will have a consistent ID for the duration of the touch.
    /// </remarks>
    uint64_t id;

    /// <summary>
    /// Timestamp in nanoseconds.
    /// <remarks>
    /// Touch events can become and stuck in a stationary state.
    /// Stationary touch points are removed after 1 second.
    /// </remarks>
    /// </summary>
    uint64_t timestamp;

    /// <summary>
    /// The x-coordinate of the touch point, normalized to [0.0, 1.0] where 0.0 is left and 1.0 is right.
    /// </summary>
    float x;

    /// <summary>
    /// The y-coordinate of the touch point, normalized to [0.0, 1.0] where 0.0 is top and 1.0 is bottom.
    /// </summary>
    float y;

    /// <summary>
    /// The pressure of the touch point, normalized to [0.0, 1.0].
    /// </summary>
    /// <remarks>
    /// 0.0 indicates no pressure, 1.0 indicates maximum pressure.
    /// If pressure is not supported, this value will be 1.0 for active touches.
    /// </remarks>
    float pressure;

    /// <summary>
    /// The current phase of this touch point.
    /// </summary>
    Phase phase;

    /// <summary>
    /// Used to check if this TouchPoint is equivalent to another.
    /// </summary>
    /// <returns>`true` if they are equal, `false` otherwise.</returns>
    bool operator==( const TouchPoint& ) const = default;

    /// <summary>
    /// Used to check if this TouchPoint is different from another.
    /// </summary>
    /// <returns>`true` if they are not equal, `false` otherwise.</returns>
    bool operator!=( const TouchPoint& ) const = default;
};

/// <summary>
/// The state of all touch input.
/// Use Touch::getState() to query the current state of touch input.
/// </summary>
struct State
{
    /// <summary>
    /// The collection of active touch points.
    /// </summary>
    std::vector<TouchPoint> touches;

    /// <summary>
    /// Used to check if this State is equivalent to another.
    /// </summary>
    /// <returns>`true` if they are equal, `false` otherwise.</returns>
    bool operator==( const State& ) const = default;

    /// <summary>
    /// Used to check if this State is different from another.
    /// </summary>
    /// <returns>`true` if they are not equal, `false` otherwise.</returns>
    bool operator!=( const State& ) const = default;
};

/// <summary>
/// Get the current state of touch input.
/// </summary>
/// <returns>The current touch state.</returns>
State getState();

/// <summary>
/// Called at the end of the frame to remove touch inputs and set moving touch inputs to stationary.
/// </summary>
void endFrame();

/// <summary>
/// Check if touch input is supported.
/// </summary>
/// <returns>`true` if touch input is supported, `false` otherwise.</returns>
bool isSupported();

/// <summary>
/// Get the number of touch devices available.
/// </summary>
/// <returns>The number of touch devices.</returns>
int getDeviceCount();

/// <summary>
/// Set the window handle for touch input (Win32 only).
/// </summary>
/// <param name="window">Pointer to the window handle.</param>
void setWindow(void* window);
}  // namespace Touch

/// <summary>
/// Helper class to track touch state changes between frames.
/// </summary>
class TouchStateTracker
{
public:
    /// <summary>
    /// Represents the state changes for a single touch point.
    /// </summary>
    struct TrackedTouch
    {
        int64_t     id;
        ButtonState state;
        float       x;
        float       y;
        float       deltaX;
        float       deltaY;
        float       pressure;
    };

    TouchStateTracker() noexcept
    {
        reset();
    }

    /// <summary>
    /// Update the tracker with the current touch state.
    /// </summary>
    /// <param name="state">The current touch state.</param>
    void update( const Touch::State& state ) noexcept;

    /// <summary>
    /// Reset the tracker to its initial state.
    /// </summary>
    void reset() noexcept;

    /// <summary>
    /// Get the last recorded touch state.
    /// </summary>
    /// <returns>The last touch state.</returns>
    Touch::State getLastState() const noexcept
    {
        return lastState;
    }

    /// <summary>
    /// Get the collection of tracked touches.
    /// </summary>
    /// <returns>Vector of tracked touch points.</returns>
    const std::vector<TrackedTouch>& getTrackedTouches() const noexcept
    {
        return trackedTouches;
    }

private:
    Touch::State             lastState;
    std::vector<TrackedTouch> trackedTouches;
};

}  // namespace input
