#include "InputGDK.hpp"

#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace input;

// Based on sample: https://github.dev/microsoft/Xbox-GDK-Samples/blob/main/Samples/System/GamepadKeyboardMouse/GamepadKeyboardMouse.cpp

void CALLBACK DeviceCallback(
    GameInputCallbackToken callbackToken,
    void*                  context,
    IGameInputDevice*      device,
    uint64_t               timestamp,
    GameInputDeviceStatus  currentStatus,
    GameInputDeviceStatus  previousStatus ) noexcept
{
    const GameInputDeviceInfo* info       = device->GetDeviceInfo();
    GameInputKind              deviceKind = info->supportedInput;

    std::string message;

    if ( ( previousStatus & GameInputDeviceConnected ) == 0 && ( currentStatus & GameInputDeviceConnected ) != 0 )
    {
        if ( ( deviceKind & GameInputKindGamepad ) != 0 )
        {
            message = "Gamepad";
            static_cast<InputGDK*>( context )->addGamepad( device );
        }
        else if ( ( deviceKind & GameInputKindKeyboard ) != 0 )
        {
            message = "Keyboard";
        }
        else if ( ( deviceKind & GameInputKindMouse ) != 0 )
        {
            message = "Mouse";
        }        

        message += " Connected.";
    }
    else if ( ( previousStatus & GameInputDeviceConnected ) != 0 && ( currentStatus & GameInputDeviceConnected ) == 0 )
    {
        if ( ( deviceKind & GameInputKindGamepad ) != 0 )
        {
            message = "Gamepad";
            static_cast<InputGDK*>( context )->removeGamepad( device );
        }
        else if ( ( deviceKind & GameInputKindKeyboard ) != 0 )
        {
            message = "Keyboard";
        }
        else if ( ( deviceKind & GameInputKindMouse ) != 0 )
        {
            message = "Mouse";
        }

        message += " Disconnected.";
    }

    std::cout << message << std::endl;
}

InputGDK::InputGDK()
{
    HRESULT hr = GameInputCreate( &m_GameInput );
    if ( FAILED( hr ) )
    {
        throw std::runtime_error( std::format( "Failed to create GameInput: {}", hr ) );
    }

    hr = m_GameInput->RegisterDeviceCallback( nullptr,
                                              GameInputKindGamepad | GameInputKindKeyboard | GameInputKindMouse,
                                              GameInputDeviceAnyStatus,
                                              GameInputAsyncEnumeration,
                                              this,
                                              &DeviceCallback,
                                              &m_DeviceCallbackToken );

    if ( FAILED( hr ) )
    {
        throw std::runtime_error( std::format( "Failed to register device callback: {}", hr ) );
    }
}

InputGDK::~InputGDK()
{
    m_GameInput->UnregisterCallback( m_DeviceCallbackToken, UINT64_MAX );
}

void InputGDK::addGamepad( IGameInputDevice* gamepad )
{
    m_Gamepads.insert( { gamepad->GetDeviceInfo()->deviceId, gamepad } );
}

void InputGDK::removeGamepad( IGameInputDevice* gamepad )
{
    m_Gamepads.erase( gamepad->GetDeviceInfo()->deviceId );
}

InputGDK& InputGDK::get()
{
    static InputGDK input;
    return input;
}

void InputGDK::update()
{

    // Gamepad input.
    {
        HRESULT hr = m_GameInput->GetCurrentReading( GameInputKindGamepad, nullptr, &reading );
        if ( SUCCEEDED( hr ) )
        {
            GameInputGamepadState gameInputGamepadState;
            if ( reading->GetGamepadState( &gameInputGamepadState ) )
            {
                GamepadState state {};
                
            }
        }
    }
}

InputGDK& getInputGDK()
{
    static InputGDK inputGDK;
    return inputGDK;
}

void Input::update()
{
}
