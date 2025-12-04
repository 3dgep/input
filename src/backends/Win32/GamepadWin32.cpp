#include <input/Gamepad.hpp>

#include <windows.gaming.input.h>
#include <wrl.h>

#include <memory>
#include <system_error>

#pragma comment( lib, "runtimeobject.lib" )

using namespace input;
using namespace Microsoft::WRL;

constexpr float XboxOneThumbDeadZone = .24f;  // Recommended Xbox One controller deadzone

namespace
{
struct handle_closer
{
    void operator()( HANDLE h ) const noexcept
    {
        if ( h )
            CloseHandle( h );
    }
};

using ScopedHandle = std::unique_ptr<void, handle_closer>;

// Helper class for COM exceptions
class com_exception : public std::exception
{
public:
    com_exception( HRESULT hr ) noexcept
    : result( hr )
    {}

    const char* what() const noexcept override
    {
        static char s_str[64] = {};
        std::ignore           = sprintf_s( s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>( result ) );
        return s_str;
    }

    HRESULT get_result() const noexcept
    {
        return result;
    }

private:
    HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
void ThrowIfFailed( HRESULT hr ) noexcept( false )
{
    if ( FAILED( hr ) )
    {
        throw com_exception( hr );
    }
}

}  // namespace

// Source (September 15, 2025): https://github.com/microsoft/DirectXTK/blob/main/Src/GamePad.cpp
class GamepadWin32
{
public:
    static GamepadWin32& get()
    {
        static GamepadWin32 gamepadWin32;
        return gamepadWin32;
    }

    Gamepad::State getState( int player, Gamepad::DeadZone deadZoneMode )
    {
        using namespace Microsoft::WRL;
        using namespace ABI::Windows::Gaming::Input;

        if ( WaitForSingleObjectEx( m_Changed.get(), 0, FALSE ) == WAIT_OBJECT_0 )
        {
            scanGamePads();
        }

        if ( player == input::Gamepad::MOST_RECENT_PLAYER )
            player = m_MostRecentGamepad;

        input::Gamepad::State state {};

        if ( ( player >= 0 ) && ( player < input::Gamepad::MAX_PLAYER_COUNT ) )
        {
            if ( m_Gamepad[player] )
            {
                GamepadReading reading;
                HRESULT        hr = m_Gamepad[player]->GetCurrentReading( &reading );
                if ( SUCCEEDED( hr ) )
                {
                    state.connected = true;
                    state.packet    = reading.Timestamp;

                    state.buttons.a = ( reading.Buttons & GamepadButtons_A ) != 0;
                    state.buttons.b = ( reading.Buttons & GamepadButtons_B ) != 0;
                    state.buttons.x = ( reading.Buttons & GamepadButtons_X ) != 0;
                    state.buttons.y = ( reading.Buttons & GamepadButtons_Y ) != 0;

                    state.buttons.leftStick  = ( reading.Buttons & GamepadButtons_LeftThumbstick ) != 0;
                    state.buttons.rightStick = ( reading.Buttons & GamepadButtons_RightThumbstick ) != 0;

                    state.buttons.leftShoulder  = ( reading.Buttons & GamepadButtons_LeftShoulder ) != 0;
                    state.buttons.rightShoulder = ( reading.Buttons & GamepadButtons_RightShoulder ) != 0;

                    state.buttons.view = ( reading.Buttons & GamepadButtons_View ) != 0;
                    state.buttons.menu = ( reading.Buttons & GamepadButtons_Menu ) != 0;

                    state.dPad.up    = ( reading.Buttons & GamepadButtons_DPadUp ) != 0;
                    state.dPad.down  = ( reading.Buttons & GamepadButtons_DPadDown ) != 0;
                    state.dPad.right = ( reading.Buttons & GamepadButtons_DPadRight ) != 0;
                    state.dPad.left  = ( reading.Buttons & GamepadButtons_DPadLeft ) != 0;

                    ApplyStickDeadZone( static_cast<float>( reading.LeftThumbstickX ), static_cast<float>( reading.LeftThumbstickY ),
                                        deadZoneMode, 1.f, XboxOneThumbDeadZone,
                                        state.thumbSticks.leftX, state.thumbSticks.leftY );

                    ApplyStickDeadZone( static_cast<float>( reading.RightThumbstickX ), static_cast<float>( reading.RightThumbstickY ),
                                        deadZoneMode, 1.f, XboxOneThumbDeadZone,
                                        state.thumbSticks.rightX, state.thumbSticks.rightY );

                    state.triggers.left  = static_cast<float>( reading.LeftTrigger );
                    state.triggers.right = static_cast<float>( reading.RightTrigger );
                }
            }
        }

        return state;
    }

