#include <input/Mouse.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>
#include <memory>
#include <system_error>

using namespace input;

void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

//======================================================================================
// Win32 desktop implementation
//======================================================================================

//
// For a Win32 desktop application, in your window setup be sure to call this method:
//
// m_mouse->SetWindow(hwnd);
//
// And call this function from your Window Message Procedure
//
// void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
// 
// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_ACTIVATE:
//     case WM_ACTIVATEAPP:
//     case WM_INPUT:
//     case WM_MOUSEMOVE:
//     case WM_LBUTTONDOWN:
//     case WM_LBUTTONUP:
//     case WM_RBUTTONDOWN:
//     case WM_RBUTTONUP:
//     case WM_MBUTTONDOWN:
//     case WM_MBUTTONUP:
//     case WM_MOUSEWHEEL:
//     case WM_XBUTTONDOWN:
//     case WM_XBUTTONUP:
//     case WM_MOUSEHOVER:
//         Mouse_ProcessMessage(message, wParam, lParam);
//         break;
//
//     }
// }
//

namespace
{
struct handle_closer
{
    void operator()( HANDLE h ) noexcept
    {
        if ( h )
            CloseHandle( h );
    }
};

using ScopedHandle = std::unique_ptr<void, handle_closer>;

}  // namespace

// Source (September 15, 2025): https://github.com/microsoft/DirectXTK/blob/main/Src/Mouse.cpp
class MouseWin32
{
public:
    static MouseWin32& get()
    {
        static MouseWin32 mouseWin32;
        return mouseWin32;
    }

    Mouse::State getState()
    {
        Mouse::State state = m_State;
        state.positionMode = m_Mode;

        DWORD result = WaitForSingleObjectEx( m_ScrollWheelValue.get(), 0, FALSE );
        if ( result == WAIT_FAILED )
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "WaitForSingleObjectEx" );

        if ( result == WAIT_OBJECT_0 )
        {
            state.scrollWheelValue = 0;
        }

        if ( state.positionMode == Mouse::Mode::Relative )
        {
            result = WaitForSingleObjectEx( m_RelativeRead.get(), 0, FALSE );

            if ( result == WAIT_FAILED )
                throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "WaitForSingleObjectEx" );

            if ( result == WAIT_OBJECT_0 )
            {
                state.x = state.y = 0;
            }
            else
            {
                SetEvent( m_RelativeRead.get() );
            }

            if ( m_AutoReset )
            {
                m_State.x = m_State.y = 0;
            }
        }

        return state;
    }

    void resetScrollWheelValue() noexcept
    {
        SetEvent( m_ScrollWheelValue.get() );
    }

    void setMode( Mouse::Mode mode )
    {
        if ( m_Mode == mode )
            return;

        SetEvent( ( mode == Mouse::Mode::Absolute ) ? m_AbsoluteMode.get() : m_RelativeMode.get() );

        assert( m_Window != nullptr );

        // Send a WM_HOVER as a way to 'kick' the message processing even if the mouse is still.
        TRACKMOUSEEVENT tme;
        tme.cbSize      = sizeof( tme );
        tme.dwFlags     = TME_HOVER;
        tme.hwndTrack   = m_Window;
        tme.dwHoverTime = 1;
        if ( !TrackMouseEvent( &tme ) )
        {
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "TrackMouseEvent" );
        }
    }

    void endOfInputFrame()
    {
        m_AutoReset = false;

        if ( m_Mode == Mouse::Mode::Relative )
        {
            m_State.x = m_State.y = 0;
        }
    }

    bool isConnected() const noexcept
    {
        return GetSystemMetrics( SM_MOUSEPRESENT ) != 0;
    }

    bool isVisible() const noexcept
    {
        if ( m_Mode == Mouse::Mode::Relative )
            return false;

        CURSORINFO info = { sizeof( CURSORINFO ), 0, nullptr, {} };
        if ( !GetCursorInfo( &info ) )
            return false;

        return ( info.flags & CURSOR_SHOWING ) != 0;
    }

    void setVisible( bool visible )
    {
        if ( m_Mode == Mouse::Mode::Relative )
            return;

        CURSORINFO info = { sizeof( CURSORINFO ), 0, nullptr, {} };
        if ( !GetCursorInfo( &info ) )
        {
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "GetCursorInfo" );
        }

        const bool isVisible = ( info.flags & CURSOR_SHOWING ) != 0;
        if ( isVisible != visible )
        {
            ShowCursor( visible );
        }
    }

    void setWindow( HWND window )
    {
        if ( m_Window == window )
            return;

        assert( window != nullptr );

        RAWINPUTDEVICE Rid;
        Rid.usUsagePage = 0x1 /* HID_USAGE_PAGE_GENERIC */;
        Rid.usUsage     = 0x2 /* HID_USAGE_GENERIC_MOUSE */;
        Rid.dwFlags     = RIDEV_INPUTSINK;
        Rid.hwndTarget  = window;
        if ( !RegisterRawInputDevices( &Rid, 1, sizeof( RAWINPUTDEVICE ) ) )
        {
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "RegisterRawInputDevices" );
        }

        m_Window = window;
    }

    MouseWin32( const MouseWin32& )            = delete;
    MouseWin32( MouseWin32&& )                 = delete;
    MouseWin32& operator=( const MouseWin32& ) = delete;
    MouseWin32& operator=( MouseWin32&& )      = delete;

