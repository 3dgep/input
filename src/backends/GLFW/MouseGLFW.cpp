#include <input/Mouse.hpp>

#include <GLFW/glfw3.h>

#include <cassert>
#include <mutex>

using namespace input;

// Register these callbacks in your application after creating the window:
// Forward-declare mouse callbacks.
// void Mouse_ScrollCallback(GLFWwindow*, double, double);
// void Mouse_CursorPosCallback(GLFWwindow*, double, double);
// void Mouse_ButtonCallback(GLFWwindow*, int, int, int)
//
// glfwSetScrollCallback(window, Mouse_ScrollCallback);
// glfwSetCursorPosCallback(window, Mouse_CursorPosCallback);
// glfwSetMouseButtonCallback(window, Mouse_ButtonCallback);

class MouseGLFW
{
public:
    static MouseGLFW& get()
    {
        static MouseGLFW instance;
        return instance;
    }

    Mouse::State getState()
    {
        std::lock_guard lock( m_Mutex );

        m_State.positionMode = m_Mode;

        return m_State;
    }

    void resetScrollWheelValue() noexcept
    {
        std::lock_guard lock( m_Mutex );
        m_State.scrollWheelValue = 0;
    }

    void setMode( Mouse::Mode mode )
    {
        std::lock_guard lock( m_Mutex );
        if ( m_Mode == mode )
            return;

        m_Mode = mode;
        if ( mode == Mouse::Mode::Relative )
        {
            assert( m_Window != nullptr );

            m_RelativeX = 0;
            m_RelativeY = 0;
            glfwSetInputMode( m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED | GLFW_CURSOR_CAPTURED );
        }
        else
        {
            glfwSetInputMode( m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
        }
    }

    void resetRelativeMotion() noexcept
    {
        std::lock_guard lock( m_Mutex );
        if ( m_Mode == Mouse::Mode::Relative )
        {
            m_RelativeX = 0;
            m_RelativeY = 0;
        }
    }

    bool isConnected() const
    {
        return true;
    }

    bool isVisible() const noexcept
    {
        return glfwGetInputMode( m_Window, GLFW_CURSOR ) == GLFW_CURSOR_NORMAL;
    }

    void setVisible( bool visible )
    {
        glfwSetInputMode( m_Window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN );
    }

    void setWindow( GLFWwindow* window )
    {
        std::lock_guard lock( m_Mutex );
        m_Window = window;
    }

    MouseGLFW( const MouseGLFW& )            = delete;
    MouseGLFW( MouseGLFW&& )                 = delete;
    MouseGLFW& operator=( const MouseGLFW& ) = delete;
    MouseGLFW& operator=( MouseGLFW&& )      = delete;

private:
    friend void Mouse_ScrollCallback( GLFWwindow* /*window*/, double /*xoffset*/, double yoffset );
    friend void Mouse_CursorPosCallback( GLFWwindow* /*window*/, double x, double y );
    friend void Mouse_ButtonCallback( GLFWwindow* /*window*/, int button, int action, int /*mods*/ );

    MouseGLFW()  = default;
    ~MouseGLFW() = default;

    mutable std::mutex m_Mutex;

    Mouse::Mode  m_Mode      = Mouse::Mode::Absolute;
    int          m_RelativeX = 0;
    int          m_RelativeY = 0;
    double       m_LastX     = 0;
    double       m_LastY     = 0;
    GLFWwindow*  m_Window    = nullptr;
    Mouse::State m_State {};
};

// GLFW scroll callback function (outside the class)
void Mouse_ScrollCallback( GLFWwindow* /*window*/, double /*xoffset*/, double yoffset )
{
    auto&           impl = MouseGLFW::get();
    std::lock_guard lock( impl.m_Mutex );
    impl.m_State.scrollWheelValue += static_cast<int>( yoffset * 120 );  // 120 is Win32/DirectX standard
}

// GLFW cursor position callback function (outside the class)
void Mouse_CursorPosCallback( GLFWwindow* /*window*/, double x, double y )
{
    auto&           impl = MouseGLFW::get();
    std::lock_guard lock( impl.m_Mutex );
    if ( impl.m_Mode == Mouse::Mode::Relative )
    {
        impl.m_RelativeX += static_cast<int>( x - impl.m_LastX );
        impl.m_RelativeY += static_cast<int>( y - impl.m_LastY );
        impl.m_State.x = impl.m_RelativeX;
        impl.m_State.y = impl.m_RelativeY;
    }
    else
    {
        impl.m_State.x = static_cast<int>( x );
        impl.m_State.y = static_cast<int>( y );
    }
    impl.m_LastX = x;
    impl.m_LastY = y;
}

// GLFW mouse button callback function (outside the class)
void Mouse_ButtonCallback( GLFWwindow* /*window*/, int button, int action, int /*mods*/ )
{
    auto&           impl = MouseGLFW::get();
    std::lock_guard lock( impl.m_Mutex );
    bool            pressed = ( action == GLFW_PRESS );

    switch ( button )
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        impl.m_State.leftButton = pressed;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        impl.m_State.rightButton = pressed;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        impl.m_State.middleButton = pressed;
        break;
    case GLFW_MOUSE_BUTTON_4:
        impl.m_State.xButton1 = pressed;
        break;
    case GLFW_MOUSE_BUTTON_5:
        impl.m_State.xButton2 = pressed;
        break;
    default:
        break;
    }
}

// Bridge to Mouse interface
Mouse::State Mouse::getState() const
{
    return MouseGLFW::get().getState();
}

void Mouse::resetScrollWheelValue() noexcept
{
    MouseGLFW::get().resetScrollWheelValue();
}

void Mouse::setMode( Mode mode )
{
    MouseGLFW::get().setMode( mode );
}

void Mouse::resetRelativeMotion() noexcept
{
    MouseGLFW::get().resetRelativeMotion();
}

bool Mouse::isConnected() const
{
    return MouseGLFW::get().isConnected();
}

bool Mouse::isVisible() const noexcept
{
    return MouseGLFW::get().isVisible();
}

void Mouse::setVisible( bool visible )
{
    MouseGLFW::get().setVisible( visible );
}

void Mouse::setWindow( void* window )
{
    MouseGLFW::get().setWindow( static_cast<GLFWwindow*>( window ) );
}
