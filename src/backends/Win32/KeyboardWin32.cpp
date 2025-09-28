#include <input/Keyboard.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstring>

using namespace input;

//======================================================================================
// Win32 desktop implementation
//======================================================================================

//
// For a Win32 desktop application, call this function from your Window Message Procedure
//
// void Keyboard_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
//
// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//
//     case WM_ACTIVATE:
//     case WM_ACTIVATEAPP:
//         Keyboard_ProcessMessage(message, wParam, lParam);
//         break;
//
//     case WM_KEYDOWN:
//     case WM_SYSKEYDOWN:
//     case WM_KEYUP:
//     case WM_SYSKEYUP:
//         Keyboard_ProcessMessage(message, wParam, lParam);
//         break;
//
//     }
// }
//

static_assert( sizeof( Keyboard::State ) == ( 256 / 8 ), "Size mismatch for State" );

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
}  // namespace

class KeyboardWin32
{
public:
    static KeyboardWin32& get()
    {
        static KeyboardWin32 keyboardWin32;
        return keyboardWin32;
    }

    Keyboard::State getState() const
    {
        return m_State;
    }

    void reset() noexcept
    {
        std::memset( &m_State, 0, sizeof( Keyboard::State ) );
    }

    bool isConnected() const
    {
        return true;
    }

    KeyboardWin32( const KeyboardWin32& )            = delete;
    KeyboardWin32( KeyboardWin32&& )                 = delete;
    KeyboardWin32& operator=( const KeyboardWin32& ) = delete;
    KeyboardWin32& operator=( KeyboardWin32&& )      = delete;

private:
    friend void Keyboard_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam ) noexcept;

    KeyboardWin32()  = default;
    ~KeyboardWin32() = default;

    Keyboard::State m_State {};
};

void Keyboard_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam ) noexcept
{
    auto& impl = KeyboardWin32::get();

    bool down = false;

    switch ( message )
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        impl.reset();
        return;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        down = true;
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        break;

    default:
        return;
    }

    int vk = LOWORD( wParam );
    // We want to distinguish left and right shift/ctrl/alt keys
    switch ( vk )
    {
    case VK_SHIFT:
    case VK_CONTROL:
    case VK_MENU:
    {
        if ( vk == VK_SHIFT && !down )
        {
            // Workaround to ensure left vs. right shift get cleared when both were pressed at same time
            KeyUp( VK_LSHIFT, impl.m_State );
            KeyUp( VK_RSHIFT, impl.m_State );
        }

        bool isExtendedKey = ( HIWORD( lParam ) & KF_EXTENDED ) == KF_EXTENDED;
        int  scanCode      = LOBYTE( HIWORD( lParam ) ) | ( isExtendedKey ? 0xe000 : 0 );
        vk                 = LOWORD( MapVirtualKeyW( static_cast<UINT>( scanCode ), MAPVK_VSC_TO_VK_EX ) );
    }
    break;

    default:
        break;
    }

    if ( down )
    {
        KeyDown( vk, impl.m_State );
    }
    else
    {
        KeyUp( vk, impl.m_State );
    }
}

namespace input::Keyboard
{
State getState()
{
    return KeyboardWin32::get().getState();
}

void reset()
{
    KeyboardWin32::get().reset();
}

bool isConnected()
{
    return KeyboardWin32::get().isConnected();
}

}  // namespace input::Keyboard
