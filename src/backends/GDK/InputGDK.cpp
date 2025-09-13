#include <input/Input.hpp>

#include <GameInput.h>
#include <wrl.h>

#include <cstring>  // for memcmp
#include <format>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

using namespace input;
using namespace Microsoft::WRL;

// Context for GDK implementation.
class InputGDK
{
public:
    void addGamepad( IGameInputDevice* gamepad );
    void removeGamepad( IGameInputDevice* gamepad );
    void setKeyboard( IGameInputDevice* keyboard );
    void setMouse( IGameInputDevice* mouse );

    void update();

    static InputGDK& get();

    InputGDK();
    ~InputGDK();

    ComPtr<IGameInput>     m_GameInput;
    GameInputCallbackToken m_DeviceCallbackToken;

    std::vector<ComPtr<IGameInputDevice>> m_Gamepads;
    ComPtr<IGameInputDevice>              m_Keyboard;
    ComPtr<IGameInputDevice>              m_Mouse;

};

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
    m_Gamepads.push_back( gamepad );
}

void InputGDK::removeGamepad( IGameInputDevice* gamepad )
{
    std::erase_if( m_Gamepads, [gamepad]( const ComPtr<IGameInputDevice>& d ) {
        return std::memcmp( d->GetDeviceInfo()->deviceId.value, gamepad->GetDeviceInfo()->deviceId.value, APP_LOCAL_DEVICE_ID_SIZE ) == 0;
    } );
}

void InputGDK::setKeyboard( IGameInputDevice* keyboard )
{
    m_Keyboard = keyboard;
}

void InputGDK::setMouse( IGameInputDevice* mouse )
{
    m_Mouse = mouse;
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
        for ( auto& device: m_Gamepads )
        {
            ComPtr<IGameInputReading> reading;
            HRESULT                   hr = m_GameInput->GetCurrentReading( GameInputKindGamepad, device.Get(), &reading );
            if ( SUCCEEDED( hr ) )
            {
                GameInputGamepadState gameInputGamepadState;
                if ( reading->GetGamepadState( &gameInputGamepadState ) )
                {

                }
            }
        }
    }
}

void Input::update()
{
    InputGDK::get().update();
}
