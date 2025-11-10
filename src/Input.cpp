#include <input/Input.hpp>
#include <input/Touch.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>

using namespace input;

namespace
{

GamepadStateTracker  g_GamepadStateTrackers[Gamepad::MAX_PLAYER_COUNT];
KeyboardStateTracker g_KeyboardStateTracker;
MouseStateTracker    g_MouseStateTracker;
TouchStateTracker    g_TouchStateTracker;

using Keyboard::Key;
using Mouse::Button;

std::unordered_map<std::string, Key> g_KeyMap = {
    { "a", Key::A },
    { "b", Key::B },
    { "c", Key::C },
    { "d", Key::D },
    { "e", Key::E },
    { "f", Key::F },
    { "g", Key::G },
    { "h", Key::H },
    { "i", Key::I },
    { "j", Key::J },
    { "k", Key::K },
    { "l", Key::L },
    { "m", Key::M },
    { "n", Key::N },
    { "o", Key::O },
    { "p", Key::P },
    { "q", Key::Q },
    { "r", Key::R },
    { "s", Key::S },
    { "t", Key::T },
    { "u", Key::U },
    { "v", Key::V },
    { "w", Key::W },
    { "x", Key::X },
    { "y", Key::Y },
    { "z", Key::Z },
    { "1", Key::D1 },
    { "2", Key::D2 },
    { "3", Key::D3 },
    { "4", Key::D4 },
    { "5", Key::D5 },
    { "6", Key::D6 },
    { "7", Key::D7 },
    { "8", Key::D8 },
    { "9", Key::D9 },
    { "0", Key::D0 },
    { "up", Key::Up },
    { "down", Key::Down },
    { "left", Key::Left },
    { "right", Key::Right },
    { "[1]", Key::NumPad1 },
    { "[2]", Key::NumPad2 },
    { "[3]", Key::NumPad3 },
    { "[4]", Key::NumPad4 },
    { "[5]", Key::NumPad5 },
    { "[6]", Key::NumPad6 },
    { "[7]", Key::NumPad7 },
    { "[8]", Key::NumPad8 },
    { "[9]", Key::NumPad9 },
    { "[0]", Key::NumPad0 },
    { "[+]", Key::Add },
    { "[-]", Key::Subtract },
    { "[*]", Key::Multiply },
    { "[=]", Key::Separator },
    { "[/]", Key::Divide },
    { "caps", Key::CapsLock },
    { "capslock", Key::CapsLock },
    { "shift", Key::ShiftKey },
    { "left shift", Key::LeftShift },
    { "right shift", Key::RightShift },
    { "ctrl", Key::ControlKey },
    { "left ctrl", Key::LeftControl },
    { "right ctrl", Key::RightControl },
    { "alt", Key::AltKey },
    { "left alt", Key::LeftAlt },
    { "right alt", Key::RightAlt },
    { "left super", Key::LeftSuper },
    { "right super", Key::RightSuper },
    { "left win", Key::LeftSuper },
    { "right win", Key::RightSuper },
    { "backspace", Key::Back },
    { "tab", Key::Tab },
    { "enter", Key::Enter },
    { "return", Key::Enter },
    { "esc", Key::Escape },
    { "escape", Key::Escape },
    { "space", Key::Space },
    { "delete", Key::Delete },
    { "ins", Key::Insert },
    { "insert", Key::Insert },
    { "home", Key::Home },
    { "end", Key::End },
    { "pgup", Key::PageUp },
    { "page up", Key::PageUp },
    { "pgdn", Key::PageDown },
    { "page down", Key::PageDown },
    { ";", Key::OemSemicolon },
    { "+", Key::OemPlus },
    { ",", Key::OemComma },
    { "-", Key::OemMinus },
    { ".", Key::OemPeriod },
    { "?", Key::OemQuestion },
    { "~", Key::OemTilde },
    { "`", Key::OemTilde },
    { "[", Key::OemOpenBrackets },
    { "{", Key::OemOpenBrackets },
    { "]", Key::OemCloseBrackets },
    { "}", Key::OemCloseBrackets },
    { "'", Key::OemQuotes },
    { "\"", Key::OemQuotes },
    { "|", Key::OemPipe },
    { "\\", Key::OemPipe },
    { "F1", Key::F1 },
    { "f1", Key::F1 },
    { "F2", Key::F2 },
    { "f2", Key::F2 },
    { "F3", Key::F3 },
    { "f3", Key::F3 },
    { "F4", Key::F4 },
    { "f4", Key::F4 },
    { "F5", Key::F5 },
    { "f5", Key::F5 },
    { "F6", Key::F6 },
    { "f6", Key::F6 },
    { "F7", Key::F7 },
    { "f7", Key::F7 },
    { "F8", Key::F8 },
    { "f8", Key::F8 },
    { "F9", Key::F9 },
    { "f9", Key::F9 },
    { "F10", Key::F10 },
    { "f10", Key::F10 },
    { "F11", Key::F11 },
    { "f11", Key::F11 },
    { "F12", Key::F12 },
    { "f12", Key::F12 },
};

std::unordered_map<std::string, AxisCallback> g_AxisMap = {
    { "Horizontal", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& mouseState ) {
         float leftX  = 0.0f;
         float rightX = 0.0f;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();

             leftX += state.thumbSticks.leftX;
             rightX += state.thumbSticks.rightX;
         }

         const auto keyState = keyboardState.getLastState();

         const float a     = keyState.isKeyDown( Key::A ) ? 1.0f : 0.0f;
         const float d     = keyState.isKeyDown( Key::D ) ? 1.0f : 0.0f;
         const float left  = keyState.isKeyDown( Key::Left ) ? 1.0f : 0.0f;
         const float right = keyState.isKeyDown( Key::Right ) ? 1.0f : 0.0f;

         return std::clamp( leftX + rightX - a + d - left + right, -1.0f, 1.0f );
     } },
    { "Vertical", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& mouseState ) {
         float leftY  = 0.0f;
         float rightY = 0.0f;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();

             leftY += state.thumbSticks.leftY;
             rightY += state.thumbSticks.rightY;
         }

         const auto keyState = keyboardState.getLastState();

         const float s    = keyState.isKeyDown( Key::S ) ? 1.0f : 0.0f;
         const float w    = keyState.isKeyDown( Key::W ) ? 1.0f : 0.0f;
         const float up   = keyState.isKeyDown( Key::Up ) ? 1.0f : 0.0f;
         const float down = keyState.isKeyDown( Key::Down ) ? 1.0f : 0.0f;

         return std::clamp( leftY + rightY - s + w - down + up, -1.0f, 1.0f );
     } },
    { "Fire1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& mouseState ) {
         float rightTrigger = 0.0f;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             rightTrigger += state.triggers.right;
         }

         const float ctrl       = keyboardState.getLastState().isKeyDown( Key::LeftControl ) ? 1.0f : 0.0f;
         const float leftButton = mouseState.getLastState().leftButton ? 1.0f : 0.0f;

         return std::clamp( rightTrigger + ctrl + leftButton, 0.0f, 1.0f );
     } },
    { "Fire2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& mouseState ) {
         float leftTrigger = 0.0f;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             leftTrigger += state.triggers.left;
         }

         const float alt         = keyboardState.getLastState().isKeyDown( Key::LeftAlt ) ? 1.0f : 0.0f;
         const float rightButton = mouseState.getLastState().rightButton ? 1.0f : 0.0f;

         return std::clamp( leftTrigger + alt + rightButton, 0.0f, 1.0f );
     } },
    { "Fire3", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker& keyboardState, const MouseStateTracker& mouseState ) {
         const float shift        = keyboardState.getLastState().isKeyDown( Key::LeftShift ) ? 1.0f : 0.0f;
         const float middleButton = mouseState.getLastState().middleButton ? 1.0f : 0.0f;

         return std::clamp( shift + middleButton, 0.0f, 1.0f );
     } },
    { "Jump", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& mouseState ) {
         float a = 0.0f;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             a += state.buttons.a ? 1.0f : 0.0f;
         }

         const float space = keyboardState.getLastState().isKeyDown( Key::Space ) ? 1.0f : 0.0f;

         return std::clamp( a + space, 0.0f, 1.0f );
     } },
    { "Mouse X", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.getLastState().x;
     } },
    { "Mouse Y", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.getLastState().y;
     } },
    { "Mouse ScrollWheel", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return static_cast<float>( mouseState.getLastState().scrollWheelValue );
     } },
    { "Submit", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         float a     = 0.0f;
         float start = 0.0f;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             a += state.buttons.a ? 1.0f : 0.0f;
             start += state.buttons.start ? 1.0f : 0.0f;
         }

         auto        keyState = keyboardState.getLastState();
         const float enter    = keyState.isKeyDown( Key::Enter ) ? 1.0f : 0.0f;
         const float space    = keyState.isKeyDown( Key::Space ) ? 1.0f : 0.0f;

         return std::clamp( a + start + enter + space, 0.0f, 1.0f );
     } },
    { "Cancel", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         float b    = 0.0f;
         float back = 0.0f;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             b += state.buttons.b ? 1.0f : 0.0f;
             back += state.buttons.back ? 1.0f : 0.0f;
         }

         const float esc = keyboardState.getLastState().isKeyDown( Key::Escape ) ? 1.0f : 0.0f;

         return std::clamp( b + back + esc, 0.0f, 1.0f );
     } },
};