    bool setVibration( int player, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger ) const noexcept
    {
        using namespace ABI::Windows::Gaming::Input;

        if ( player == input::Gamepad::MOST_RECENT_PLAYER )
            player = m_MostRecentGamepad;

        if ( ( player >= 0 ) && ( player < input::Gamepad::MAX_PLAYER_COUNT ) )
        {
            if ( m_Gamepad[player] )
            {
                GamepadVibration vib;
                vib.LeftMotor    = static_cast<double>( leftMotor );
                vib.RightMotor   = static_cast<double>( rightMotor );
                vib.LeftTrigger  = static_cast<double>( leftTrigger );
                vib.RightTrigger = static_cast<double>( rightTrigger );
                HRESULT hr       = m_Gamepad[player]->put_Vibration( vib );

                if ( SUCCEEDED( hr ) )
                    return true;
            }
        }

        return false;
    }

    void suspend() noexcept
    {
        for ( auto& gamepad: m_Gamepad )
        {
            gamepad.Reset();
        }
    }

    void resume() const noexcept
    {
        // Make sure we rescan gamepads
        SetEvent( m_Changed.get() );
    }

    void registerEvents( HANDLE ctrlChanged, HANDLE userChanged ) noexcept
    {
        m_CtrlChanged = ( !ctrlChanged ) ? INVALID_HANDLE_VALUE : ctrlChanged;
        m_UserChanged = ( !userChanged ) ? INVALID_HANDLE_VALUE : userChanged;
    }

private:
    static HRESULT gamepadAdded( IInspectable*, ABI::Windows::Gaming::Input::IGamepad* )
    {
        GamepadWin32& impl = get();

        SetEvent( impl.m_Changed.get() );

        if ( impl.m_CtrlChanged != INVALID_HANDLE_VALUE )
        {
            SetEvent( impl.m_CtrlChanged );
        }

        return S_OK;
    }

    static HRESULT gamepadRemoved( IInspectable*, ABI::Windows::Gaming::Input::IGamepad* )
    {
        GamepadWin32& impl = get();

        SetEvent( impl.m_Changed.get() );

        if ( impl.m_CtrlChanged != INVALID_HANDLE_VALUE )
        {
            SetEvent( impl.m_CtrlChanged );
        }

        return S_OK;
    }

    static HRESULT userChanged( ABI::Windows::Gaming::Input::IGameController*, ABI::Windows::System::IUserChangedEventArgs* )
    {
        GamepadWin32& impl = get();

        if ( impl.m_UserChanged != INVALID_HANDLE_VALUE )
        {
            SetEvent( impl.m_UserChanged );
        }

        return S_OK;
    }

    void scanGamePads()
    {
        using ABI::Windows::Foundation::Collections::IVectorView;
        using ABI::Windows::Gaming::Input::Gamepad, ABI::Windows::Gaming::Input::IGamepad, ABI::Windows::Gaming::Input::IGameController;

        ComPtr<IVectorView<Gamepad*>> pads;

        ThrowIfFailed( m_Statics->get_Gamepads( pads.GetAddressOf() ) );

        unsigned int count = 0;
        ThrowIfFailed( pads->get_Size( &count ) );

        // Check for removed gamepads
        for ( size_t j = 0; j < input::Gamepad::MAX_PLAYER_COUNT; ++j )
        {
            if ( m_Gamepad[j] )
            {
                unsigned int k = 0;
                for ( ; k < count; ++k )
                {
                    ComPtr<IGamepad> pad;
                    HRESULT          hr = pads->GetAt( k, pad.GetAddressOf() );
                    if ( SUCCEEDED( hr ) && ( pad == m_Gamepad[j] ) )
                    {
                        break;
                    }
                }

                if ( k >= count )
                {
                    ComPtr<IGameController> ctrl;
                    HRESULT                 hr = m_Gamepad[j].As( &ctrl );

                    if ( SUCCEEDED( hr ) && ctrl )
                    {
                        std::ignore                = ctrl->remove_UserChanged( m_UserChangeToken[j] );
                        m_UserChangeToken[j].value = 0;
                    }

                    m_Gamepad[j].Reset();
                }
            }
        }

        // Check for added gamepads
        for ( unsigned int j = 0; j < count; ++j )
        {
            ComPtr<IGamepad> pad;
            HRESULT          hr = pads->GetAt( j, pad.GetAddressOf() );
            if ( SUCCEEDED( hr ) )
            {
                size_t empty = input::Gamepad::MAX_PLAYER_COUNT;
                size_t k     = 0;
                for ( ; k < input::Gamepad::MAX_PLAYER_COUNT; ++k )
                {
                    if ( m_Gamepad[k] == pad )
                    {
                        if ( j == ( count - 1 ) )
                            m_MostRecentGamepad = static_cast<int>( k );
                        break;
                    }
                    if ( !m_Gamepad[k] )
                    {
                        if ( empty >= input::Gamepad::MAX_PLAYER_COUNT )
                            empty = k;
                    }
                }

                if ( k >= input::Gamepad::MAX_PLAYER_COUNT )
                {
                    // Silently ignore "extra" gamepads as there's no hard limit
                    if ( empty < input::Gamepad::MAX_PLAYER_COUNT )
                    {
                        m_Gamepad[empty] = pad;
                        if ( j == ( count - 1 ) )
                            m_MostRecentGamepad = static_cast<int>( empty );

                        ComPtr<IGameController> ctrl;
                        hr = pad.As( &ctrl );
                        if ( SUCCEEDED( hr ) && ctrl )
                        {
                            typedef __FITypedEventHandler_2_Windows__CGaming__CInput__CIGameController_Windows__CSystem__CUserChangedEventArgs UserHandler;
                            ThrowIfFailed( ctrl->add_UserChanged( Callback<UserHandler>( userChanged ).Get(), &m_UserChangeToken[empty] ) );
                        }
                    }
                }
            }
        }
    }

