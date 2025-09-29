#include <input/Mouse.hpp>

#include <cassert>
#include <cstring>  // for memset

using namespace input;

#define UPDATE_BUTTON_STATE( field ) field = static_cast<ButtonState>( ( !!state.field ) | ( ( !!state.field ^ !!lastState.field ) << 1 ) )

void MouseStateTracker::update( const Mouse::State& state ) noexcept
{
    UPDATE_BUTTON_STATE( leftButton );

    assert( ( !state.leftButton && !lastState.leftButton ) == ( leftButton == ButtonState::Up ) );
    assert( ( state.leftButton && lastState.leftButton ) == ( leftButton == ButtonState::Held ) );
    assert( ( !state.leftButton && lastState.leftButton ) == ( leftButton == ButtonState::Released ) );
    assert( ( state.leftButton && !lastState.leftButton ) == ( leftButton == ButtonState::Pressed ) );

    UPDATE_BUTTON_STATE( middleButton );
    UPDATE_BUTTON_STATE( rightButton );
    UPDATE_BUTTON_STATE( xButton1 );
    UPDATE_BUTTON_STATE( xButton2 );

    scrollWheelDelta = state.scrollWheelValue - lastState.scrollWheelValue;

    if ( state.positionMode == Mouse::Mode::Relative )
    {
        x = state.x;
        y = state.y;
    }
    else
    {
        x = state.x - lastState.x;
        y = state.y - lastState.y;
    }

    lastState = state;
}

#undef UPDATE_BUTTON_STATE

void MouseStateTracker::reset() noexcept
{
    std::memset( this, 0, sizeof( MouseStateTracker ) );
}
