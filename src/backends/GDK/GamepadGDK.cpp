#include <input/Gamepad.hpp>

#include <GameInput.h>
#include <wrl.h>

#include <cassert>
#include <format>
#include <iostream>
#include <stdexcept>

#pragma comment( lib, "gameinput.lib" )

using namespace input;
using Microsoft::WRL::ComPtr;

const int Gamepad::MAX_PLAYER_COUNT = 8;

constexpr float XboxOneThumbDeadZone = .24f;  // Recommended Xbox One controller deadzone

// Source (September 15, 2025): https://github.com/microsoft/DirectXTK/blob/main/Src/GamePad.cpp
class GamepadGDK
{
public:
    static GamepadGDK& get()
    {
        static GamepadGDK gamepadGDK;
        return gamepadGDK;
    }

    Gamepad::State getState( int player, Gamepad::DeadZone deadZoneMode )
    {
        Gamepad::State state = {};

        if ( !m_GameInput )
            return state;

        IGameInputDevice* device = nullptr;

        if ( player >= 0 && player < Gamepad::MAX_PLAYER_COUNT )
        {
            device = m_InputDevices[player].Get();
            if ( !device )
                return state;
        }
        else if ( player == Gamepad::MOST_RECENT_PLAYER )
        {
            player = m_MostRecentGamepad;
            assert( player >= 0 && player < Gamepad::MAX_PLAYER_COUNT );

            device = m_InputDevices[player].Get();
            if ( !device )
                return state;
        }

        ComPtr<IGameInputReading> reading;
        if ( SUCCEEDED( m_GameInput->GetCurrentReading( GameInputKindGamepad, device, reading.GetAddressOf() ) ) )
        {
            GameInputGamepadState pad;
            if ( reading->GetGamepadState( &pad ) )
            {
                state.connected = true;
                state.packet    = reading->GetTimestamp();

                state.buttons.a             = ( pad.buttons & GameInputGamepadA ) != 0;
                state.buttons.b             = ( pad.buttons & GameInputGamepadB ) != 0;
                state.buttons.x             = ( pad.buttons & GameInputGamepadX ) != 0;
                state.buttons.y             = ( pad.buttons & GameInputGamepadY ) != 0;
                state.buttons.leftStick     = ( pad.buttons & GameInputGamepadLeftThumbstick ) != 0;
                state.buttons.rightStick    = ( pad.buttons & GameInputGamepadRightThumbstick ) != 0;
                state.buttons.leftShoulder  = ( pad.buttons & GameInputGamepadLeftShoulder ) != 0;
                state.buttons.rightShoulder = ( pad.buttons & GameInputGamepadRightShoulder ) != 0;
                state.buttons.view          = ( pad.buttons & GameInputGamepadView ) != 0;
                state.buttons.menu          = ( pad.buttons & GameInputGamepadMenu ) != 0;

                state.dPad.up    = ( pad.buttons & GameInputGamepadDPadUp ) != 0;
                state.dPad.down  = ( pad.buttons & GameInputGamepadDPadDown ) != 0;
                state.dPad.right = ( pad.buttons & GameInputGamepadDPadRight ) != 0;
                state.dPad.left  = ( pad.buttons & GameInputGamepadDPadLeft ) != 0;

                ApplyStickDeadZone( pad.leftThumbstickX, pad.leftThumbstickY,
                                    deadZoneMode, 1.0f, XboxOneThumbDeadZone,
                                    state.thumbSticks.leftX, state.thumbSticks.leftY );

                ApplyStickDeadZone( pad.rightThumbstickX, pad.rightThumbstickY,
                                    deadZoneMode, 1.0f, XboxOneThumbDeadZone,
                                    state.thumbSticks.rightX, state.thumbSticks.rightY );

                state.triggers.left  = pad.leftTrigger;
                state.triggers.right = pad.rightTrigger;
            }
        }

        return state;
    }

    bool setVibration( int player, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger ) noexcept
    {
        if ( player == Gamepad::MOST_RECENT_PLAYER )
            player = m_MostRecentGamepad;

        if ( player >= 0 && player < Gamepad::MAX_PLAYER_COUNT )
        {
            IGameInputDevice* device = m_InputDevices[player].Get();
            if ( device )
            {
                const GameInputRumbleParams params {
                    leftMotor, rightMotor, leftTrigger, rightTrigger
                };

                device->SetRumbleState( &params );
                return true;
            }
        }

        return false;
    }

