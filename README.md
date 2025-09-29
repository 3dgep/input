![alt text](doc/Input_banner.png)

# Input

Input is a cross-platform C++ library for handling gamepad, keyboard, and mouse input. It provides a unified API for querying input states and supports multiple backends, including SDL2, SDL3, GDK, GLFW, and Win32.

## Features

- Unified input API for gamepad, keyboard, and mouse
- Multiple backend support (SDL2, SDL3, GDK, GLFW, Win32)
- Easy integration with CMake
- Sample applications included

## Getting Started

### Prerequisites

- CMake 3.12 or higher
- C++20 compatible compiler
- Supported backends (SDL2, SDL3, GDK, GLFW, Win32)

### Building

Clone the repository and configure the build options in CMake:

```sh
git clone https://github.com/3dgep/input.git
cd input
cmake -B build -DINPUT_USE_SDL3=ON -DINPUT_BUILD_SAMPLES=ON
cmake --build build
```

You can enable/disable backends using the following CMake options:

| CMake Option          | Description                                                                                        |
| --------------------- | -------------------------------------------------------------------------------------------------- |
| `INPUT_USE_SDL2`      | Build the input::SDL2 backend. SDL2 will be fetched if it is not already included in your project. |
| `INPUT_USE_SDL3`      | Build the input::SDL3 backend. SDL3 will be fetched if it is not already included in your project. |
| `INPUT_USE_GLFW`      | Build the GLFW backend. GLFW will be fetched if it is not already included in your project.          |
| `INPUT_USE_GDK`       | Build the input::GDK backend. Requires Windows Game Development Toolkit.                           |
| `INPUT_USE_WIN32`     | Build the Win32 backend. Only available if building for Windows.                                   |
| `INPUT_BUILD_SAMPLES` | Build samples. Only samples for enabled backends will be built.                                    |