std::unordered_map<std::string, ButtonCallback> g_ButtonMap = {
    { "win", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         return keyboardState.getLastState().isKeyDown( Key::LeftSuper ) || keyboardState.getLastState().isKeyDown( Key::RightSuper );
     } },
    { "mouse 0", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.getLastState().leftButton;
     } },
    { "mouse 1", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.getLastState().rightButton;
     } },
    { "mouse 2", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.getLastState().middleButton;
     } },
    { "mouse x1", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.getLastState().xButton1;
     } },
    { "mouse x2", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.getLastState().xButton2;
     } },
    { "joystick button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.a;
         }

         return button;
     } },
    { "joystick button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.b;
         }

         return button;
     } },
    { "joystick button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.x;
         }

         return button;
     } },
    { "joystick button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.y;
         }

         return button;
     } },
    { "joystick button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.leftShoulder;
         }

         return button;
     } },
    { "joystick button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.rightShoulder;
         }

         return button;
     } },
    { "joystick button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.view;
         }

         return button;
     } },
    { "joystick button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.menu;
         }

         return button;
     } },
    { "joystick button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.leftStick;
         }

         return button;
     } },
    { "joystick button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.buttons.rightStick;
         }

         return button;
     } },
    { "joystick dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.dPad.up;
         }

         return button;
     } },
    { "joystick dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.dPad.down;
         }

         return button;
     } },
    { "joystick dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.dPad.left;
         }

         return button;
     } },
    { "joystick dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             button |= state.dPad.right;
         }

         return button;
     } },
    { "joystick 1 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.a;
     } },
    { "joystick 1 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.b;
     } },
    { "joystick 1 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.x;
     } },
    { "joystick 1 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.y;
     } },
    { "joystick 1 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.leftShoulder;
     } },
    { "joystick 1 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.rightShoulder;
     } },
    { "joystick 1 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.view;
     } },
    { "joystick 1 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.menu;
     } },
    { "joystick 1 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.leftStick;
     } },
    { "joystick 1 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().buttons.rightStick;
     } },
    { "joystick 1 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().dPad.up;
     } },
    { "joystick 1 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().dPad.down;
     } },
    { "joystick 1 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().dPad.left;
     } },
    { "joystick 1 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].getLastState().dPad.right;
     } },
    { "joystick 2 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.a;
     } },
    { "joystick 2 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.b;
     } },
    { "joystick 2 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.x;
     } },
    { "joystick 2 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.y;
     } },
    { "joystick 2 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.leftShoulder;
     } },
    { "joystick 2 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.rightShoulder;
     } },
    { "joystick 2 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.view;
     } },
    { "joystick 2 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.menu;
     } },
    { "joystick 2 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.leftStick;
     } },
    { "joystick 2 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().buttons.rightStick;
     } },
    { "joystick 2 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().dPad.up;
     } },
    { "joystick 2 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().dPad.down;
     } },
    { "joystick 2 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().dPad.left;
     } },
    { "joystick 2 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].getLastState().dPad.right;
     } },
    { "joystick 3 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.a;
     } },
    { "joystick 3 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.b;
     } },
    { "joystick 3 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.x;
     } },
    { "joystick 3 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.y;
     } },
    { "joystick 3 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.leftShoulder;
     } },
    { "joystick 3 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.rightShoulder;
     } },
    { "joystick 3 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.view;
     } },
    { "joystick 3 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.menu;
     } },
    { "joystick 3 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.leftStick;
     } },
    { "joystick 3 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().buttons.rightStick;
     } },
    { "joystick 3 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().dPad.up;
     } },
    { "joystick 3 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().dPad.down;
     } },
    { "joystick 3 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().dPad.left;
     } },
    { "joystick 3 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].getLastState().dPad.right;
     } },
    { "joystick 4 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.a;
     } },
    { "joystick 4 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.b;
     } },
    { "joystick 4 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.x;
     } },
    { "joystick 4 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.y;
     } },
    { "joystick 4 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.leftShoulder;
     } },
    { "joystick 4 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.rightShoulder;
     } },
    { "joystick 4 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.view;
     } },
    { "joystick 4 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.menu;
     } },
    { "joystick 4 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.leftStick;
     } },
    { "joystick 4 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().buttons.rightStick;
     } },
    { "joystick 4 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().dPad.up;
     } },
    { "joystick 4 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().dPad.down;
     } },
    { "joystick 4 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().dPad.left;
     } },
    { "joystick 4 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].getLastState().dPad.right;
     } },
    { "Submit", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         bool a     = false;
         bool start = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             a                = a || state.buttons.a;
             start            = start || state.buttons.start;
         }

         auto       keyState = keyboardState.getLastState();
         const bool enter    = keyState.isKeyDown( Key::Enter );
         const bool space    = keyState.isKeyDown( Key::Space );

         return a || start || enter || space;
     } },
    { "Cancel", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         bool b    = false;
         bool back = false;

         for ( auto& gamePadState: gamePadStates )
         {
             const auto state = gamePadState.getLastState();
             b                = b || state.buttons.b;
             back             = back || state.buttons.back;
         }

         auto       keyState = keyboardState.getLastState();
         const bool esc      = keyState.isKeyDown( Key::Escape );

         return b || back || esc;
     } },
};