private:
    MouseWin32()
    {
        m_ScrollWheelValue.reset( CreateEventEx( nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE ) );
        m_RelativeRead.reset( CreateEventEx( nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE ) );
        m_AbsoluteMode.reset( CreateEventEx( nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE ) );
        m_RelativeRead.reset( CreateEventEx( nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE ) );

        if ( !m_ScrollWheelValue || !m_RelativeRead || !m_AbsoluteMode || !m_RelativeRead )
        {
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "CreateEventEx" );
        }
    }

    ~MouseWin32() = default;

    friend void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

    void clipToWindow() noexcept
    {
        assert( m_Window != nullptr );

        RECT rect   = {};
        std::ignore = GetClientRect( m_Window, &rect );

        POINT ul;
        ul.x = rect.left;
        ul.y = rect.top;

        POINT lr;
        lr.x = rect.right;
        lr.y = rect.bottom;

        std::ignore = MapWindowPoints( m_Window, nullptr, &ul, 1 );
        std::ignore = MapWindowPoints( m_Window, nullptr, &lr, 1 );

        rect.left = ul.x;
        rect.top  = ul.y;

        rect.right  = lr.x;
        rect.bottom = lr.y;

        ClipCursor( &rect );
    }

    Mouse::State m_State {};

    HWND        m_Window { nullptr };
    Mouse::Mode m_Mode { Mouse::Mode::Absolute };

    ScopedHandle m_ScrollWheelValue;
    ScopedHandle m_RelativeRead;
    ScopedHandle m_AbsoluteMode;
    ScopedHandle m_RelativeMode;

    int m_LastX { 0 };
    int m_LastY { 0 };
    int m_RelativeX { INT32_MAX };
    int m_RelativeY { INT32_MAX };

    bool m_InFocus { true };
    bool m_AutoReset { true };
};

