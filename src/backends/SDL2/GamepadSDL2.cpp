#include <SDL.h>
#include <array>
#include <input/Gamepad.hpp>
#include <mutex>

using namespace input;

const int       Gamepad::MAX_PLAYER_COUNT = 8;
constexpr float SDLThumbDeadZone          = 0.24f;  // Consistent with XboxOneThumbDeadZone

class GamepadSDL2
{
public:
    static GamepadSDL2& get()
    {
        static GamepadSDL2 instance;
        return instance;
    }

    Gamepad::State getState( int player, Gamepad::DeadZone deadZoneMode )
    {
        std::lock_guard<std::mutex> lock( m_Mutex );
        Gamepad::State              state = {};

        if ( player == Gamepad::MOST_RECENT_PLAYER )
            player = m_MostRecentGamepad;

        if ( player < 0 || player >= Gamepad::MAX_PLAYER_COUNT )
            return state;

        SDL_GameController* ctrl = m_Controllers[player];
        if ( !ctrl )
            return state;

        state.connected = true;
        state.packet    = SDL_GetTicks();

        // Buttons
        state.buttons.a             = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_A );
        state.buttons.b             = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_B );
        state.buttons.x             = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_X );
        state.buttons.y             = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_Y );
        state.buttons.leftStick     = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_LEFTSTICK );
        state.buttons.rightStick    = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_RIGHTSTICK );
        state.buttons.leftShoulder  = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_LEFTSHOULDER );
        state.buttons.rightShoulder = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER );
        state.buttons.back          = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_BACK );
        state.buttons.start         = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_START );

        // DPad
        state.dPad.up    = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_DPAD_UP );
        state.dPad.down  = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_DPAD_DOWN );
        state.dPad.left  = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_DPAD_LEFT );
        state.dPad.right = SDL_GameControllerGetButton( ctrl, SDL_CONTROLLER_BUTTON_DPAD_RIGHT );

        // Raw thumbstick values
        float rawLeftX  = SDL_GameControllerGetAxis( ctrl, SDL_CONTROLLER_AXIS_LEFTX ) / 32767.0f;
        float rawLeftY  = SDL_GameControllerGetAxis( ctrl, SDL_CONTROLLER_AXIS_LEFTY ) / 32767.0f;
        float rawRightX = SDL_GameControllerGetAxis( ctrl, SDL_CONTROLLER_AXIS_RIGHTX ) / 32767.0f;
        float rawRightY = SDL_GameControllerGetAxis( ctrl, SDL_CONTROLLER_AXIS_RIGHTY ) / 32767.0f;

        // Apply deadzone
        ApplyStickDeadZone( rawLeftX, rawLeftY, deadZoneMode, 1.0f, SDLThumbDeadZone, state.thumbSticks.leftX, state.thumbSticks.leftY );
        ApplyStickDeadZone( rawRightX, rawRightY, deadZoneMode, 1.0f, SDLThumbDeadZone, state.thumbSticks.rightX, state.thumbSticks.rightY );

        // Triggers
        state.triggers.left  = SDL_GameControllerGetAxis( ctrl, SDL_CONTROLLER_AXIS_TRIGGERLEFT ) / 32767.0f;
        state.triggers.right = SDL_GameControllerGetAxis( ctrl, SDL_CONTROLLER_AXIS_TRIGGERRIGHT ) / 32767.0f;

        return state;
    }

    bool setVibration( int player, float leftMotor, float rightMotor, float /*leftTrigger*/, float /*rightTrigger*/ )
    {
        std::lock_guard<std::mutex> lock( m_Mutex );
        if ( player == Gamepad::MOST_RECENT_PLAYER )
            player = m_MostRecentGamepad;

        if ( player < 0 || player >= Gamepad::MAX_PLAYER_COUNT )
            return false;

        SDL_GameController* ctrl = m_Controllers[player];
        if ( !ctrl )
            return false;

        Uint16 lowFreq  = static_cast<Uint16>( leftMotor * 0xFFFF );
        Uint16 highFreq = static_cast<Uint16>( rightMotor * 0xFFFF );

        return SDL_GameControllerRumble( ctrl, lowFreq, highFreq, UINT32_MAX ) == 0;
    }

    void suspend()
    {
        std::lock_guard<std::mutex> lock( m_Mutex );
        for ( auto& ctrl: m_Controllers )
        {
            if ( ctrl )
                SDL_GameControllerRumble( ctrl, 0, 0, 0 );
        }
    }

    void resume()
    {
        std::lock_guard<std::mutex> lock( m_Mutex );

        // Close all currently open controllers
        for ( auto& ctrl: m_Controllers )
        {
            if ( ctrl )
            {
                SDL_GameControllerClose( ctrl );
                ctrl = nullptr;
            }
        }

        // Rescan and open all available controllers
        int numJoysticks = SDL_NumJoysticks();
        int idx          = 0;
        for ( int i = 0; i < numJoysticks && idx < Gamepad::MAX_PLAYER_COUNT; ++i )
        {
            if ( SDL_IsGameController( i ) )
            {
                SDL_GameController* ctrl = SDL_GameControllerOpen( i );
                if ( ctrl )
                {
                    m_Controllers[idx++] = ctrl;
                    m_MostRecentGamepad  = idx - 1;
                }
            }
        }
    }

