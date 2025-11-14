#include <input/Mouse.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>

#include <cassert>
#include <mutex>

using namespace input;

class MouseSDL3
{
public:
    static MouseSDL3& get()
    {
        static MouseSDL3 instance;
        return instance;
    }

    Mouse::State getState() const
    {
        std::lock_guard lock( m_Mutex );

        Mouse::State state {};
        state.positionMode = m_Mode;

        float  x = 0, y = 0;
        Uint32 buttons = SDL_GetMouseState( &x, &y );

        state.leftButton   = ( buttons & SDL_BUTTON_MASK( SDL_BUTTON_LEFT ) ) != 0;
        state.middleButton = ( buttons & SDL_BUTTON_MASK( SDL_BUTTON_MIDDLE ) ) != 0;
        state.rightButton  = ( buttons & SDL_BUTTON_MASK( SDL_BUTTON_RIGHT ) ) != 0;
        state.xButton1     = ( buttons & SDL_BUTTON_MASK( SDL_BUTTON_X1 ) ) != 0;
        state.xButton2     = ( buttons & SDL_BUTTON_MASK( SDL_BUTTON_X2 ) ) != 0;

        if ( m_Mode == Mouse::Mode::Absolute )
        {
            state.x = x;
            state.y = y;
        }
        else  // Relative mode
        {
            state.x = m_RelativeX;
            state.y = m_RelativeY;
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
                m_AccumulateX = m_RelativeX = 0;
                m_AccumulateY = m_RelativeY = 0;
            }
        }

        // The mutex must be unlocked before calling
        // these functions since they will call the SDLEventWatch method
        // which also tries to lock the mutex.
        if ( mode == Mouse::Mode::Relative )
        {
            assert( m_Window != nullptr );
            SDL_SetWindowRelativeMouseMode( m_Window, true );
        }
        else
        {
            SDL_SetWindowRelativeMouseMode( m_Window, false );
        }
    }

    void resetRelativeMotion() noexcept
    {
        std::lock_guard lock( m_Mutex );
        if ( m_Mode == Mouse::Mode::Relative )
        {
            m_RelativeX = m_AccumulateX;
            m_RelativeY = m_AccumulateY;

            m_AccumulateX = 0.0f;
            m_AccumulateY = 0.0f;
        }
    }

    bool isConnected() const
    {
        return SDL_HasMouse();
    }

    bool isVisible() const noexcept
    {
        return SDL_CursorVisible();
    }

    void setVisible( bool visible )
    {
        if ( visible )
            SDL_ShowCursor();
        else
            SDL_HideCursor();
    }

    void setWindow( SDL_Window* window )
    {
        m_Window = window;
    }

    MouseSDL3( const MouseSDL3& )            = delete;
    MouseSDL3( MouseSDL3&& )                 = delete;
    MouseSDL3& operator=( const MouseSDL3& ) = delete;
    MouseSDL3& operator=( MouseSDL3&& )      = delete;

private:
    static bool SDLEventWatch( void* userdata, SDL_Event* event )
    {
        auto*           self = static_cast<MouseSDL3*>( userdata );
        std::lock_guard lock( self->m_Mutex );

        switch ( event->type )
        {
        case SDL_EVENT_MOUSE_WHEEL:
            self->m_ScrollWheelValue += event->wheel.y * 120;  // 120 is Win32/DirectX standard
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if ( self->m_Mode == Mouse::Mode::Relative )
            {
                self->m_AccumulateX += event->motion.xrel;
                self->m_AccumulateY += event->motion.yrel;
            }
            break;
        }

        return true;
    }

    MouseSDL3()
    {
        SDL_SetWindowRelativeMouseMode( nullptr, false );
        SDL_AddEventWatch( &MouseSDL3::SDLEventWatch, this );
    }

    ~MouseSDL3()
    {
        SDL_RemoveEventWatch( &MouseSDL3::SDLEventWatch, this );
    }

    mutable std::mutex m_Mutex;

    Mouse::Mode m_Mode             = Mouse::Mode::Absolute;
    SDL_Window* m_Window           = nullptr;
    float       m_ScrollWheelValue = 0.0f;
    float       m_AccumulateX      = 0.0f;
    float       m_AccumulateY      = 0.0f;
    float       m_RelativeX        = 0.0f;
    float       m_RelativeY        = 0.0f;
};

namespace input::Mouse
{
State getState()
{
    return MouseSDL3::get().getState();
}

void resetScrollWheelValue() noexcept
{
    MouseSDL3::get().resetScrollWheelValue();
}

void setMode( Mode mode )
{
    MouseSDL3::get().setMode( mode );
}

void resetRelativeMotion() noexcept
{
    MouseSDL3::get().resetRelativeMotion();
}

bool isConnected()
{
    return MouseSDL3::get().isConnected();
}

bool isVisible() noexcept
{
    return MouseSDL3::get().isVisible();
}

void setVisible( bool visible )
{
    MouseSDL3::get().setVisible( visible );
}

void setWindow( void* window )
{
    MouseSDL3::get().setWindow( static_cast<SDL_Window*>( window ) );
}

}