std::unordered_map<std::string, ButtonCallback> g_ButtonDownMap = {
    { "win", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         return keyboardState.isKeyPressed( Key::LeftSuper ) || keyboardState.isKeyPressed( Key::RightSuper );
     } },
    { "mouse 0", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.leftButton == ButtonState::Pressed;
     } },
    { "mouse 1", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.rightButton == ButtonState::Pressed;
     } },
    { "mouse 2", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.middleButton == ButtonState::Pressed;
     } },
    { "mouse x1", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.xButton1 == ButtonState::Pressed;
     } },
    { "mouse x2", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.xButton2 == ButtonState::Pressed;
     } },
    { "joystick button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.a == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.b == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.x == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.y == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.leftShoulder == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.rightShoulder == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.view == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.menu == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.leftStick == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.rightStick == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadUp == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadDown == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadLeft == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadRight == ButtonState::Pressed;
         }

         return button;
     } },
    { "joystick 1 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].a == ButtonState::Pressed;
     } },
    { "joystick 1 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].b == ButtonState::Pressed;
     } },
    { "joystick 1 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].x == ButtonState::Pressed;
     } },
    { "joystick 1 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].y == ButtonState::Pressed;
     } },
    { "joystick 1 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].leftShoulder == ButtonState::Pressed;
     } },
    { "joystick 1 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].rightShoulder == ButtonState::Pressed;
     } },
    { "joystick 1 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].view == ButtonState::Pressed;
     } },
    { "joystick 1 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].menu == ButtonState::Pressed;
     } },
    { "joystick 1 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].leftStick == ButtonState::Pressed;
     } },
    { "joystick 1 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].rightStick == ButtonState::Pressed;
     } },
    { "joystick 1 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadUp == ButtonState::Pressed;
     } },
    { "joystick 1 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadDown == ButtonState::Pressed;
     } },
    { "joystick 1 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadLeft == ButtonState::Pressed;
     } },
    { "joystick 1 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadRight == ButtonState::Pressed;
     } },
    { "joystick 2 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].a == ButtonState::Pressed;
     } },
    { "joystick 2 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].b == ButtonState::Pressed;
     } },
    { "joystick 2 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].x == ButtonState::Pressed;
     } },
    { "joystick 2 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].y == ButtonState::Pressed;
     } },
    { "joystick 2 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].leftShoulder == ButtonState::Pressed;
     } },
    { "joystick 2 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].rightShoulder == ButtonState::Pressed;
     } },
    { "joystick 2 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].view == ButtonState::Pressed;
     } },
    { "joystick 2 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].menu == ButtonState::Pressed;
     } },
    { "joystick 2 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].leftStick == ButtonState::Pressed;
     } },
    { "joystick 2 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].rightStick == ButtonState::Pressed;
     } },
    { "joystick 2 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadUp == ButtonState::Pressed;
     } },
    { "joystick 2 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadDown == ButtonState::Pressed;
     } },
    { "joystick 2 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadLeft == ButtonState::Pressed;
     } },
    { "joystick 2 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadRight == ButtonState::Pressed;
     } },
    { "joystick 3 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].a == ButtonState::Pressed;
     } },
    { "joystick 3 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].b == ButtonState::Pressed;
     } },
    { "joystick 3 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].x == ButtonState::Pressed;
     } },
    { "joystick 3 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].y == ButtonState::Pressed;
     } },
    { "joystick 3 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].leftShoulder == ButtonState::Pressed;
     } },
    { "joystick 3 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].rightShoulder == ButtonState::Pressed;
     } },
    { "joystick 3 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].view == ButtonState::Pressed;
     } },
    { "joystick 3 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].menu == ButtonState::Pressed;
     } },
    { "joystick 3 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].leftStick == ButtonState::Pressed;
     } },
    { "joystick 3 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].rightStick == ButtonState::Pressed;
     } },
    { "joystick 3 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadUp == ButtonState::Pressed;
     } },
    { "joystick 3 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadDown == ButtonState::Pressed;
     } },
    { "joystick 3 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadLeft == ButtonState::Pressed;
     } },
    { "joystick 3 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadRight == ButtonState::Pressed;
     } },
    { "joystick 4 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].a == ButtonState::Pressed;
     } },
    { "joystick 4 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].b == ButtonState::Pressed;
     } },
    { "joystick 4 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].x == ButtonState::Pressed;
     } },
    { "joystick 4 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].y == ButtonState::Pressed;
     } },
    { "joystick 4 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].leftShoulder == ButtonState::Pressed;
     } },
    { "joystick 4 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].rightShoulder == ButtonState::Pressed;
     } },
    { "joystick 4 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].view == ButtonState::Pressed;
     } },
    { "joystick 4 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].menu == ButtonState::Pressed;
     } },
    { "joystick 4 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].leftStick == ButtonState::Pressed;
     } },
    { "joystick 4 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].rightStick == ButtonState::Pressed;
     } },
    { "joystick 4 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadUp == ButtonState::Pressed;
     } },
    { "joystick 4 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadDown == ButtonState::Pressed;
     } },
    { "joystick 4 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadLeft == ButtonState::Pressed;
     } },
    { "joystick 4 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadRight == ButtonState::Pressed;
     } },
    { "Submit", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         bool a     = false;
         bool start = false;

         for ( auto& gamePadState: gamePadStates )
         {
             a     = a || gamePadState.a == ButtonState::Pressed;
             start = start || gamePadState.start == ButtonState::Pressed;
         }

         const bool enter = keyboardState.isKeyPressed( Key::Enter );
         const bool space = keyboardState.isKeyPressed( Key::Space );

         return a || start || enter || space;
     } },
    { "Cancel", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         bool b    = false;
         bool back = false;

         for ( auto& gamePadState: gamePadStates )
         {
             b    = b || gamePadState.b == ButtonState::Pressed;
             back = back || gamePadState.back == ButtonState::Pressed;
         }

         const bool esc = keyboardState.isKeyPressed( Key::Escape );

         return b || back || esc;
     } },
};

