#include <input/Touch.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_touch.h>

#include <algorithm>
#include <mutex>

using namespace input;

class TouchSDL3
{
public:
    static TouchSDL3& get()
    {
        static TouchSDL3 instance;
        return instance;
    }

    Touch::State getState() const
    {
        std::lock_guard lock( m_Mutex );

        Touch::State state {};
        state.touches = m_Touches;

        return state;
    }

    void endFrame()
    {
        std::lock_guard lock( m_Mutex );

        // Remove touches that ended in the previous frame
        std::erase_if( m_Touches,
                       []( const Touch::TouchPoint& t ) {
                           return t.phase == Touch::Phase::Ended || t.phase == Touch::Phase::Cancelled;
                       } );

        // Mark remaining touches as stationary (they will be updated to Moved if motion events occur)
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
        return getDeviceCount() > 0;
    }

    int getDeviceCount() const
    {
        int          numTouchDevices = 0;
        SDL_TouchID* touchId         = SDL_GetTouchDevices( &numTouchDevices );
        SDL_free( touchId );

        return numTouchDevices;
    }

    TouchSDL3( const TouchSDL3& )            = delete;
    TouchSDL3( TouchSDL3&& )                 = delete;
    TouchSDL3& operator=( const TouchSDL3& ) = delete;
    TouchSDL3& operator=( TouchSDL3&& )      = delete;

private:
    static bool SDLEventWatch( void* userdata, SDL_Event* event )
    {
        auto*           self = static_cast<TouchSDL3*>( userdata );
        std::lock_guard lock( self->m_Mutex );

        switch ( event->type )
        {
        case SDL_EVENT_FINGER_DOWN:
        {
            Touch::TouchPoint touch;
            touch.id        = event->tfinger.fingerID;
            touch.timestamp = event->tfinger.timestamp;
            touch.x         = event->tfinger.x;
            touch.y         = event->tfinger.y;
            touch.pressure  = event->tfinger.pressure;
            touch.phase     = Touch::Phase::Began;
            self->m_Touches.push_back( touch );
            break;
        }
        case SDL_EVENT_FINGER_MOTION:
        {
            auto it = std::find_if( self->m_Touches.begin(), self->m_Touches.end(),
                                    [&]( const Touch::TouchPoint& t ) {
                                        return t.id == event->tfinger.fingerID;
                                    } );
            if ( it != self->m_Touches.end() )
            {
                it->timestamp = event->tfinger.timestamp;
                it->x         = event->tfinger.x;
                it->y         = event->tfinger.y;
                it->pressure  = event->tfinger.pressure;
                it->phase     = Touch::Phase::Moved;
            }
            break;
        }
        case SDL_EVENT_FINGER_UP:
        {
            auto it = std::find_if( self->m_Touches.begin(), self->m_Touches.end(),
                                    [&]( const Touch::TouchPoint& t ) {
                                        return t.id == static_cast<int64_t>( event->tfinger.fingerID );
                                    } );
            if ( it != self->m_Touches.end() )
            {
                it->x        = event->tfinger.x;
                it->y        = event->tfinger.y;
                it->pressure = 0.0f;
                it->phase    = Touch::Phase::Ended;
                // Keep the touch for one more frame so it can be detected as Released
                // It will be removed when endFrame is called.
            }
            break;
        }
        }

        return true;
    }

    TouchSDL3()
    {
        SDL_AddEventWatch( &TouchSDL3::SDLEventWatch, this );
    }

    ~TouchSDL3()
    {
        SDL_RemoveEventWatch( &TouchSDL3::SDLEventWatch, this );
    }

    mutable std::mutex             m_Mutex;
    std::vector<Touch::TouchPoint> m_Touches;

    friend class TouchSDL3Updater;
};

namespace input::Touch
{

State getState()
{
    return TouchSDL3::get().getState();
}

void endFrame()
{
    TouchSDL3::get().endFrame();
}

bool isSupported()
{
    return TouchSDL3::get().isSupported();
}

int getDeviceCount()
{
    return TouchSDL3::get().getDeviceCount();
}

}  // namespace input::Touch
