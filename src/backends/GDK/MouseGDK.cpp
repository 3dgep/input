#include <input/Mouse.hpp>

#include <GameInput.h>
#include <wrl.h>

#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>

void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

//======================================================================================
// Win32 + GameInput implementation
//======================================================================================

//
// Call this static function from your Window Message Procedure
//
// extern void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
//
// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_ACTIVATE:
//     case WM_ACTIVATEAPP:
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
//         Mouse_ProcessMessage(message, wParam, lParam);
//         break;
//
//     }
// }
//

using namespace input;
using Microsoft::WRL::ComPtr;

namespace
{
struct handle_closer
{
    void operator()( HANDLE h ) const noexcept
    {
        if ( h )
            CloseHandle( h );
    }
};

using ScopedHandle = std::unique_ptr<void, handle_closer>;

}  // namespace

// Source (September 15, 2025): https://github.com/microsoft/DirectXTK/blob/main/Src/Mouse.cpp
class MouseGDK
{
public:
    static MouseGDK& get()
    {
        static MouseGDK mouseGDK;
        return mouseGDK;
    }

    Mouse::State getState() const
    {
        Mouse::State state = m_State;
        state.positionMode = m_Mode;

        DWORD result = WaitForSingleObjectEx( m_ScrollWheelValue.get(), 0, FALSE );
        if ( result == WAIT_FAILED )
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ) );

        if ( result == WAIT_OBJECT_0 )
        {
            m_ScrollWheelCurrent = 0;
        }

        if ( state.positionMode == Mouse::Mode::Relative )
        {
            state.x = state.y = 0;

            if ( m_GameInput )
            {
                ComPtr<IGameInputReading> reading;
                if ( SUCCEEDED( m_GameInput->GetCurrentReading( GameInputKindMouse, nullptr, reading.GetAddressOf() ) ) )
                {
                    GameInputMouseState mouseState;
                    if ( reading->GetMouseState( &mouseState ) )
                    {
                        state.leftButton   = ( mouseState.buttons & GameInputMouseLeftButton ) != 0;
                        state.middleButton = ( mouseState.buttons & GameInputMouseMiddleButton ) != 0;
                        state.rightButton  = ( mouseState.buttons & GameInputMouseRightButton ) != 0;
                        state.xButton1     = ( mouseState.buttons & GameInputMouseButton4 ) != 0;
                        state.xButton2     = ( mouseState.buttons & GameInputMouseButton5 ) != 0;

                        if ( m_RelativeX != INT64_MAX )
                        {
                            state.x         = static_cast<int>( mouseState.positionX - m_RelativeX );
                            state.y         = static_cast<int>( mouseState.positionY - m_RelativeY );
                            int scrollDelta = static_cast<int>( mouseState.wheelY - m_RelativeWheelY );
                            m_ScrollWheelCurrent += scrollDelta;
                        }

                        if ( m_AutoReset )
                        {
                            m_RelativeX = mouseState.positionX;
                            m_RelativeY = mouseState.positionY;
                        }

                        m_LastX          = mouseState.positionX;
                        m_LastY          = mouseState.positionY;
                        m_RelativeWheelY = mouseState.wheelY;
                    }
                }
            }
        }

        state.scrollWheelValue = m_ScrollWheelCurrent;

        return state;
    }

    void resetScrollWheelValue() noexcept
    {
        SetEvent( m_ScrollWheelValue.get() );
    }

    void setWindow( HWND window )
    {
        m_Window = window;
    }

    void setMode( Mouse::Mode mode )
    {
        if ( m_Mode == mode )
            return;

        m_Mode  = mode;
        m_LastX = m_RelativeX = INT64_MAX;
        m_LastY = m_RelativeY = INT64_MAX;
        m_RelativeWheelY      = INT64_MAX;

        if ( mode == Mouse::Mode::Relative )
        {
            ShowCursor( FALSE );
            ClipToWindow();
        }
        else
        {
            ShowCursor( TRUE );

#ifndef _GAMING_XBOX
            POINT point;
            point.x = m_State.x;
            point.y = m_State.y;

            if ( MapWindowPoints( m_Window, nullptr, &point, 1 ) )
            {
                SetCursorPos( point.x, point.y );
            }

            ClipCursor( nullptr );
#endif
        }
    }

    void endOfInputFrame()
    {
        m_AutoReset = false;

        if ( m_Mode == Mouse::Mode::Relative )
        {
            m_RelativeX = m_LastX;
            m_RelativeY = m_LastY;
        }
    }

    bool isConnected() const noexcept
    {
        return m_Connected > 0;
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

        bool isVisible = ( info.flags & CURSOR_SHOWING ) != 0;
        if ( isVisible != visible )
        {
            ShowCursor( visible );
        }
    }

    MouseGDK( const MouseGDK& )            = delete;
    MouseGDK( MouseGDK&& ) noexcept        = delete;
    MouseGDK& operator=( const MouseGDK& ) = delete;
    MouseGDK& operator=( MouseGDK&& )      = delete;

    Mouse::State m_State {};
    float        m_Scale     = 1.0f;
    uint32_t     m_Connected = 0;