std::unordered_map<std::string, ButtonCallback> g_ButtonUpMap = {
    { "win", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         return keyboardState.isKeyReleased( Key::LeftSuper ) || keyboardState.isKeyReleased( Key::RightSuper );
     } },
    { "mouse 0", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.leftButton == ButtonState::Released;
     } },
    { "mouse 1", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.rightButton == ButtonState::Released;
     } },
    { "mouse 2", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.middleButton == ButtonState::Released;
     } },
    { "mouse x1", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.xButton1 == ButtonState::Released;
     } },
    { "mouse x2", []( std::span<const GamepadStateTracker>, const KeyboardStateTracker&, const MouseStateTracker& mouseState ) {
         return mouseState.xButton2 == ButtonState::Released;
     } },
    { "joystick button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.a == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.b == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.x == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.y == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.leftShoulder == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.rightShoulder == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.view == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.menu == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.leftStick == ButtonState::Released;
         }

         return button;
     } },
    { "joystick button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.rightStick == ButtonState::Released;
         }

         return button;
     } },
    { "joystick dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadUp == ButtonState::Released;
         }

         return button;
     } },
    { "joystick dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadDown == ButtonState::Released;
         }

         return button;
     } },
    { "joystick dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadLeft == ButtonState::Released;
         }

         return button;
     } },
    { "joystick dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         bool button = false;

         for ( auto& gamePadState: gamePadStates )
         {
             button |= gamePadState.dPadRight == ButtonState::Released;
         }

         return button;
     } },
    { "joystick 1 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].a == ButtonState::Released;
     } },
    { "joystick 1 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].b == ButtonState::Released;
     } },
    { "joystick 1 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].x == ButtonState::Released;
     } },
    { "joystick 1 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].y == ButtonState::Released;
     } },
    { "joystick 1 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].leftShoulder == ButtonState::Released;
     } },
    { "joystick 1 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].rightShoulder == ButtonState::Released;
     } },
    { "joystick 1 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].view == ButtonState::Released;
     } },
    { "joystick 1 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].menu == ButtonState::Released;
     } },
    { "joystick 1 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].leftStick == ButtonState::Released;
     } },
    { "joystick 1 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].rightStick == ButtonState::Released;
     } },
    { "joystick 1 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadUp == ButtonState::Released;
     } },
    { "joystick 1 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadDown == ButtonState::Released;
     } },
    { "joystick 1 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadLeft == ButtonState::Released;
     } },
    { "joystick 1 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[0].dPadRight == ButtonState::Released;
     } },
    { "joystick 2 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].a == ButtonState::Released;
     } },
    { "joystick 2 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].b == ButtonState::Released;
     } },
    { "joystick 2 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].x == ButtonState::Released;
     } },
    { "joystick 2 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].y == ButtonState::Released;
     } },
    { "joystick 2 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].leftShoulder == ButtonState::Released;
     } },
    { "joystick 2 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].rightShoulder == ButtonState::Released;
     } },
    { "joystick 2 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].view == ButtonState::Released;
     } },
    { "joystick 2 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].menu == ButtonState::Released;
     } },
    { "joystick 2 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].leftStick == ButtonState::Released;
     } },
    { "joystick 2 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].rightStick == ButtonState::Released;
     } },
    { "joystick 2 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadUp == ButtonState::Released;
     } },
    { "joystick 2 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadDown == ButtonState::Released;
     } },
    { "joystick 2 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadLeft == ButtonState::Released;
     } },
    { "joystick 2 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[1].dPadRight == ButtonState::Released;
     } },
    { "joystick 3 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].a == ButtonState::Released;
     } },
    { "joystick 3 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].b == ButtonState::Released;
     } },
    { "joystick 3 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].x == ButtonState::Released;
     } },
    { "joystick 3 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].y == ButtonState::Released;
     } },
    { "joystick 3 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].leftShoulder == ButtonState::Released;
     } },
    { "joystick 3 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].rightShoulder == ButtonState::Released;
     } },
    { "joystick 3 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].view == ButtonState::Released;
     } },
    { "joystick 3 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].menu == ButtonState::Released;
     } },
    { "joystick 3 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].leftStick == ButtonState::Released;
     } },
    { "joystick 3 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].rightStick == ButtonState::Released;
     } },
    { "joystick 3 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadUp == ButtonState::Released;
     } },
    { "joystick 3 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadDown == ButtonState::Released;
     } },
    { "joystick 3 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadLeft == ButtonState::Released;
     } },
    { "joystick 3 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[2].dPadRight == ButtonState::Released;
     } },
    { "joystick 4 button 1", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].a == ButtonState::Released;
     } },
    { "joystick 4 button 2", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].b == ButtonState::Released;
     } },
    { "joystick 4 button 3", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].x == ButtonState::Released;
     } },
    { "joystick 4 button 4", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].y == ButtonState::Released;
     } },
    { "joystick 4 button 5", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].leftShoulder == ButtonState::Released;
     } },
    { "joystick 4 button 6", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].rightShoulder == ButtonState::Released;
     } },
    { "joystick 4 button 7", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].view == ButtonState::Released;
     } },
    { "joystick 4 button 8", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].menu == ButtonState::Released;
     } },
    { "joystick 4 button 9", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].leftStick == ButtonState::Released;
     } },
    { "joystick 4 button 10", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].rightStick == ButtonState::Released;
     } },
    { "joystick 4 dpad up", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadUp == ButtonState::Released;
     } },
    { "joystick 4 dpad down", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadDown == ButtonState::Released;
     } },
    { "joystick 4 dpad left", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadLeft == ButtonState::Released;
     } },
    { "joystick 4 dpad right", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker&, const MouseStateTracker& ) {
         return gamePadStates[3].dPadRight == ButtonState::Released;
     } },
    { "Submit", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         bool a     = false;
         bool start = false;

         for ( auto& gamePadState: gamePadStates )
         {
             a     = a || gamePadState.a == ButtonState::Released;
             start = start || gamePadState.start == ButtonState::Released;
         }

         const bool enter = keyboardState.isKeyReleased( Key::Enter );
         const bool space = keyboardState.isKeyReleased( Key::Space );

         return a || start || enter || space;
     } },
    { "Cancel", []( std::span<const GamepadStateTracker> gamePadStates, const KeyboardStateTracker& keyboardState, const MouseStateTracker& ) {
         bool b    = false;
         bool back = false;

         for ( auto& gamePadState: gamePadStates )
         {
             b    = b || gamePadState.b == ButtonState::Released;
             back = back || gamePadState.back == ButtonState::Released;
         }

         const bool esc = keyboardState.isKeyReleased( Key::Escape );

         return b || back || esc;
     } },
};

}  // namespace

