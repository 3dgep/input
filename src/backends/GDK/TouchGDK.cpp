#include <input/Touch.hpp>

#include <GameInput.h>
#include <wrl.h>

#include <format>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace input;
using namespace Microsoft::WRL;

//======================================================================================
// GDK implementation - Stub (Xbox/Console platforms typically don't use touch)
//======================================================================================

class TouchGDK
{
public:
    static TouchGDK& get()
    {
        static TouchGDK touchGDK;
        return touchGDK;
    }

    Touch::State getState()
    {
        // Reset active touch states from the previous frame.
        for (Touch::TouchPoint& touchPoint : m_Touches)
        {
            touchPoint.phase = Touch::Phase::Ended;
        }

        ComPtr<IGameInputReading> reading = nullptr;
        if ( SUCCEEDED( m_GameInput->GetCurrentReading( GameInputKindTouch, nullptr, reading.GetAddressOf() ) ) )
        {
            uint32_t touchCount = reading->GetTouchCount();
            if (touchCount > 0)
            {
                std::vector<GameInputTouchState> touchStates( touchCount );
                touchCount = reading->GetTouchState( touchCount, touchStates.data() );

                for (const GameInputTouchState& touchState : touchStates )
                {
                    // Check to see if we are already tracking this touch point.
                    auto it = std::find_if( m_Touches.begin(), m_Touches.end(), [&]( const Touch::TouchPoint& t ) {
                        return t.id == touchState.touchId;
                    } );

                    if (it != m_Touches.end() )
                    {
                        if (std::abs( it->x - touchState.positionX ) > FLT_EPSILON || std::abs( it->y - touchState.positionY ) > FLT_EPSILON )
                        {
                            it->x = touchState.positionX;
                            it->y = touchState.positionY;
                            it->phase = Touch::Phase::Moved;
                        }
                        else
                        {
                            it->phase = Touch::Phase::Stationary;
                        }
                    }
                    else
                    {
                        // This is a new touch point. Start tracking it.
                        Touch::TouchPoint point;
                        point.id = touchState.touchId;
                        point.x     = touchState.positionX;
                        point.y     = touchState.positionY;
                        point.pressure = touchState.pressure;
                        point.phase    = Touch::Phase::Began;

                        m_Touches.push_back( point );
                    }
                }
            }
        }

        return { m_Touches };
    }

    void endFrame()
    {
        // Remove ended touches
        std::erase_if( m_Touches,
                       []( const Touch::TouchPoint& t ) {
                           return t.phase == Touch::Phase::Ended || t.phase == Touch::Phase::Cancelled;
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
        return m_Connected > 0;
    }

    int getDeviceCount() const
    {
        return static_cast<int>( m_Connected );
    }

private:
    static void CALLBACK OnGameInputDevice(
        _In_       GameInputCallbackToken,
        _In_ void* context,
        _In_ IGameInputDevice*,
        _In_                       uint64_t,
        _In_ GameInputDeviceStatus currentStatus,
        _In_ GameInputDeviceStatus previousStatus ) noexcept
    {
        auto impl = static_cast<TouchGDK*>( context );

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

    TouchGDK()
    {
        HRESULT hr = GameInputCreate( m_GameInput.GetAddressOf() );
        if ( SUCCEEDED( hr ) )
        {
            hr = m_GameInput->RegisterDeviceCallback(
                nullptr,
                GameInputKindTouch,
                GameInputDeviceConnected,
                GameInputBlockingEnumeration,
                this,
                OnGameInputDevice,
                &m_CallbackToken
            );
            if ( FAILED( hr ) )
            {
                throw std::runtime_error( std::format( "Failed to register touch device callback: {:08X}", static_cast<unsigned int>( hr ) ) );
            }
        }
        else
        {
            throw std::runtime_error( std::format( "Failed to create GameInput: {:08X}", static_cast<unsigned int>( hr ) ) );
        }
    }

    ~TouchGDK()
    {
        if (m_CallbackToken && m_GameInput )
        {
            if ( !m_GameInput->UnregisterCallback( m_CallbackToken, UINT64_MAX ) )
            {
                std::cerr << "Failed to unregister device callback." << std::endl;
            }
        }
    }

    ComPtr<IGameInput>             m_GameInput;
    GameInputCallbackToken         m_CallbackToken {};
    uint32_t                       m_Connected = 0;
    std::vector<Touch::TouchPoint> m_Touches;
};

namespace input::Touch
{

State getState()
{
    return TouchGDK::get().getState();
}

void endFrame()
{
    return TouchGDK::get().endFrame();
}

bool isSupported()
{
    return TouchGDK::get().isSupported();
}

int getDeviceCount()
{
    return TouchGDK::get().getDeviceCount();
}

}  // namespace input::Touch
