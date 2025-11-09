#include <input/Touch.hpp>

using namespace input;

//======================================================================================
// GLFW implementation - Stub (GLFW has limited touch support)
//======================================================================================

//
// GLFW does not provide direct touch input API.
// Touch events may be simulated as mouse events on some platforms.
// This is a stub implementation that returns no touch input.
//

namespace input::Touch
{

State getState()
{
    // Return empty state - no touch support
    State state {};
    return state;
}

void endFrame()
{
    // GLFW does not support touch input.
}

bool isSupported()
{
    // GLFW does not support touch input directly
    return false;
}

int getDeviceCount()
{
    // No touch devices available
    return 0;
}

}  // namespace input::Touch
