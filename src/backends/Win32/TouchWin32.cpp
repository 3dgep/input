#include <input/Touch.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <algorithm>
#include <mutex>
#include <system_error>
#include <vector>

using namespace input;

void Touch_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

//======================================================================================
// Win32 desktop implementation
//======================================================================================

//
// For a Win32 desktop application, in your window setup be sure to call:
//
// RegisterTouchWindow(hwnd, 0);
//
// And call this function from your Window Message Procedure
//
// void Touch_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
//
// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_TOUCH:
//         Touch_ProcessMessage(message, wParam, lParam);
//         break;
//     }
// }
//

class TouchWin32
{
public:
    static TouchWin32& get()
    {
        static TouchWin32 instance;
        return instance;
    }

    Touch::State getState()
    {
        std::lock_guard lock( m_Mutex );

        Touch::State state {};
        state.touches = m_Touches;

        // Update phase for touches that haven't changed
        for ( auto& touch : state.touches )
        {
            if ( touch.phase == Touch::Phase::Began )
            {
                // Keep Began for one frame
            }
            else if ( touch.phase != Touch::Phase::Ended && touch.phase != Touch::Phase::Cancelled )
            {
                touch.phase = Touch::Phase::Stationary;
            }
        }

        // Remove ended touches
        state.touches.erase(
            std::remove_if( state.touches.begin(), state.touches.end(),
                            []( const Touch::TouchPoint& t ) {
                                return t.phase == Touch::Phase::Ended || t.phase == Touch::Phase::Cancelled;
                            } ),
            state.touches.end() );

        m_Touches = state.touches;

        return state;
    }

    bool isSupported() const
    {
        return ( GetSystemMetrics( SM_DIGITIZER ) & NID_READY ) != 0;
    }

    int getDeviceCount() const
    {
        const int digitizer = GetSystemMetrics( SM_DIGITIZER );
        if ( digitizer & NID_READY )
        {
            const int maxContacts = GetSystemMetrics( SM_MAXIMUMTOUCHES );
            return maxContacts > 0 ? 1 : 0;  // Windows reports one touch device
        }
        return 0;
    }

    void setWindow( HWND window )
    {
        if ( m_Window == window )
            return;

        if ( window != nullptr )
        {
            if ( !RegisterTouchWindow( window, 0 ) )
            {
                throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "RegisterTouchWindow" );
            }
        }

        m_Window = window;
    }

    TouchWin32( const TouchWin32& )            = delete;
    TouchWin32( TouchWin32&& )                 = delete;
    TouchWin32& operator=( const TouchWin32& ) = delete;
    TouchWin32& operator=( TouchWin32&& )      = delete;

private:
    TouchWin32()  = default;
    ~TouchWin32() = default;

    friend void Touch_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

    std::mutex                     m_Mutex;
    std::vector<Touch::TouchPoint> m_Touches;
    HWND                           m_Window { nullptr };
};

void Touch_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam )
{
    auto& impl = TouchWin32::get();

    if ( message != WM_TOUCH )
        return;

    const UINT cInputs = LOWORD( wParam );
    auto       pInputs = std::make_unique<TOUCHINPUT[]>( cInputs );

    if ( GetTouchInputInfo( reinterpret_cast<HTOUCHINPUT>( lParam ), cInputs, pInputs.get(), sizeof( TOUCHINPUT ) ) )
    {
        std::lock_guard lock( impl.m_Mutex );

        for ( UINT i = 0; i < cInputs; ++i )
        {
            const TOUCHINPUT& ti = pInputs[i];

            // Convert screen coordinates to normalized coordinates
            POINT pt;
            pt.x = TOUCH_COORD_TO_PIXEL( ti.x );
            pt.y = TOUCH_COORD_TO_PIXEL( ti.y );

            if ( impl.m_Window )
            {
                ScreenToClient( impl.m_Window, &pt );

                RECT rect;
                GetClientRect( impl.m_Window, &rect );

                const int width  = rect.right - rect.left;
                const int height = rect.bottom - rect.top;
                float normalizedX = 0.0f;
                float normalizedY = 0.0f;
                if (width != 0 && height != 0)
                {
                    normalizedX = static_cast<float>( pt.x ) / static_cast<float>( width );
                    normalizedY = static_cast<float>( pt.y ) / static_cast<float>( height );
                }

                // Find existing touch
                auto it = std::find_if( impl.m_Touches.begin(), impl.m_Touches.end(),
                                        [&]( const Touch::TouchPoint& t ) {
                                            return t.id == static_cast<int64_t>( ti.dwID );
                                        } );

                if ( ti.dwFlags & TOUCHEVENTF_DOWN )
                {
                    // New touch
                    Touch::TouchPoint touch;
                    touch.id       = static_cast<int64_t>( ti.dwID );
                    touch.x        = normalizedX;
                    touch.y        = normalizedY;
                    touch.pressure = 1.0f;  // Windows doesn't provide pressure in basic touch
                    touch.phase    = Touch::Phase::Began;
                    impl.m_Touches.push_back( touch );
                }
                else if ( ti.dwFlags & TOUCHEVENTF_MOVE )
                {
                    if ( it != impl.m_Touches.end() )
                    {
                        it->x     = normalizedX;
                        it->y     = normalizedY;
                        it->phase = Touch::Phase::Moved;
                    }
                }
                else if ( ti.dwFlags & TOUCHEVENTF_UP )
                {
                    if ( it != impl.m_Touches.end() )
                    {
                        it->x        = normalizedX;
                        it->y        = normalizedY;
                        it->pressure = 0.0f;
                        it->phase    = Touch::Phase::Ended;
                    }
                }
            }
        }

        CloseTouchInputHandle( reinterpret_cast<HTOUCHINPUT>( lParam ) );
    }
}

namespace input::Touch
{

State getState()
{
    return TouchWin32::get().getState();
}

bool isSupported()
{
    return TouchWin32::get().isSupported();
}

int getDeviceCount()
{
    return TouchWin32::get().getDeviceCount();
}

void setWindow( void* window )
{
    TouchWin32::get().setWindow( static_cast<HWND>( window ) );
}

}  // namespace input::Touch
