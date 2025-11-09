#include <algorithm>
#include <input/Touch.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <algorithm>
#include <chrono>
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
// Touch::setWindow( hWnd );
//
// And call this function from your Window Message Procedure
//
// void Touch_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
//
// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_POINTERDOWN:
//     case WM_POINTERUPDATE:
//     case WM_POINTERUP:
//         Touch_ProcessMessage(message, wParam, lParam);
//         break;
//     }
// }
//
static uint64_t getTimestamp()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>( steady_clock::now().time_since_epoch() ).count();
}

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

        return state;
    }

    void endFrame()
    {
        uint64_t timestamp = getTimestamp();
        // Remove ended touches
        std::erase_if( m_Touches,
                       [timestamp]( const Touch::TouchPoint& t ) {
                           return t.phase == Touch::Phase::Ended ||
                                  t.phase == Touch::Phase::Cancelled ||
                                  ( t.phase == Touch::Phase::Stationary && ( timestamp - t.timestamp ) > 1000000000ull );  // Stale touch points.
                       } );

        // Update phase for touches that haven't changed
        for ( auto& touch: m_Touches )
        {
            if ( touch.phase != Touch::Phase::Began )
            {
                touch.phase = Touch::Phase::Stationary;
            }
        }
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

    if ( message == WM_POINTERDOWN || message == WM_POINTERUPDATE || message == WM_POINTERUP )
    {
        const UINT32 pointerId = GET_POINTERID_WPARAM( wParam );
        POINTER_INFO pointerInfo;
        if ( GetPointerInfo( pointerId, &pointerInfo ) )
        {
            POINT pt = pointerInfo.ptPixelLocation;
            if ( impl.m_Window )
            {
                ScreenToClient( impl.m_Window, &pt );

                RECT rect;
                GetClientRect( impl.m_Window, &rect );

                const int width       = rect.right - rect.left;
                const int height      = rect.bottom - rect.top;
                float     normalizedX = 0.0f;
                float     normalizedY = 0.0f;
                if ( width != 0 && height != 0 )
                {
                    normalizedX = static_cast<float>( pt.x ) / static_cast<float>( width );
                    normalizedY = static_cast<float>( pt.y ) / static_cast<float>( height );
                }

                float              pressure = 1.0f;
                POINTER_INPUT_TYPE pointerType;
                bool               isPen   = false;
                bool               isTouch = false;
                if ( GetPointerType( pointerId, &pointerType ) )
                {
                    isPen   = ( pointerType == PT_PEN );
                    isTouch = ( pointerType == PT_TOUCH );
                    if ( isPen )
                    {
                        POINTER_PEN_INFO penInfo;
                        if ( GetPointerPenInfo( pointerId, &penInfo ) )
                        {
                            pressure = static_cast<float>( penInfo.pressure ) / 1024.0f;
                            pressure = std::clamp( pressure, 0.0f, 1.0f );
                        }
                    }
                }

                std::lock_guard lock( impl.m_Mutex );
                auto            it = std::find_if( impl.m_Touches.begin(), impl.m_Touches.end(),
                                                   [&]( const Touch::TouchPoint& t ) {
                                            return t.id == static_cast<int64_t>( pointerId );
                                        } );

                // Handle pointer cancel
                if ( pointerInfo.pointerFlags & POINTER_FLAG_CANCELED )
                {
                    if ( it != impl.m_Touches.end() )
                    {
                        it->x        = normalizedX;
                        it->y        = normalizedY;
                        it->pressure = 0.0f;
                        it->phase    = Touch::Phase::Cancelled;
                    }
                    return;
                }

                if ( message == WM_POINTERDOWN )
                {
                    if ( it != impl.m_Touches.end() )
                    {
                        it->timestamp = getTimestamp();
                        it->x         = normalizedX;
                        it->y         = normalizedY;
                        it->pressure  = pressure;
                        it->phase     = Touch::Phase::Began;
                    }
                    else
                    {
                        Touch::TouchPoint touch;
                        touch.id        = static_cast<int64_t>( pointerId );
                        touch.timestamp = getTimestamp();
                        touch.x         = normalizedX;
                        touch.y         = normalizedY;
                        touch.pressure  = pressure;
                        touch.phase     = Touch::Phase::Began;
                        impl.m_Touches.push_back( touch );
                    }
                }
                else if ( message == WM_POINTERUPDATE )
                {
                    if ( it != impl.m_Touches.end() )
                    {
                        it->timestamp = getTimestamp();
                        it->x         = normalizedX;
                        it->y         = normalizedY;
                        it->pressure  = pressure;
                        it->phase     = Touch::Phase::Moved;
                    }
                }
                else if ( message == WM_POINTERUP )
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
    }
}

namespace input::Touch
{

State getState()
{
    return TouchWin32::get().getState();
}

void endFrame()
{
    TouchWin32::get().endFrame();
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
