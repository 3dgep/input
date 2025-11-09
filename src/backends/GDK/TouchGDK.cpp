#include <input/Touch.hpp>

using namespace input;

//======================================================================================
// GDK implementation - Stub (Xbox/Console platforms typically don't use touch)
//======================================================================================

//
// The Microsoft Game Development Kit (GDK) is primarily for Xbox consoles and Windows.
// Xbox consoles do not have touch screens, so this is a stub implementation.
// For Windows GDK builds, use the Win32 touch implementation instead if needed.
//

namespace input::Touch
{

State getState()
{
    // Return empty state - no touch support
    State state {};
    return state;
}

bool isSupported()
{
    // GDK targets typically don't support touch input
    return false;
}

int getDeviceCount()
{
    // No touch devices available
    return 0;
}

}  // namespace input::Touch