void Input::update()
{
    Mouse::resetRelativeMotion();
    Touch::endFrame();

    for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
        g_GamepadStateTrackers[i].update( Gamepad::getState( i ) );

    g_KeyboardStateTracker.update( Keyboard::getState() );
    g_MouseStateTracker.update( Mouse::getState() );
    g_TouchStateTracker.update( Touch::getState() );
}

float Input::getAxis( std::string_view axisName )
{
    if ( const auto& iter = g_AxisMap.find( std::string( axisName ) ); iter != g_AxisMap.end() )
    {
        return iter->second( g_GamepadStateTrackers, g_KeyboardStateTracker, g_MouseStateTracker );
    }

    return 0.0f;
}

bool Input::getButton( std::string_view buttonName )
{
    // First check if there is a matching button mapping.
    if ( const auto& iter = g_ButtonMap.find( std::string( buttonName ) ); iter != g_ButtonMap.end() )
    {
        return iter->second( g_GamepadStateTrackers, g_KeyboardStateTracker, g_MouseStateTracker );
    }

    // Then check if there is a matching axis mapping.
    if ( const auto& iter = g_AxisMap.find( std::string( buttonName ) ); iter != g_AxisMap.end() )
    {
        return iter->second( g_GamepadStateTrackers, g_KeyboardStateTracker, g_MouseStateTracker ) > 0.0f;
    }

    return false;
}