private:
    static int SDLEventWatch( void* userdata, SDL_Event* event )
    {
        auto*           self = static_cast<GamepadSDL2*>( userdata );
        std::lock_guard lock( self->m_Mutex );

        if ( event->type == SDL_CONTROLLERDEVICEADDED )
        {
            int joyIdx = event->cdevice.which;
            if ( joyIdx < 0 || joyIdx >= Gamepad::MAX_PLAYER_COUNT )
                return 0;

            if ( !SDL_IsGameController( joyIdx ) )
                return 0;

            SDL_GameController* ctrl = SDL_GameControllerOpen( joyIdx );
            if ( ctrl )
            {
                for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
                {
                    if ( !self->m_Controllers[i] )
                    {
                        self->m_Controllers[i]    = ctrl;
                        self->m_MostRecentGamepad = i;
                        break;
                    }
                }
            }
        }
        else if ( event->type == SDL_CONTROLLERDEVICEREMOVED )
        {
            SDL_JoystickID joyId = event->cdevice.which;
            for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
            {
                if ( self->m_Controllers[i] && SDL_JoystickInstanceID( SDL_GameControllerGetJoystick( self->m_Controllers[i] ) ) == joyId )
                {
                    SDL_GameControllerClose( self->m_Controllers[i] );
                    self->m_Controllers[i] = nullptr;
                    break;
                }
            }
        }
        return 0;
    }

    GamepadSDL2()
    {
        if ( SDL_WasInit( SDL_INIT_GAMECONTROLLER ) == 0 )
            SDL_InitSubSystem( SDL_INIT_GAMECONTROLLER );

        // Initial scan for connected controllers
        int numJoysticks = SDL_NumJoysticks();
        int idx          = 0;
        for ( int i = 0; i < numJoysticks && idx < Gamepad::MAX_PLAYER_COUNT; ++i )
        {
            if ( SDL_IsGameController( i ) )
            {
                SDL_GameController* ctrl = SDL_GameControllerOpen( i );
                if ( ctrl )
                {
                    m_Controllers[idx++] = ctrl;
                    m_MostRecentGamepad  = idx - 1;
                }
            }
        }

        SDL_AddEventWatch( &SDLEventWatch, this );
    }

    ~GamepadSDL2()
    {
        SDL_DelEventWatch( &SDLEventWatch, this );

        for ( auto& ctrl: m_Controllers )
        {
            if ( ctrl )
            {
                SDL_GameControllerClose( ctrl );
                ctrl = nullptr;
            }
        }
    }

    std::array<SDL_GameController*, Gamepad::MAX_PLAYER_COUNT> m_Controllers       = {};
    int                                                        m_MostRecentGamepad = 0;
    mutable std::mutex                                         m_Mutex;
};

// Bridge to Gamepad interface
Gamepad::State Gamepad::getState( DeadZone deadZoneMode ) const
{
    return GamepadSDL2::get().getState( playerIndex, deadZoneMode );
}

bool Gamepad::setVibration( float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )
{
    return GamepadSDL2::get().setVibration( playerIndex, leftMotor, rightMotor, leftTrigger, rightTrigger );
}

void Gamepad::suspend() noexcept
{
    GamepadSDL2::get().suspend();
}

void Gamepad::resume() noexcept
{
    GamepadSDL2::get().resume();
}