private:
    friend void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

    static void CALLBACK OnGameInputDevice(
        _In_       GameInputCallbackToken,
        _In_ void* context,
        _In_ IGameInputDevice*,
        _In_                       uint64_t,
        _In_ GameInputDeviceStatus currentStatus,
        _In_ GameInputDeviceStatus previousStatus ) noexcept
    {
        auto impl = static_cast<MouseGDK*>( context );

        const bool wasConnected = ( previousStatus & GameInputDeviceConnected ) != 0;
        const bool isConnected  = ( currentStatus & GameInputDeviceConnected ) != 0;

        if ( isConnected && !wasConnected )
        {
            ++impl->m_Connected;
        }
        else if ( !isConnected && wasConnected && impl->m_Connected > 0 )
        {
            --impl->m_Connected;
        }
    }

    MouseGDK()
    {
        HRESULT hr = GameInputCreate( m_GameInput.GetAddressOf() );
        if ( SUCCEEDED( hr ) )
        {
            hr = m_GameInput->RegisterDeviceCallback(
                nullptr,
                GameInputKindMouse,
                GameInputDeviceConnected,
                GameInputBlockingEnumeration,
                this,
                OnGameInputDevice,
                &m_CallbackToken );

            if ( FAILED( hr ) )
            {
                throw std::runtime_error( std::format( "Failed to register mouse device callback: {:08X}", static_cast<unsigned int>( hr ) ) );
            }
        }
        else
        {
            throw std::runtime_error( std::format( "Failed to create GameInput: {:08X}", static_cast<unsigned int>( hr ) ) );
        }

        m_ScrollWheelValue.reset( CreateEventEx( nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE ) );
        if ( !m_ScrollWheelValue )
        {
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "CreateEventEx" );
        }
    }

    ~MouseGDK()
    {
        if ( m_CallbackToken && m_GameInput )
        {
            if ( !m_GameInput->UnregisterCallback( m_CallbackToken, UINT64_MAX ) )
            {
                std::cerr << "Failed to unregister device callback." << std::endl;
            }
        }
    }

    void ClipToWindow() const noexcept
    {
#ifndef _GAMING_XBOX
        assert( m_Window != nullptr );

        RECT rect;
        GetClientRect( m_Window, &rect );

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
#endif
    }

    ComPtr<IGameInput>     m_GameInput;
    GameInputCallbackToken m_CallbackToken {};
    HWND                   m_Window { nullptr };
    Mouse::Mode            m_Mode      = Mouse::Mode::Absolute;
    bool                   m_AutoReset = true;

    ScopedHandle m_ScrollWheelValue;

    mutable int     m_ScrollWheelCurrent = 0;
    mutable int64_t m_RelativeX          = INT64_MAX;
    mutable int64_t m_RelativeY          = INT64_MAX;
    mutable int64_t m_LastX              = INT64_MAX;
    mutable int64_t m_LastY              = INT64_MAX;
    mutable int64_t m_RelativeWheelY     = INT64_MAX;
};

void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam )
{
    auto& impl = MouseGDK::get();

    DWORD result = WaitForSingleObjectEx( impl.m_ScrollWheelValue.get(), 0, FALSE );
    if ( result == WAIT_FAILED )
        throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "WaitForSingleObjectEx" );

    if ( result == WAIT_OBJECT_0 )
    {
        impl.m_ScrollWheelCurrent = 0;
    }

    switch ( message )
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        if ( wParam )
        {
            if ( impl.m_Mode == Mouse::Mode::Relative )
            {
                impl.m_LastX = impl.m_RelativeX = INT64_MAX;
                impl.m_LastY = impl.m_RelativeY = INT64_MAX;

                ShowCursor( FALSE );

                impl.ClipToWindow();
            }
            else
            {
#ifndef _GAMING_XBOX
                ClipCursor( nullptr );
#endif
            }
        }
        else
        {
            impl.m_State = {};
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
        if ( impl.m_Mode == Mouse::Mode::Absolute )
        {
            impl.m_ScrollWheelCurrent += GET_WHEEL_DELTA_WPARAM( wParam );
        }
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

    default:
        // Not a mouse message, so exit
        return;
    }

    if ( impl.m_Mode == Mouse::Mode::Absolute )
    {
        // All mouse messages provide a new pointer position
        int xPos = static_cast<short>( LOWORD( lParam ) );  // GET_X_LPARAM(lParam);
        int yPos = static_cast<short>( HIWORD( lParam ) );  // GET_Y_LPARAM(lParam);

        impl.m_State.x = static_cast<int>( static_cast<float>( xPos ) * impl.m_Scale );
        impl.m_State.y = static_cast<int>( static_cast<float>( yPos ) * impl.m_Scale );
    }
}

Mouse::State Mouse::getState() const
{
    return MouseGDK::get().getState();
}

void Mouse::resetScrollWheelValue() noexcept
{
    MouseGDK::get().resetScrollWheelValue();
}

void Mouse::setMode( Mode mode )
{
    MouseGDK::get().setMode( mode );
}

void Mouse::endOfInputFrame() noexcept
{
    MouseGDK::get().endOfInputFrame();
}

bool Mouse::isConnected() const
{
    return MouseGDK::get().isConnected();
}

bool Mouse::isVisible() const noexcept
{
    return MouseGDK::get().isVisible();
}

void Mouse::setVisible( bool visible )
{
    MouseGDK::get().setVisible( visible );
}


void Mouse::setWindow( void* window )
{
    MouseGDK::get().setWindow( static_cast<HWND>( window ) );
}