bool Input::getButtonDown( std::string_view buttonName )
{
    // First check the button down map
    if ( const auto& iter = g_ButtonDownMap.find( std::string( buttonName ) ); iter != g_ButtonDownMap.end() )
    {
        return iter->second( g_GamepadStateTrackers, g_KeyboardStateTracker, g_MouseStateTracker );
    }

    // Then check to see if there is a matching key in the key map:
    if ( const auto& iter = g_KeyMap.find( std::string( buttonName ) ); iter != g_KeyMap.end() )
    {
        return g_KeyboardStateTracker.isKeyPressed( iter->second );
    }

    return false;
}

bool Input::getButtonUp( std::string_view buttonName )
{
    // First check the button up map
    if ( const auto& iter = g_ButtonUpMap.find( std::string( buttonName ) ); iter != g_ButtonUpMap.end() )
    {
        return iter->second( g_GamepadStateTrackers, g_KeyboardStateTracker, g_MouseStateTracker );
    }

    // Then check to see if there is a matching key in the key map:
    if ( const auto& iter = g_KeyMap.find( std::string( buttonName ) ); iter != g_KeyMap.end() )
    {
        return g_KeyboardStateTracker.isKeyReleased( iter->second );
    }

    return false;
}

bool Input::getKey( std::string_view keyName )
{
    // First check to see if there is a matching key in the key map:
    if ( const auto& iter = g_KeyMap.find( std::string( keyName ) ); iter != g_KeyMap.end() )
    {
        return g_KeyboardStateTracker.getLastState().isKeyDown( iter->second );
    }

    return false;
}

