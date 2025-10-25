#include <input/Gamepad.hpp>

#include <algorithm>  // for std::min & std::max
#include <cassert>    // for assert.
#include <cstring>    // for std::memset.
#include <cmath>      // for std::sqrtf.

namespace input
{
float ApplyLinearDeadZone( float value, float maxValue, float deadZoneSize ) noexcept
{
    if ( value < -deadZoneSize )
    {
        // Increase negative values to remove the deadzone discontinuity.
        value += deadZoneSize;
    }
    else if ( value > deadZoneSize )
    {
        // Decrease positive values to remove the deadzone discontinuity.
        value -= deadZoneSize;
    }
    else
    {
        // Values inside the deadzone come out zero.
        return 0;
    }

    // Scale into 0-1 range.
    const float scaledValue = value / ( maxValue - deadZoneSize );
    return std::max( -1.f, std::min( scaledValue, 1.f ) );
}

void ApplyStickDeadZone(
    float             x,
    float             y,
    Gamepad::DeadZone deadZoneMode,
    float             maxValue,
    float             deadZoneSize,
    float&            resultX,
    float&            resultY ) noexcept
{
    switch ( deadZoneMode )
    {
    case Gamepad::DeadZone::IndependentAxis:
        resultX = ApplyLinearDeadZone( x, maxValue, deadZoneSize );
        resultY = ApplyLinearDeadZone( y, maxValue, deadZoneSize );
        break;

    case Gamepad::DeadZone::Circular:
    {
        const float dist   = std::sqrt( x * x + y * y );
        const float wanted = ApplyLinearDeadZone( dist, maxValue, deadZoneSize );

        const float scale = ( wanted > 0.f ) ? ( wanted / dist ) : 0.f;

        resultX = std::max( -1.f, std::min( x * scale, 1.f ) );
        resultY = std::max( -1.f, std::min( y * scale, 1.f ) );
    }
    break;

    default:  // Gamepad::DeadZone::None
        resultX = ApplyLinearDeadZone( x, maxValue, 0 );
        resultY = ApplyLinearDeadZone( y, maxValue, 0 );
        break;
    }
}
}  // namespace input

using namespace input;

Gamepad::Gamepad( int playerIndex )
: playerIndex { playerIndex }
{}

Gamepad::State Gamepad::getState( DeadZone deadZoneMode ) const
{
    return getState( playerIndex, deadZoneMode );
}
bool Gamepad::setVibration( float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )
{
    return setVibration( playerIndex, leftMotor, rightMotor, leftTrigger, rightTrigger );
}

#define UPDATE_BUTTON_STATE( field ) field = static_cast<ButtonState>( ( !!state.buttons.field ) | ( ( !!state.buttons.field ^ !!lastState.buttons.field ) << 1 ) )

void GamepadStateTracker::update( const Gamepad::State& state )
{
    UPDATE_BUTTON_STATE( a );

    assert( ( !state.buttons.a && !lastState.buttons.a ) == ( a == ButtonState::Up ) );
    assert( ( state.buttons.a && lastState.buttons.a ) == ( a == ButtonState::Held ) );
    assert( ( !state.buttons.a && lastState.buttons.a ) == ( a == ButtonState::Released ) );
    assert( ( state.buttons.a && !lastState.buttons.a ) == ( a == ButtonState::Pressed ) );

    UPDATE_BUTTON_STATE( b );
    UPDATE_BUTTON_STATE( x );
    UPDATE_BUTTON_STATE( y );

    UPDATE_BUTTON_STATE( leftStick );
    UPDATE_BUTTON_STATE( rightStick );

    UPDATE_BUTTON_STATE( leftShoulder );
    UPDATE_BUTTON_STATE( rightShoulder );

    UPDATE_BUTTON_STATE( back );
    UPDATE_BUTTON_STATE( start );

    dPadUp    = static_cast<ButtonState>( ( !!state.dPad.up ) | ( ( !!state.dPad.up ^ !!lastState.dPad.up ) << 1 ) );
    dPadDown  = static_cast<ButtonState>( ( !!state.dPad.down ) | ( ( !!state.dPad.down ^ !!lastState.dPad.down ) << 1 ) );
    dPadLeft  = static_cast<ButtonState>( ( !!state.dPad.left ) | ( ( !!state.dPad.left ^ !!lastState.dPad.left ) << 1 ) );
    dPadRight = static_cast<ButtonState>( ( !!state.dPad.right ) | ( ( !!state.dPad.right ^ !!lastState.dPad.right ) << 1 ) );

    assert( ( !state.dPad.up && !lastState.dPad.up ) == ( dPadUp == ButtonState::Up ) );
    assert( ( state.dPad.up && lastState.dPad.up ) == ( dPadUp == ButtonState::Held ) );
    assert( ( !state.dPad.up && lastState.dPad.up ) == ( dPadUp == ButtonState::Released ) );
    assert( ( state.dPad.up && !lastState.dPad.up ) == ( dPadUp == ButtonState::Pressed ) );

    // Handle 'threshold' tests which emulate buttons

    bool threshold = state.isLeftThumbStickUp();
    leftStickUp    = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isLeftThumbStickUp() ) << 1 ) );

    threshold     = state.isLeftThumbStickDown();
    leftStickDown = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isLeftThumbStickDown() ) << 1 ) );

    threshold     = state.isLeftThumbStickLeft();
    leftStickLeft = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isLeftThumbStickLeft() ) << 1 ) );

    threshold      = state.isLeftThumbStickRight();
    leftStickRight = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isLeftThumbStickRight() ) << 1 ) );

    threshold    = state.isRightThumbStickUp();
    rightStickUp = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isRightThumbStickUp() ) << 1 ) );

    threshold      = state.isRightThumbStickDown();
    rightStickDown = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isRightThumbStickDown() ) << 1 ) );

    threshold      = state.isRightThumbStickLeft();
    rightStickLeft = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isRightThumbStickLeft() ) << 1 ) );

    threshold       = state.isRightThumbStickRight();
    rightStickRight = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isRightThumbStickRight() ) << 1 ) );

    threshold   = state.isLeftTriggerPressed();
    leftTrigger = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isLeftTriggerPressed() ) << 1 ) );

    threshold    = state.isRightTriggerPressed();
    rightTrigger = static_cast<ButtonState>( ( !!threshold ) | ( ( !!threshold ^ !!lastState.isRightTriggerPressed() ) << 1 ) );

    lastState = state;
}

#undef UPDATE_BUTTON_STATE

void GamepadStateTracker::reset() noexcept
{
    std::memset( this, 0, sizeof( GamepadStateTracker ) );
}
