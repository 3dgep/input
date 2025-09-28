#include "input/Mouse.hpp"

#include <input/Mouse.hpp>

#include <SDL.h>

#include <cstring>
#include <mutex>

using namespace input;

class MouseSDL2
{
public:
    static MouseSDL2& get()
    {
        static MouseSDL2 instance;
        return instance;
    }

    Mouse::State getState() const
    {
        std::lock_guard lock( m_Mutex );

        Mouse::State state;
        state.positionMode = m_Mode;

        int    x = 0, y = 0;
        Uint32 buttons = SDL_GetMouseState( &x, &y );

        state.leftButton   = ( buttons & SDL_BUTTON( SDL_BUTTON_LEFT ) ) != 0;
        state.middleButton = ( buttons & SDL_BUTTON( SDL_BUTTON_MIDDLE ) ) != 0;
        state.rightButton  = ( buttons & SDL_BUTTON( SDL_BUTTON_RIGHT ) ) != 0;
        state.xButton1     = ( buttons & SDL_BUTTON( SDL_BUTTON_X1 ) ) != 0;
        state.xButton2     = ( buttons & SDL_BUTTON( SDL_BUTTON_X2 ) ) != 0;

        if ( m_Mode == Mouse::Mode::Absolute )
        {
            state.x = static_cast<float>( x );
            state.y = static_cast<float>( y );
        }
        else  // Relative mode
        {
            state.x = static_cast<float>( m_RelativeX );
            state.y = static_cast<float>( m_RelativeY );
        }

        state.scrollWheelValue = m_ScrollWheelValue;

        return state;
    }

    void resetScrollWheelValue() noexcept
    {
        std::lock_guard lock( m_Mutex );
        m_ScrollWheelValue = 0;
    }

    void setMode( Mouse::Mode mode )
    {
        {
            std::lock_guard lock( m_Mutex );
            if ( m_Mode == mode )
                return;

            m_Mode = mode;
            if ( mode == Mouse::Mode::Relative )
            {
                m_RelativeX = 0;
                m_RelativeY = 0;
            }
        }

        // The mutex must be unlocked before calling
        // these functions since they will call the SDLEventWatch method
        // which also tries to lock the mutex.
        if ( mode == Mouse::Mode::Relative )
        {
            SDL_SetRelativeMouseMode( SDL_TRUE );
        }
        else
        {
            SDL_SetRelativeMouseMode( SDL_FALSE );
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
        // SDL2 always has a mouse device
        return true;
    }

    bool isVisible() const noexcept
    {
        return SDL_ShowCursor( SDL_QUERY ) == SDL_ENABLE;
    }

    void setVisible( bool visible )
    {
        SDL_ShowCursor( visible ? SDL_ENABLE : SDL_DISABLE );
    }

    void setWindow( void* window )
    {
        // SDL2 does not require explicit window binding for mouse
        (void)window;
    }

    MouseSDL2( const MouseSDL2& )            = delete;
    MouseSDL2( MouseSDL2&& )                 = delete;
    MouseSDL2& operator=( const MouseSDL2& ) = delete;
    MouseSDL2& operator=( MouseSDL2&& )      = delete;

private:
    static int SDLEventWatch( void* userdata, SDL_Event* event )
    {
        auto*           self = static_cast<MouseSDL2*>( userdata );
        std::lock_guard lock( self->m_Mutex );

        if ( event->type == SDL_MOUSEWHEEL )
        {
            self->m_ScrollWheelValue += event->wheel.y * 120;  // 120 is Win32/DirectX standard
        }
        else if ( event->type == SDL_MOUSEMOTION && self->m_Mode == Mouse::Mode::Relative )
        {
            self->m_RelativeX += event->motion.xrel;
            self->m_RelativeY += event->motion.yrel;
        }
        return 0;
    }

    MouseSDL2()
    {
        SDL_SetRelativeMouseMode( SDL_FALSE );
        SDL_AddEventWatch( &SDLEventWatch, this );
    }

    ~MouseSDL2()
    {
        SDL_DelEventWatch( &SDLEventWatch, this );
    }

    mutable std::mutex m_Mutex;
    Mouse::Mode        m_Mode = Mouse::Mode::Absolute;
    int                m_ScrollWheelValue = 0;
    int                m_RelativeX = 0;
    int                m_RelativeY = 0;
};

namespace input::Mouse
{
State getState()
{
    return MouseSDL2::get().getState();
}

void resetScrollWheelValue() noexcept
{
    MouseSDL2::get().resetScrollWheelValue();
}

void setMode( Mode mode )
{
    MouseSDL2::get().setMode( mode );
}

void resetRelativeMotion() noexcept
{
    MouseSDL2::get().resetRelativeMotion();
}

bool isConnected()
{
    return MouseSDL2::get().isConnected();
}

bool isVisible() noexcept
{
    return MouseSDL2::get().isVisible();
}

void setVisible( bool visible )
{
    MouseSDL2::get().setVisible( visible );
}

void setWindow( void* window )
{
    MouseSDL2::get().setWindow( window );
}
}