    GamepadWin32()
    {
        using namespace Microsoft::WRL::Wrappers;
        using namespace ABI::Windows::Foundation;

        m_Changed.reset( CreateEventEx( nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE ) );

        if ( !m_Changed )
        {
            throw std::system_error( std::error_code( static_cast<int>( GetLastError() ), std::system_category() ), "CreateEventEx" );
        }

        ThrowIfFailed( GetActivationFactory( HStringReference( RuntimeClass_Windows_Gaming_Input_Gamepad ).Get(), m_Statics.GetAddressOf() ) );

        typedef __FIEventHandler_1_Windows__CGaming__CInput__CGamepad AddedHandler;
        ThrowIfFailed( m_Statics->add_GamepadAdded( Callback<AddedHandler>( gamepadAdded ).Get(), &m_AddedToken ) );

        typedef __FIEventHandler_1_Windows__CGaming__CInput__CGamepad RemovedHandler;
        ThrowIfFailed( m_Statics->add_GamepadRemoved( Callback<RemovedHandler>( gamepadRemoved ).Get(), &m_RemovedToken ) );

        scanGamePads();
    }

    ~GamepadWin32()
    {
        using namespace ABI::Windows::Gaming::Input;

        for ( size_t j = 0; j < input::Gamepad::MAX_PLAYER_COUNT; ++j )
        {
            if ( m_Gamepad[j] )
            {
                ComPtr<IGameController> ctrl;
                HRESULT                 hr = m_Gamepad[j].As( &ctrl );
                if ( SUCCEEDED( hr ) && ctrl )
                {
                    std::ignore                = ctrl->remove_UserChanged( m_UserChangeToken[j] );
                    m_UserChangeToken[j].value = 0;
                }

                m_Gamepad[j].Reset();
            }
        }

        if ( m_Statics )
        {
            std::ignore        = m_Statics->remove_GamepadAdded( m_AddedToken );
            m_AddedToken.value = 0;

            std::ignore          = m_Statics->remove_GamepadRemoved( m_RemovedToken );
            m_RemovedToken.value = 0;

            m_Statics.Reset();
        }
    }

    int m_MostRecentGamepad = 0;

    HANDLE       m_CtrlChanged { INVALID_HANDLE_VALUE };
    HANDLE       m_UserChanged { INVALID_HANDLE_VALUE };
    ScopedHandle m_Changed;

    ComPtr<ABI::Windows::Gaming::Input::IGamepadStatics> m_Statics;
    ComPtr<ABI::Windows::Gaming::Input::IGamepad>        m_Gamepad[Gamepad::MAX_PLAYER_COUNT];

    EventRegistrationToken m_UserChangeToken[Gamepad::MAX_PLAYER_COUNT] {};
    EventRegistrationToken m_AddedToken {};
    EventRegistrationToken m_RemovedToken {};
};

Gamepad::State Gamepad::getState( int playerIndex, DeadZone deadZoneMode )
{
    return GamepadWin32::get().getState( playerIndex, deadZoneMode );
}

bool Gamepad::setVibration( int playerIndex, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )
{
    return GamepadWin32::get().setVibration( playerIndex, leftMotor, rightMotor, leftTrigger, rightTrigger );
}

void Gamepad::suspend() noexcept
{
    GamepadWin32::get().suspend();
}

void Gamepad::resume() noexcept
{
    GamepadWin32::get().resume();
}


