#include <input/Keyboard.hpp>

#include <GameInput.h>

#include <mutex>

using namespace input;

static KeyboardState state {};
static std::mutex    stateMutex;

KeyboardState Keyboard::getState()
{
    std::lock_guard lock( stateMutex );

    state.ShiftKey   = state.LeftShift || state.RightShift;
    state.ControlKey = state.LeftControl || state.RightControl;
    state.AltKey     = state.LeftAlt || state.RightAlt;

    return state;
}

void Keyboard::reset()
{
    std::lock_guard lock( stateMutex );

    std::memset( &state, 0, sizeof( KeyboardState ) );
}