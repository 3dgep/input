#include "input/Mouse.hpp"

#include <SDL3/SDL.h>
#include <cassert>
#include <input/Mouse.hpp>
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
            state.x = static_cast<int>( x );
            state.y = static_cast<int>( y );
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
        std::lock_guard lock( m_Mutex );
        if ( m_Mode == mode )
            return;

        m_Mode = mode;
        if ( mode == Mouse::Mode::Relative )
        {
            assert( m_Window != nullptr );

            SDL_SetWindowRelativeMouseMode( m_Window, true );

            m_RelativeX = 0;
            m_RelativeY = 0;
        }
        else
        {
            SDL_SetWindowRelativeMouseMode( m_Window, false );
        }
    }

    void endOfInputFrame() noexcept
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
        // SDL3 always has a mouse device
        return true;
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

    void setWindow( void* window )
    {
        m_Window = static_cast<SDL_Window*>( window );
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

        if ( event->type == SDL_EVENT_MOUSE_WHEEL )
        {
            self->m_ScrollWheelValue += event->wheel.y * 120;  // 120 is Win32/DirectX standard
        }
        else if ( event->type == SDL_EVENT_MOUSE_MOTION && self->m_Mode == Mouse::Mode::Relative )
        {
            self->m_RelativeX += event->motion.xrel;
            self->m_RelativeY += event->motion.yrel;
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
    float       m_RelativeX        = 0.0f;
    float       m_RelativeY        = 0.0f;
};

// Bridge to Mouse interface
Mouse::State Mouse::getState() const
{
    return MouseSDL3::get().getState();
}

void Mouse::resetScrollWheelValue() noexcept
{
    MouseSDL3::get().resetScrollWheelValue();
}

void Mouse::setMode( Mode mode )
{
    MouseSDL3::get().setMode( mode );
}

void Mouse::endOfInputFrame() noexcept
{
    MouseSDL3::get().endOfInputFrame();
}

bool Mouse::isConnected() const
{
    return MouseSDL3::get().isConnected();
}

bool Mouse::isVisible() const noexcept
{
    return MouseSDL3::get().isVisible();
}

void Mouse::setVisible( bool visible )
{
    MouseSDL3::get().setVisible( visible );
}

void Mouse::setWindow( void* window )
{
    MouseSDL3::get().setWindow( window );
}