bool Input::getKeyDown( std::string_view keyName )
{
    // First check to see if there is a matching key in the key map:
    if ( const auto& iter = g_KeyMap.find( std::string( keyName ) ); iter != g_KeyMap.end() )
    {
        return g_KeyboardStateTracker.isKeyPressed( iter->second );
    }

    return false;
}

bool Input::getKeyUp( std::string_view keyName )
{
    // First check to see if there is a matching button in the key map:
    if ( const auto& iter = g_KeyMap.find( std::string( keyName ) ); iter != g_KeyMap.end() )
    {
        return g_KeyboardStateTracker.isKeyReleased( iter->second );
    }

    return false;
}

bool Input::getKey( Key key )
{
    return g_KeyboardStateTracker.getLastState().isKeyDown( key );
}

bool Input::getKeyDown( Key key )
{
    return g_KeyboardStateTracker.isKeyPressed( key );
}

bool Input::getKeyUp( Key key )
{
    return g_KeyboardStateTracker.isKeyReleased( key );
}

bool Input::getMouseButton( Button button )
{
    switch ( button )
    {
    case Button::Left:
        return g_MouseStateTracker.getLastState().leftButton;
    case Button::Right:
        return g_MouseStateTracker.getLastState().rightButton;
    case Button::Middle:
        return g_MouseStateTracker.getLastState().middleButton;
    case Button::XButton1:
        return g_MouseStateTracker.getLastState().xButton1;
    case Button::XButton2:
        return g_MouseStateTracker.getLastState().xButton2;
    default:
        return false;
    }
}