void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam )
{
    auto& impl = MouseWin32::get();

    // First handle any pending scroll wheel reset event.
    switch ( WaitForSingleObjectEx( impl.m_ScrollWheelValue.get(), 0, FALSE ) )
    {
    default:
    case WAIT_TIMEOUT:
        break;

    case WAIT_OBJECT_0:
        impl.m_State.scrollWheelValue = 0;
        ResetEvent( impl.m_ScrollWheelValue.get() );
        break;

    case WAIT_FAILED:
        throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "WaitForMultipleObjectsEx" );
    }

    // Next handle mode change events.
    HANDLE events[2] = { impl.m_AbsoluteMode.get(), impl.m_RelativeMode.get() };
    switch ( WaitForMultipleObjectsEx( static_cast<DWORD>( std::size( events ) ), events, FALSE, 0, FALSE ) )
    {
    default:
    case WAIT_TIMEOUT:
        break;

    case WAIT_OBJECT_0:
    {
        impl.m_Mode = Mouse::Mode::Absolute;
        ClipCursor( nullptr );

        POINT point;
        point.x = impl.m_LastX;
        point.y = impl.m_LastY;

        // We show the cursor before moving it to support Remote Desktop
        ShowCursor( TRUE );

        if ( MapWindowPoints( impl.m_Window, nullptr, &point, 1 ) )
        {
            SetCursorPos( point.x, point.y );
        }
        impl.m_State.x = impl.m_LastX;
        impl.m_State.y = impl.m_LastY;
    }
    break;

    case ( WAIT_OBJECT_0 + 1 ):
    {
        ResetEvent( impl.m_RelativeRead.get() );

        impl.m_Mode    = Mouse::Mode::Relative;
        impl.m_State.x = impl.m_State.y = 0;
        impl.m_RelativeX                = INT32_MAX;
        impl.m_RelativeY                = INT32_MAX;

        ShowCursor( FALSE );

        impl.clipToWindow();
    }
    break;

    case WAIT_FAILED:
        throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "WaitForMultipleObjectsEx" );
    }

    switch ( message )
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        if ( wParam )
        {
            impl.m_InFocus = true;

            if ( impl.m_Mode == Mouse::Mode::Relative )
            {
                impl.m_State.x = impl.m_State.y = 0;

                ShowCursor( FALSE );

                impl.clipToWindow();
            }
        }
        else
        {
            const int scrollWheel = impl.m_State.scrollWheelValue;
            memset( &impl.m_State, 0, sizeof( Mouse::State ) );
            impl.m_State.scrollWheelValue = scrollWheel;

            if ( impl.m_Mode == Mouse::Mode::Relative )
            {
                ClipCursor( nullptr );
            }

            impl.m_InFocus = false;
        }
        return;

    case WM_INPUT:
        if ( impl.m_InFocus && impl.m_Mode == Mouse::Mode::Relative )
        {
            RAWINPUT raw;
            UINT     rawSize = sizeof( raw );

            const UINT resultData = GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT, &raw, &rawSize, sizeof( RAWINPUTHEADER ) );
            if ( resultData == static_cast<UINT>( -1 ) )
            {
                throw std::runtime_error( "GetRawInputData" );
            }

            if ( raw.header.dwType == RIM_TYPEMOUSE )
            {
                if ( !( raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE ) )
                {
                    impl.m_State.x += raw.data.mouse.lLastX;
                    impl.m_State.y += raw.data.mouse.lLastY;

                    ResetEvent( impl.m_RelativeRead.get() );
                }
                else if ( raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP )
                {
                    // This is used to make Remote Desktop sessions work
                    const int width  = GetSystemMetrics( SM_CXVIRTUALSCREEN );
                    const int height = GetSystemMetrics( SM_CYVIRTUALSCREEN );

                    const auto x = static_cast<int>( ( static_cast<float>( raw.data.mouse.lLastX ) / 65535.0f ) * static_cast<float>( width ) );
                    const auto y = static_cast<int>( ( static_cast<float>( raw.data.mouse.lLastY ) / 65535.0f ) * static_cast<float>( height ) );

                    if ( impl.m_RelativeX == INT32_MAX )
                    {
                        impl.m_State.x = impl.m_State.y = 0;
                    }
                    else
                    {
                        impl.m_State.x = x - impl.m_RelativeX;
                        impl.m_State.y = y - impl.m_RelativeY;
                    }

                    impl.m_RelativeX = x;
                    impl.m_RelativeY = y;

                    ResetEvent( impl.m_RelativeRead.get() );
                }
            }
        }
        return;

    case WM_MOUSEMOVE:
        break;

    case WM_LBUTTONDOWN:
        impl.m_State.leftButton = true;
        break;

    case WM_LBUTTONUP:
        impl.m_State.leftButton = false;
        break;

    case WM_RBUTTONDOWN:
        impl.m_State.rightButton = true;
        break;

    case WM_RBUTTONUP:
        impl.m_State.rightButton = false;
        break;

    case WM_MBUTTONDOWN:
        impl.m_State.middleButton = true;
        break;

    case WM_MBUTTONUP:
        impl.m_State.middleButton = false;
        break;

    case WM_MOUSEWHEEL:
        impl.m_State.scrollWheelValue += GET_WHEEL_DELTA_WPARAM( wParam );
        return;

    case WM_XBUTTONDOWN:
        switch ( GET_XBUTTON_WPARAM( wParam ) )
        {
        case XBUTTON1:
            impl.m_State.xButton1 = true;
            break;

        case XBUTTON2:
            impl.m_State.xButton2 = true;
            break;

        default:
            break;
        }
        break;

    case WM_XBUTTONUP:
        switch ( GET_XBUTTON_WPARAM( wParam ) )
        {
        case XBUTTON1:
            impl.m_State.xButton1 = false;
            break;

        case XBUTTON2:
            impl.m_State.xButton2 = false;
            break;

        default:
            break;
        }
        break;

    case WM_MOUSEHOVER:
        break;

    default:
        // Not a mouse message, so exit
        return;
    }

    if ( impl.m_Mode == Mouse::Mode::Absolute )
    {
        // All mouse messages provide a new pointer position
        const int xPos = static_cast<short>( LOWORD( lParam ) );  // GET_X_LPARAM(lParam);
        const int yPos = static_cast<short>( HIWORD( lParam ) );  // GET_Y_LPARAM(lParam);

        impl.m_State.x = impl.m_LastX = xPos;
        impl.m_State.y = impl.m_LastY = yPos;
    }
}

Mouse::State Mouse::getState() const
{
    return MouseWin32::get().getState();
}

void Mouse::resetScrollWheelValue() noexcept
{
    MouseWin32::get().resetScrollWheelValue();
}

void Mouse::setMode( Mode mode )
{
    MouseWin32::get().setMode( mode );
}

void Mouse::endOfInputFrame() noexcept
{
    MouseWin32::get().endOfInputFrame();
}

bool Mouse::isConnected() const
{
    return MouseWin32::get().isConnected();
}

bool Mouse::isVisible() const noexcept
{
    return MouseWin32::get().isVisible();
}

void Mouse::setVisible( bool visible )
{
    MouseWin32::get().setVisible( visible );
}
