#include <SDL3/SDL.h>
#include <array>
#include <input/Gamepad.hpp>
#include <mutex>

using namespace input;

const int       Gamepad::MAX_PLAYER_COUNT = 8;
constexpr float SDLThumbDeadZone          = 0.24f;  // Consistent with XboxOneThumbDeadZone

class GamepadSDL3
{
public:
    static GamepadSDL3& get()
    {
        static GamepadSDL3 instance;
        return instance;
    }

    Gamepad::State getState( int player, Gamepad::DeadZone deadZoneMode )
    {
        std::lock_guard lock( m_Mutex );
        Gamepad::State  state = {};

        if ( player == Gamepad::MOST_RECENT_PLAYER )
            player = m_MostRecentGamepad;

        if ( player < 0 || player >= Gamepad::MAX_PLAYER_COUNT )
            return state;

        SDL_Gamepad* pad = m_Gamepads[player];
        if ( !pad )
            return state;

        state.connected = true;
        state.packet    = SDL_GetTicks();

        // Buttons
        state.buttons.a             = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_SOUTH );
        state.buttons.b             = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_EAST );
        state.buttons.x             = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_WEST );
        state.buttons.y             = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_NORTH );
        state.buttons.leftStick     = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_LEFT_STICK );
        state.buttons.rightStick    = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_RIGHT_STICK );
        state.buttons.leftShoulder  = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER );
        state.buttons.rightShoulder = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER );
        state.buttons.back          = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_BACK );
        state.buttons.start         = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_START );

        // DPad
        state.dPad.up    = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_UP );
        state.dPad.down  = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN );
        state.dPad.left  = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT );
        state.dPad.right = SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT );

        // Raw thumbstick values
        float rawLeftX  = SDL_GetGamepadAxis( pad, SDL_GAMEPAD_AXIS_LEFTX ) / 32767.0f;
        float rawLeftY  = SDL_GetGamepadAxis( pad, SDL_GAMEPAD_AXIS_LEFTY ) / 32767.0f;
        float rawRightX = SDL_GetGamepadAxis( pad, SDL_GAMEPAD_AXIS_RIGHTX ) / 32767.0f;
        float rawRightY = SDL_GetGamepadAxis( pad, SDL_GAMEPAD_AXIS_RIGHTY ) / 32767.0f;

        // Apply deadzone
        ApplyStickDeadZone( rawLeftX, rawLeftY, deadZoneMode, 1.0f, SDLThumbDeadZone, state.thumbSticks.leftX, state.thumbSticks.leftY );
        ApplyStickDeadZone( rawRightX, rawRightY, deadZoneMode, 1.0f, SDLThumbDeadZone, state.thumbSticks.rightX, state.thumbSticks.rightY );

        // Triggers
        state.triggers.left  = SDL_GetGamepadAxis( pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER ) / 32767.0f;
        state.triggers.right = SDL_GetGamepadAxis( pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER ) / 32767.0f;

        return state;
    }

    bool setVibration( int player, float leftMotor, float rightMotor, float /*leftTrigger*/, float /*rightTrigger*/ )
    {
        std::lock_guard lock( m_Mutex );
        if ( player == Gamepad::MOST_RECENT_PLAYER )
            player = m_MostRecentGamepad;

        if ( player < 0 || player >= Gamepad::MAX_PLAYER_COUNT )
            return false;

        SDL_Gamepad* pad = m_Gamepads[player];
        if ( !pad )
            return false;

        Uint16 lowFreq  = static_cast<Uint16>( leftMotor * 0xFFFF );
        Uint16 highFreq = static_cast<Uint16>( rightMotor * 0xFFFF );

        return SDL_RumbleGamepad( pad, lowFreq, highFreq, 100 ) == 0;
    }

    void suspend()
    {
        std::lock_guard lock( m_Mutex );
        for ( auto& pad: m_Gamepads )
        {
            if ( pad )
                SDL_RumbleGamepad( pad, 0, 0, 0 );
        }
    }

    void resume()
    {
        std::lock_guard lock( m_Mutex );
        for ( auto& pad: m_Gamepads )
        {
            if ( pad )
            {
                SDL_CloseGamepad( pad );
                pad = nullptr;
            }
        }

        scanGamepads();
    }

private:
    static bool SDLEventWatch( void* userdata, SDL_Event* event )
    {
        auto*           self = static_cast<GamepadSDL3*>( userdata );
        std::lock_guard lock( self->m_Mutex );

        if ( event->type == SDL_EVENT_GAMEPAD_ADDED )
        {
            SDL_JoystickID joyId = event->gdevice.which;
            if ( !SDL_IsGamepad( joyId ) )
                return true;

            SDL_Gamepad* pad = SDL_OpenGamepad( joyId );
            if ( pad )
            {
                for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
                {
                    if ( !self->m_Gamepads[i] )
                    {
                        self->m_Gamepads[i]       = pad;
                        self->m_MostRecentGamepad = i;
                        break;
                    }
                }
            }
        }
        else if ( event->type == SDL_EVENT_GAMEPAD_REMOVED )
        {
            SDL_JoystickID joyId = event->gdevice.which;
            for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
            {
                if ( self->m_Gamepads[i] && SDL_GetGamepadID( self->m_Gamepads[i] ) == joyId )
                {
                    SDL_CloseGamepad( self->m_Gamepads[i] );
                    self->m_Gamepads[i] = nullptr;
                    break;
                }
            }
        }
        return true;
    }

    GamepadSDL3()
    {
        if ( SDL_WasInit( SDL_INIT_GAMEPAD ) == 0 )
            SDL_InitSubSystem( SDL_INIT_GAMEPAD );

        scanGamepads();

        SDL_AddEventWatch( &SDLEventWatch, this );
    }

    ~GamepadSDL3()
    {
        SDL_RemoveEventWatch( &SDLEventWatch, this );
        for ( auto& pad: m_Gamepads )
        {
            if ( pad )
            {
                SDL_CloseGamepad( pad );
                pad = nullptr;
            }
        }
    }

    void scanGamepads()
    {
        // SDL3: enumerate gamepads using SDL_GetGamepads
        int             count = 0;
        SDL_JoystickID* ids   = SDL_GetGamepads( &count );
        count                 = std::min( count, Gamepad::MAX_PLAYER_COUNT );
        int gamepadId         = 0;
        if ( ids )
        {
            for ( int i = 0; i < count; ++i )
            {
                SDL_JoystickID id = ids[i];
                if ( SDL_IsGamepad( id ) )
                {
                    SDL_Gamepad* pad = SDL_OpenGamepad( id );
                    if ( pad )
                    {
                        m_Gamepads[gamepadId++] = pad;
                        m_MostRecentGamepad     = gamepadId - 1;
                    }
                }
            }
            SDL_free( ids );
        }
    }

    std::array<SDL_Gamepad*, Gamepad::MAX_PLAYER_COUNT> m_Gamepads          = {};
    int                                                 m_MostRecentGamepad = 0;
    mutable std::mutex                                  m_Mutex;
};

// Bridge to Gamepad interface
Gamepad::State Gamepad::getState( Gamepad::DeadZone deadZoneMode ) const
{
    return GamepadSDL3::get().getState( playerIndex, deadZoneMode );
}

bool Gamepad::setVibration( float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )
{
    return GamepadSDL3::get().setVibration( playerIndex, leftMotor, rightMotor, leftTrigger, rightTrigger );
}

void Gamepad::suspend() noexcept
{
    GamepadSDL3::get().suspend();
}

void Gamepad::resume() noexcept
{
    GamepadSDL3::get().resume();
}
