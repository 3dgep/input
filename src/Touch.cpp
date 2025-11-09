#include <input/Touch.hpp>

#include <algorithm>

using namespace input;

void TouchStateTracker::update( const Touch::State& state ) noexcept
{
    trackedTouches.clear();

    // Process current touches
    for ( const auto& touch : state.touches )
    {
        TrackedTouch tracked;
        tracked.id       = touch.id;
        tracked.x        = touch.x;
        tracked.y        = touch.y;
        tracked.pressure = touch.pressure;

        // Find matching touch in last state
        auto lastTouch = std::find_if( lastState.touches.begin(), lastState.touches.end(),
                                       [&]( const Touch::TouchPoint& t ) { return t.id == touch.id; } );

        if ( lastTouch != lastState.touches.end() )
        {
            // Touch existed in last frame
            tracked.deltaX = touch.x - lastTouch->x;
            tracked.deltaY = touch.y - lastTouch->y;

            // Determine button state based on phase
            switch ( touch.phase )
            {
            case Touch::Phase::Began:
                tracked.state = ButtonState::Pressed;
                break;
            case Touch::Phase::Moved:
            case Touch::Phase::Stationary:
                tracked.state = ButtonState::Held;
                break;
            case Touch::Phase::Ended:
            case Touch::Phase::Cancelled:
                tracked.state = ButtonState::Released;
                break;
            }
        }
        else
        {
            // New touch
            tracked.deltaX = 0.0f;
            tracked.deltaY = 0.0f;
            tracked.state  = ButtonState::Pressed;
        }

        trackedTouches.push_back( tracked );
    }

    // Check for touches that ended (were in last state but not in current)
    for ( const auto& lastTouch : lastState.touches )
    {
        auto currentTouch = std::find_if( state.touches.begin(), state.touches.end(),
                                          [&]( const Touch::TouchPoint& t ) { return t.id == lastTouch.id; } );

        if ( currentTouch == state.touches.end() )
        {
            // Touch ended - add as released
            TrackedTouch tracked;
            tracked.id       = lastTouch.id;
            tracked.x        = lastTouch.x;
            tracked.y        = lastTouch.y;
            tracked.pressure = 0.0f;
            tracked.deltaX   = 0.0f;
            tracked.deltaY   = 0.0f;
            tracked.state    = ButtonState::Released;
            trackedTouches.push_back( tracked );
        }
    }

    lastState = state;
}

void TouchStateTracker::reset() noexcept
{
    lastState.touches.clear();
    trackedTouches.clear();
}