For each enabled backed, there is a matching CMake target which you can add to your own targets using [target_link_libraries](https://cmake.org/cmake/help/latest/command/target_link_libraries.html).

For example, if you are creating an SDL3 application, make sure you enable the `INPUT_USE_SDL3` option in CMake, then add the `input::SDL3` target to your own application (or library):

```cmake
target_link_libraries( MySDL3App PUBLIC input::SDL3 )
```

Optionally, you can just copy the contents of the [inc](inc) folder into your project's include folder and the source files in the [src](src) folder *and* **one** of the folders in the [backends](src/backends) folder and compile it with your project.

### Basic Usage

Include the headers from [`inc`](inc) folder and link against the appropriate backend library. Example:

```cpp
#include <input/Input.hpp>

void main()
{
    // Hook up any special callback functions, depending on the backend (see below).

    // You can create special action handlers for input:
    // Controls for the left paddle.
    input::Input::addAxisCallback( "Left Paddle", []( 
        std::span<const input::GamepadStateTracker> gamepadStates, 
        const input::KeyboardStateTracker& keyboardState, 
        const input::MouseStateTracker& mouseState ) 
    {
        float leftY  = gamepadStates[0].getLastState().thumbSticks.leftY;
        float rightY = gamepadStates[0].getLastState().thumbSticks.rightY;

        float w = keyboardState.isKeyDown( input::Keyboard::Key::W ) ? 1.0f : 0.0f;
        float s = keyboardState.isKeyDown( input::Keyboard::Key::S ) ? 1.0f : 0.0f;

        return std::clamp( s - w + leftY + rightY, -1.0f, 1.0f );
    } );

    // Controls for the right paddle.
    input::Input::addAxisCallback( "Right Paddle", []( 
        std::span<const input::GamepadStateTracker> gamepadStates, 
        const input::KeyboardStateTracker& keyboardState, 
        const input::MouseStateTracker& mouseState ) 
    {
        float leftY  = gamepadStates[1].getLastState().thumbSticks.leftY;
        float rightY = gamepadStates[1].getLastState().thumbSticks.rightY;

        float up   = keyboardState.isKeyDown( input::Keyboard::Key::Up ) ? 1.0f : 0.0f;
        float down = keyboardState.isKeyDown( input::Keyboard::Key::Down ) ? 1.0f : 0.0f;

        return std::clamp( down - up + leftY + rightY, -1.0f, 1.0f );
    } );
}

void updateInput() {
   input::Input::update(); // Call this once per frame to update the mouse, keyboard, and gamepad states.

    float leftPaddle = input::Input::getAxis("Left Paddle"); // Invoke the "Left Paddle" action.
    float rightPaddle  = input::Input::getAxis("Right Paddle"); // Invoke the "Right Paddle" action.
    if( input::Input::getButton("Jump") ) // There are also some "default" actions you can just use in your game.
    {
        doJump();
    }
}
```

## Backends

Some backends require certain functions provided by the input library to be "hooked" into your applications message loop.

### Win32

The Win32 backend requires that you call following functions in your application's message handler:

```cpp
// Forward declare callback functions.
void Keyboard_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    // Keyboard callback.
    Keyboard_ProcessMessage( msg, wParam, lParam );
    // Mouse callback.
    Mouse_ProcessMessage( msg, wParam, lParam );
    // The rest of your WndProc function...
}
```

The `Gamepad` does not require any special handling for Win32 applications.

See the [Win32 sample](samples/Win32/main.cpp) for details.

### Microsoft Game Development Kit

See [Game Development Kit (GDK)](https://learn.microsoft.com/en-us/gaming/gdk/) for more information.

The Microsoft Game Development Toolkit only requires special handling for the mouse:

```cpp
// Forward declare callback functions.
void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    // Mouse callback.
    Mouse_ProcessMessage( msg, wParam, lParam );
    // Keyboard callback.
    // Keyboard_ProcessMessage( msg, wParam, lParam ); // You can call this when using GDK, but it's not required.

    // The rest of your WndProc function...
}
```

The `Gamepad` and `Keyboard` don't require any special handling.

See the [GDK sample](samples/GDK/main.cpp) for details.

### GLFW

See [www.glfw.org](https://www.glfw.org/) for more information on GLFW.

GLFW requires special functions to be registered with GLFW's callback function mechanism:

```cpp
// Forward-declare callback functions.
void Mouse_ScrollCallback( GLFWwindow*, double, double );
void Mouse_CursorPosCallback( GLFWwindow*, double, double );
void Mouse_ButtonCallback( GLFWwindow*, int, int, int );
void Keyboard_Callback( GLFWwindow* , int , int , int, int );

int main()
{
    // Initialize and create a GLFWwindow...

    // Register callback functions.
    glfwSetScrollCallback( g_pWindow, Mouse_ScrollCallback );
    glfwSetCursorPosCallback( g_pWindow, Mouse_CursorPosCallback );
    glfwSetMouseButtonCallback( g_pWindow, Mouse_ButtonCallback );
    glfwSetKeyCallback( g_pWindow, Keyboard_Callback );

}
```

The `Gamepad` doesn't require any special handling with GLFW.

### SDL2 & SDL3

See [www.libsdl.org](https://www.libsdl.org/) for more information on SDL2 & SDL3.

The input library will automatically hook into the SDL event loop using [`SDL_AddEventWatch`](https://wiki.libsdl.org/SDL2/SDL_AddEventWatch) (SDL2) or [`SDL_AddEventWatch`](https://wiki.libsdl.org/SDL3/SDL_AddEventWatch) (SDL3) so no special handling is required when using these backends.

The `GamepadSDL2` and `GamepadSDL3` classes will also enable the `SDL_INIT_GAMECONTROLLER` (SDL2) or the `SDL_INIT_GAMEPAD` (SDL3) subsystem in case you forget 😉.

See [SDL2](samples/SDL2/main.cpp) or [SDL3](samples/SDL3/main.cpp) samples for more detailed information.

## Samples

Sample applications are available in the [`samples`](samples) directory. Enable `INPUT_BUILD_SAMPLES` to build them.

![alt text](doc/sample_GDK.png)

## Gamepad

The [`Gamepad`](inc/input/Gamepad.hpp) class can be used as a singleton class, or if you find it more convenient to create an instance of a `Gamepad` object with an associated player ID.

The `Gampad` class provides the following functions:

- `static Gamepad::State getState( int playerId, DeadZoneMode deadZoneMode )`: Get the current gamepad state for the player at index `playerId`.
- `static bool setVibration( int playerId, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )`: Set the controller vibration (rumble). Note: The GLFW backend does not support controller rumble.
- `static void suspend()`: Call this function when your game window loses focus.
- `static void resume()`: Call this function when your game window gains focus.

Although you should prefer to use input actions, you can read the gamepad state directly by using the `Gamepad::getState` function:

```cpp
void updateGamepads()
{
    for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
    {
        // Get the current state of the gamepad, apply independent axis deadzone mode.
        Gamepad::State state = Gamepad::getState( i, Gamepad::DeadZone::IndependentAxis ); 
        if ( state.connected )
        {
            if(state.buttons.a)
                // The a button is pressed on the controller.
            
            if(state.buttons.b)
                // The b button is pressed on the controller.
            
            // Similar for x, y, leftStick (click), rightStick (click), leftShoulder, rightShoulder, back/view, and start/menu buttons.

            if(state.dPad.up)
                // The D-pad is pressed up.

            // Similar for down, left, right D-pad buttons.

            if(state.thumbSticks.leftX > 0.5f) // Analog stick
                // The left analog stick is pushed to the right.
            
            // Similar for leftY, rightX, and rightY analog sticks.

            if(state.triggers.left > 0.5f) // Left trigger
                // The left triggers is pushed 50%

            // Similar for the right trigger.
            
        }
    }
}
```

The analog sticks will have deadzone applied according to the recommended deadzone for the controller (currently values below 0.24 are clamped to 0). There are two deadzone modes:

- `IndependentAxis`: Apply deadzone values to the X, and Y axis independently.
- `Circular`: Apply deadzone based on the radial distance from the center point.

Currently the deazone for the thumbsticks is currently not configurable. The recommended value for XBox controllers (0.24) is used in all backends.

See the [Gamepad.hpp](inc/input/Gamepad.hpp) file for more information on the layout of the `Gamepad::State` structure.

## GamepadStateTracker

The `GamepadStateTracker` class is used to check if a button was "just pressed" or "just released" this frame. If you want to react to "pressed" and "released" states, you can do this in your controller logic:

```cpp
// Define a GamepadStateTracker for each gamepad you want to get pressed/released key states.
GampadStateTracker gamepadStateTrackers[Gamepad::MAX_PLAYER_COUNT];

void updateGamepads()
{
   
    for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
    {
        // Get the current state of the gamepad, apply independent axis deadzone mode.
        Gamepad::State state = Gamepad::getState( i, Gamepad::DeadZone::IndependentAxis ); 

        // Update the gamepadStateTracker once per frame!
        gamepadStateTrackers[i].update( state );

        if ( state.connected )
        {
            if(gamepadStateTrackers[i].a == ButtonState::Pressed)
                // The a button was pressed this frame.
            
            if(gamepadStateTrackers[i].b == ButtonState::Released)
                // The b button was released this frame.
            
            // Similar for x, y, leftStick (click), rightStick (click), 
            // leftShoulder, rightShoulder, back/view, start/menu,
            // dPadUp, dPadDown, dPadLeft, dPadRight, 
            // leftTrigger, rightTrigger (with a threshold of 0.5 to emulate buttons)
            // and left/rightStickUp/Down/Left/Right (with a threshold of -0.5/0.5 to emulate buttons)
        }
    }
}
```

If a button is pressed or released in the current frame, the input system should report that regardless to how many times the input state is queried each frame. For this reason, it is important that each instance of the `GamepadStateTracker` is only updated once per frame. Multiple calls to `gamepadStateTracker.a` in the same frame should return the same up/held/pressed/released state.

You can also get the last state that the `GamepadStateTracker` was updated with using `GamepadStateTracker::getLastState`, but you can only determine if a button is up or down (held), but not if it was pressed/released this frame.

## Mouse

## License

This project is licensed under the MIT License. See: [LICENSE](LICENSE).
