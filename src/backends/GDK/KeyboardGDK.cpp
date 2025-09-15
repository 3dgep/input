#include <input/Keyboard.hpp>

#include <GameInput.h>
#include <wrl.h>

#include <format>
#include <iostream>
#include <stdexcept>

using namespace input;
using namespace Microsoft::WRL;

namespace
{
void KeyDown( int key, Keyboard::State& state ) noexcept
{
    if ( key < 0 || key > 0xfe )
        return;

    auto ptr = reinterpret_cast<uint32_t*>( &state );

    const unsigned int bf = 1u << ( key & 0x1f );
    ptr[( key >> 5 )] |= bf;
}

void KeyUp( int key, Keyboard::State& state ) noexcept
{
    if ( key < 0 || key > 0xfe )
        return;

    auto ptr = reinterpret_cast<uint32_t*>( &state );

    const unsigned int bf = 1u << ( key & 0x1f );
    ptr[( key >> 5 )] &= ~bf;
}
}


class KeyboardGDK
{
public:
    static KeyboardGDK& get()
    {
        static KeyboardGDK keyboardGDK;
        return keyboardGDK;
    }

    bool isConnected() const
    {
        return m_Connected > 0;
    }

    Keyboard::State getState() const
    {
        Keyboard::State state{};

        if ( !m_GameInput )
            return state;

        ComPtr<IGameInputReading> reading;
        if ( SUCCEEDED( m_GameInput->GetCurrentReading( GameInputKindKeyboard, nullptr, reading.GetAddressOf() ) ) )
        {
            GameInputKeyState keyState[256];
            uint32_t          readCount = reading->GetKeyState( 256, keyState );
            for ( size_t i = 0; i < readCount; ++i )
            {
                int vk = static_cast<int>( keyState[i].virtualKey );

                // Workaround for known issues with VK_RSHIFT and VK_NUMLOCK
                if ( vk == 0 )
                {
                    switch ( keyState[i].scanCode )
                    {
                    case 0xe036:
                        vk = VK_RSHIFT;
                        break;
                    case 0xe045:
                        vk = VK_NUMLOCK;
                        break;
                    default:
                        break;
                    }
                }

                KeyDown( vk, state );
            }
        }

        return state;
    }

    uint32_t m_Connected = 0;

private:
    static void CALLBACK OnGameInputDevice(
        _In_       GameInputCallbackToken,
        _In_ void* context,
        _In_ IGameInputDevice*,
        _In_                       uint64_t,
        _In_ GameInputDeviceStatus currentStatus,
        _In_ GameInputDeviceStatus previousStatus ) noexcept
    {
        auto impl = static_cast<KeyboardGDK*>( context );

        const bool wasConnected = ( previousStatus & GameInputDeviceConnected ) != 0;
        const bool isConnected  = ( currentStatus & GameInputDeviceConnected ) != 0;

        if ( isConnected && !wasConnected )
        {
            ++impl->m_Connected;
        }
        else if ( !isConnected && wasConnected && impl->m_Connected > 0 )
        {
            --impl->m_Connected;
        }
    }

    KeyboardGDK()
    {
        HRESULT hr = GameInputCreate( m_GameInput.GetAddressOf() );
        if ( SUCCEEDED( hr ) )
        {
            hr = m_GameInput->RegisterDeviceCallback(
                nullptr,
                GameInputKindKeyboard,
                GameInputDeviceConnected,
                GameInputBlockingEnumeration,
                this,
                OnGameInputDevice,
                &m_CallbackToken );

            if ( FAILED( hr ) )
            {
                throw std::runtime_error( std::format( "Failed to register keyboard device callback: {:08X}", static_cast<unsigned int>( hr ) ) );
            }
        }
        else
        {
            throw std::runtime_error( std::format( "Failed to create GameInput: {:08X}", static_cast<unsigned int>( hr ) ) );
        }
    }

    ~KeyboardGDK()
    {
        if ( m_CallbackToken && m_GameInput )
        {
            if ( !m_GameInput->UnregisterCallback( m_CallbackToken, UINT64_MAX ) )
            {
                std::cerr << "Failed to unregister device callback." << std::endl;
            }
        }
    }

    ComPtr<IGameInput>     m_GameInput;
    GameInputCallbackToken m_CallbackToken = 0;
};

Keyboard::State Keyboard::getState() const
{
    return KeyboardGDK::get().getState();
}

void Keyboard::reset()
{}

bool Keyboard::isConnected()
{
    return KeyboardGDK::get().isConnected();
}

void Keyboard_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam )
{
    // GameInput for Keyboard doesn't require Win32 messages, but this simplifies integration.
}
