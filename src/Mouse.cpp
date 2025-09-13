#include <input/Mouse.hpp>

#include <cassert>
#include <cstring> // for memset

using namespace input;

#define UPDATE_BUTTON_STATE( field ) field = static_cast<ButtonState>( ( !!state.field ) | ( ( !!state.field ^ !!lastState.field ) << 1 ) )

void MouseStateTracker::update( const Mouse::State& state ) noexcept
{
    UPDATE_BUTTON_STATE( leftButton );

    assert( ( !state.leftButton && !lastState.leftButton ) == ( leftButton == UP ) );
    assert( ( state.leftButton && lastState.leftButton ) == ( leftButton == HELD ) );
    assert( ( !state.leftButton && lastState.leftButton ) == ( leftButton == RELEASED ) );
    assert( ( state.leftButton && !lastState.leftButton ) == ( leftButton == PRESSED ) );

    UPDATE_BUTTON_STATE( middleButton );
    UPDATE_BUTTON_STATE( rightButton );
    UPDATE_BUTTON_STATE( xButton1 );
    UPDATE_BUTTON_STATE( xButton2 );

    lastState = state;
}

#undef UPDATE_BUTTON_STATE

void MouseStateTracker::reset() noexcept
{
    std::memset( this, 0, sizeof( MouseStateTracker ) );
}