bool Input::getMouseButtonDown( Button button )
{
    switch ( button )
    {
    case Button::Left:
        return g_MouseStateTracker.leftButton == ButtonState::Pressed;
    case Button::Right:
        return g_MouseStateTracker.rightButton == ButtonState::Pressed;
    case Button::Middle:
        return g_MouseStateTracker.middleButton == ButtonState::Pressed;
    case Button::XButton1:
        return g_MouseStateTracker.xButton1 == ButtonState::Pressed;
    case Button::XButton2:
        return g_MouseStateTracker.xButton2 == ButtonState::Pressed;
    default:
        return false;
    }
}

bool Input::getMouseButtonUp( Button button )
{
    switch ( button )
    {
    case Button::Left:
        return g_MouseStateTracker.leftButton == ButtonState::Released;
    case Button::Right:
        return g_MouseStateTracker.rightButton == ButtonState::Released;
    case Button::Middle:
        return g_MouseStateTracker.middleButton == ButtonState::Released;
    case Button::XButton1:
        return g_MouseStateTracker.xButton1 == ButtonState::Released;
    case Button::XButton2:
        return g_MouseStateTracker.xButton2 == ButtonState::Released;
    default:
        return false;
    }
}

float Input::getMouseX()
{
    return g_MouseStateTracker.getLastState().x;
}

float Input::getMouseY()
{
    return g_MouseStateTracker.getLastState().y;
}

float Input::getMouseMoveX()
{
    return g_MouseStateTracker.x;
}

float Input::getMouseMoveY()
{
    return g_MouseStateTracker.y;
}

void Input::addAxisCallback( std::string_view axisName, AxisCallback callback )
{
    g_AxisMap[std::string( axisName )] = std::move( callback );
}

void Input::addButtonCallback( std::string_view buttonName, ButtonCallback callback )
{
    g_ButtonMap[std::string( buttonName )] = std::move( callback );
}

void Input::addButtonDownCallback( std::string_view buttonName, ButtonCallback callback )
{
    g_ButtonDownMap[std::string( buttonName )] = std::move( callback );
}

void Input::addButtonUpCallback( std::string_view buttonName, ButtonCallback callback )
{
    g_ButtonUpMap[std::string( buttonName )] = std::move( callback );
}
