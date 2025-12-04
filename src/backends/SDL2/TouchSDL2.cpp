#include <input/Touch.hpp>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_touch.h>

#include <algorithm>
#include <mutex>

using namespace input;

class TouchSDL2
{
public:
    static TouchSDL2& get()
    {
        static TouchSDL2 instance;
        return instance;
    }

    Touch::State getState() const
    {
        std::scoped_lock lock( m_Mutex );

        Touch::State state {};
        state.touches = m_Touches;

        return state;
    }

    void endFrame()
    {
        std::scoped_lock lock( m_Mutex );

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
        return SDL_GetNumTouchDevices() > 0;
    }

    int getDeviceCount() const
    {
        return SDL_GetNumTouchDevices();
    }

    TouchSDL2( const TouchSDL2& )            = delete;
    TouchSDL2( TouchSDL2&& )                 = delete;
    TouchSDL2& operator=( const TouchSDL2& ) = delete;
    TouchSDL2& operator=( TouchSDL2&& )      = delete;

private:
    static int SDLEventWatch( void* userdata, SDL_Event* event )
    {
        auto*            self = static_cast<TouchSDL2*>( userdata );
        std::scoped_lock lock( self->m_Mutex );

        switch ( event->type )
        {
        case SDL_FINGERDOWN:
        {
            Touch::TouchPoint touch;
            touch.id        = static_cast<uint64_t>( event->tfinger.fingerId );
            touch.timestamp = event->tfinger.timestamp;
            touch.x         = event->tfinger.x;
            touch.y         = event->tfinger.y;
            touch.pressure  = event->tfinger.pressure;
            touch.phase     = Touch::Phase::Began;
            self->m_Touches.push_back( touch );
            break;
        }
        case SDL_FINGERMOTION:
        {
            auto it = std::ranges::find_if( self->m_Touches,
                                            [&]( const Touch::TouchPoint& t ) {
                                                return std::cmp_equal( t.id, event->tfinger.fingerId );
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
        case SDL_FINGERUP:
        {
            auto it = std::ranges::find_if( self->m_Touches,
                                            [&]( const Touch::TouchPoint& t ) {
                                                return std::cmp_equal( t.id, event->tfinger.fingerId );
                                            } );
            if ( it != self->m_Touches.end() )
            {
                it->x        = event->tfinger.x;
                it->y        = event->tfinger.y;
                it->pressure = 0.0f;
                it->phase    = Touch::Phase::Ended;
                // Keep the touch for one more frame so it can be detected as Released
                // It will be removed on the next frame update
            }
            break;
        }
        }

        return 1;  // SDL2 returns int instead of bool
    }

    TouchSDL2()
    {
        SDL_AddEventWatch( &TouchSDL2::SDLEventWatch, this );
    }

    ~TouchSDL2()
    {
        SDL_DelEventWatch( &TouchSDL2::SDLEventWatch, this );
    }

    mutable std::mutex             m_Mutex;
    std::vector<Touch::TouchPoint> m_Touches;

    friend class TouchSDL2Updater;
};

namespace input::Touch
{

State getState()
{
    return TouchSDL2::get().getState();
}

void endFrame()
{
    TouchSDL2::get().endFrame();
}

bool isSupported()
{
    return TouchSDL2::get().isSupported();
}

int getDeviceCount()
{
    return TouchSDL2::get().getDeviceCount();
}

}  // namespace input::Touch