    void suspend() noexcept
    {
        for ( const auto& inputDevice: m_InputDevices )
        {
            if ( IGameInputDevice* device = inputDevice.Get() )
            {
                device->SetRumbleState( nullptr );
            }
        }
    }

    void resume() noexcept
    {
        for ( auto& inputDevice: m_InputDevices )
        {
            if ( IGameInputDevice* device = inputDevice.Get() )
            {
                if ( !( device->GetDeviceStatus() & GameInputDeviceConnected ) )
                {
                    inputDevice.Reset();
                }
            }
        }
    }

    GamepadGDK( const GamepadGDK& )            = delete;
    GamepadGDK( GamepadGDK&& )                 = delete;
    GamepadGDK& operator=( const GamepadGDK& ) = delete;
    GamepadGDK& operator=( GamepadGDK&& )      = delete;

    HANDLE m_CtrlChanged { INVALID_HANDLE_VALUE };

private:
    static void CALLBACK OnGameInputDevice(
        GameInputCallbackToken,
        void*             context,
        IGameInputDevice* device,
        uint64_t,
        GameInputDeviceStatus currentStatus,
        GameInputDeviceStatus ) noexcept
    {
        auto impl = static_cast<GamepadGDK*>( context );

        if ( currentStatus & GameInputDeviceConnected )
        {
            size_t empty = Gamepad::MAX_PLAYER_COUNT;
            size_t k     = 0;
            for ( ; k < Gamepad::MAX_PLAYER_COUNT; ++k )
            {
                if ( impl->m_InputDevices[k].Get() == device )
                {
                    impl->m_MostRecentGamepad = static_cast<int>( k );
                    break;
                }
                if ( !impl->m_InputDevices[k] )
                {
                    if ( empty >= Gamepad::MAX_PLAYER_COUNT )
                        empty = k;
                }
            }

            if ( k >= Gamepad::MAX_PLAYER_COUNT )
            {
                // Silently ignore "extra" gamepads as there's no hard limit
                if ( empty < Gamepad::MAX_PLAYER_COUNT )
                {
                    impl->m_InputDevices[empty] = device;
                    impl->m_MostRecentGamepad   = static_cast<int>( empty );
                }
            }
        }
        else
        {
            for ( size_t k = 0; k < Gamepad::MAX_PLAYER_COUNT; ++k )
            {
                if ( impl->m_InputDevices[k].Get() == device )
                {
                    impl->m_InputDevices[k].Reset();
                    break;
                }
            }
        }

        if ( impl->m_CtrlChanged != INVALID_HANDLE_VALUE )
        {
            SetEvent( impl->m_CtrlChanged );
        }
    }

    GamepadGDK()
    {
        HRESULT hr = GameInputCreate( m_GameInput.GetAddressOf() );
        if ( SUCCEEDED( hr ) )
        {
            hr = m_GameInput->RegisterDeviceCallback(
                nullptr,
                GameInputKindGamepad,
                GameInputDeviceConnected,
                GameInputBlockingEnumeration,
                this,
                OnGameInputDevice,
                &m_CallbackToken );

            if ( FAILED( hr ) )
            {
                throw std::runtime_error( std::format( "Failed to register gamepad device callback: {:08X}", static_cast<unsigned int>( hr ) ) );
            }
        }
        else
        {
            throw std::runtime_error( std::format( "Failed to create GameInput: {:08X}", static_cast<unsigned int>( hr ) ) );
        }
    }
    ~GamepadGDK()
    {
        if ( m_CallbackToken && m_GameInput )
        {
            if ( !m_GameInput->UnregisterCallback( m_CallbackToken, UINT64_MAX ) )
            {
                std::cerr << "Failed to unregister device callback." << std::endl;
            }
        }
    }

    ComPtr<IGameInput>       m_GameInput;
    ComPtr<IGameInputDevice> m_InputDevices[Gamepad::MAX_PLAYER_COUNT];
    GameInputCallbackToken   m_CallbackToken { 0ull };
    int                      m_MostRecentGamepad = 0;
};

Gamepad::State Gamepad::getState( int playerIndex, DeadZone deadZoneMode )
{
    return GamepadGDK::get().getState( playerIndex, deadZoneMode );
}

bool Gamepad::setVibration( int playerIndex, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )
{
    return GamepadGDK::get().setVibration( playerIndex, leftMotor, rightMotor, leftTrigger, rightTrigger );
}

void Gamepad::suspend() noexcept
{
    GamepadGDK::get().suspend();
}

void Gamepad::resume() noexcept
{
    GamepadGDK::get().resume();
}
