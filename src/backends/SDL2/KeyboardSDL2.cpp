#include <input/Keyboard.hpp>

#include <SDL.h>

#include <cstring>
#include <mutex>

using namespace input;

namespace
{
void KeyDown( int key, Keyboard::State& state ) noexcept
{
    if ( key < 0 || key > 0xfe )
        return;

    auto               ptr = reinterpret_cast<uint32_t*>( &state );
    const unsigned int bf  = 1u << ( key & 0x1f );
    ptr[( key >> 5 )] |= bf;
}

void KeyUp( int key, Keyboard::State& state ) noexcept
{
    if ( key < 0 || key > 0xfe )
        return;

    auto               ptr = reinterpret_cast<uint32_t*>( &state );
    const unsigned int bf  = 1u << ( key & 0x1f );
    ptr[( key >> 5 )] &= ~bf;
}

// Map SDL scancode to Win32 virtual key code (VK_*)
int SDLScancodeToVirtualKey( SDL_Scancode scancode )
{
    switch ( scancode )
    {
    // Mouse buttons (not handled by SDL keyboard events)
    // case SDL_SCANCODE_MOUSE_LEFT: return 0x01; // LeftButton
    // case SDL_SCANCODE_MOUSE_RIGHT: return 0x02; // RightButton
    // case SDL_SCANCODE_MOUSE_MIDDLE: return 0x04; // MButton
    // case SDL_SCANCODE_MOUSE_X1: return 0x05; // XButton1
    // case SDL_SCANCODE_MOUSE_X2: return 0x06; // XButton2

    // Control keys
    case SDL_SCANCODE_BACKSPACE:
        return 0x08;  // Back
    case SDL_SCANCODE_TAB:
        return 0x09;  // Tab
    case SDL_SCANCODE_CLEAR:
        return 0x0C;  // Clear
    case SDL_SCANCODE_RETURN:
        return 0x0D;  // Enter
    case SDL_SCANCODE_PAUSE:
        return 0x13;  // Pause
    case SDL_SCANCODE_CAPSLOCK:
        return 0x14;  // CapsLock
    case SDL_SCANCODE_ESCAPE:
        return 0x1B;  // Escape
    case SDL_SCANCODE_SPACE:
        return 0x20;  // Space
    case SDL_SCANCODE_PAGEUP:
        return 0x21;  // PageUp
    case SDL_SCANCODE_PAGEDOWN:
        return 0x22;  // PageDown
    case SDL_SCANCODE_END:
        return 0x23;  // End
    case SDL_SCANCODE_HOME:
        return 0x24;  // Home
    case SDL_SCANCODE_LEFT:
        return 0x25;  // Left
    case SDL_SCANCODE_UP:
        return 0x26;  // Up
    case SDL_SCANCODE_RIGHT:
        return 0x27;  // Right
    case SDL_SCANCODE_DOWN:
        return 0x28;  // Down
    case SDL_SCANCODE_SELECT:
        return 0x29;  // Select
    case SDL_SCANCODE_PRINTSCREEN:
        return 0x2C;  // PrintScreen
    case SDL_SCANCODE_INSERT:
        return 0x2D;  // Insert
    case SDL_SCANCODE_DELETE:
        return 0x2E;  // Delete
    case SDL_SCANCODE_HELP:
        return 0x2F;  // Help

    // Number keys
    case SDL_SCANCODE_0:
        return 0x30;
    case SDL_SCANCODE_1:
        return 0x31;
    case SDL_SCANCODE_2:
        return 0x32;
    case SDL_SCANCODE_3:
        return 0x33;
    case SDL_SCANCODE_4:
        return 0x34;
    case SDL_SCANCODE_5:
        return 0x35;
    case SDL_SCANCODE_6:
        return 0x36;
    case SDL_SCANCODE_7:
        return 0x37;
    case SDL_SCANCODE_8:
        return 0x38;
    case SDL_SCANCODE_9:
        return 0x39;

    // Letter keys
    case SDL_SCANCODE_A:
        return 0x41;
    case SDL_SCANCODE_B:
        return 0x42;
    case SDL_SCANCODE_C:
        return 0x43;
    case SDL_SCANCODE_D:
        return 0x44;
    case SDL_SCANCODE_E:
        return 0x45;
    case SDL_SCANCODE_F:
        return 0x46;
    case SDL_SCANCODE_G:
        return 0x47;
    case SDL_SCANCODE_H:
        return 0x48;
    case SDL_SCANCODE_I:
        return 0x49;
    case SDL_SCANCODE_J:
        return 0x4A;
    case SDL_SCANCODE_K:
        return 0x4B;
    case SDL_SCANCODE_L:
        return 0x4C;
    case SDL_SCANCODE_M:
        return 0x4D;
    case SDL_SCANCODE_N:
        return 0x4E;
    case SDL_SCANCODE_O:
        return 0x4F;
    case SDL_SCANCODE_P:
        return 0x50;
    case SDL_SCANCODE_Q:
        return 0x51;
    case SDL_SCANCODE_R:
        return 0x52;
    case SDL_SCANCODE_S:
        return 0x53;
    case SDL_SCANCODE_T:
        return 0x54;
    case SDL_SCANCODE_U:
        return 0x55;
    case SDL_SCANCODE_V:
        return 0x56;
    case SDL_SCANCODE_W:
        return 0x57;
    case SDL_SCANCODE_X:
        return 0x58;
    case SDL_SCANCODE_Y:
        return 0x59;
    case SDL_SCANCODE_Z:
        return 0x5A;

    // Windows keys
    case SDL_SCANCODE_LGUI:
        return 0x5B;  // LeftWindows
    case SDL_SCANCODE_RGUI:
        return 0x5C;  // RightWindows
    case SDL_SCANCODE_APPLICATION:
        return 0x5D;  // Apps

    // Numpad keys
    case SDL_SCANCODE_KP_0:
        return 0x60;
    case SDL_SCANCODE_KP_1:
        return 0x61;
    case SDL_SCANCODE_KP_2:
        return 0x62;
    case SDL_SCANCODE_KP_3:
        return 0x63;
    case SDL_SCANCODE_KP_4:
        return 0x64;
    case SDL_SCANCODE_KP_5:
        return 0x65;
    case SDL_SCANCODE_KP_6:
        return 0x66;
    case SDL_SCANCODE_KP_7:
        return 0x67;
    case SDL_SCANCODE_KP_8:
        return 0x68;
    case SDL_SCANCODE_KP_9:
        return 0x69;
    case SDL_SCANCODE_KP_MULTIPLY:
        return 0x6A;
    case SDL_SCANCODE_KP_PLUS:
        return 0x6B;
    case SDL_SCANCODE_KP_ENTER:
        return 0x6C;
    case SDL_SCANCODE_KP_MINUS:
        return 0x6D;
    case SDL_SCANCODE_KP_DECIMAL:
        return 0x6E;
    case SDL_SCANCODE_KP_DIVIDE:
        return 0x6F;

    // Function keys
    case SDL_SCANCODE_F1:
        return 0x70;
    case SDL_SCANCODE_F2:
        return 0x71;
    case SDL_SCANCODE_F3:
        return 0x72;
    case SDL_SCANCODE_F4:
        return 0x73;
    case SDL_SCANCODE_F5:
        return 0x74;
    case SDL_SCANCODE_F6:
        return 0x75;
    case SDL_SCANCODE_F7:
        return 0x76;
    case SDL_SCANCODE_F8:
        return 0x77;
    case SDL_SCANCODE_F9:
        return 0x78;
    case SDL_SCANCODE_F10:
        return 0x79;
    case SDL_SCANCODE_F11:
        return 0x7A;
    case SDL_SCANCODE_F12:
        return 0x7B;
    case SDL_SCANCODE_F13:
        return 0x7C;
    case SDL_SCANCODE_F14:
        return 0x7D;
    case SDL_SCANCODE_F15:
        return 0x7E;
    case SDL_SCANCODE_F16:
        return 0x7F;
    case SDL_SCANCODE_F17:
        return 0x80;
    case SDL_SCANCODE_F18:
        return 0x81;
    case SDL_SCANCODE_F19:
        return 0x82;
    case SDL_SCANCODE_F20:
        return 0x83;
    case SDL_SCANCODE_F21:
        return 0x84;
    case SDL_SCANCODE_F22:
        return 0x85;
    case SDL_SCANCODE_F23:
        return 0x86;
    case SDL_SCANCODE_F24:
        return 0x87;

    // Lock keys
    case SDL_SCANCODE_NUMLOCKCLEAR:
        return 0x90;  // NumLock
    case SDL_SCANCODE_SCROLLLOCK:
        return 0x91;  // Scroll

    // Modifier keys
    case SDL_SCANCODE_LSHIFT:
        return 0xA0;  // LeftShift
    case SDL_SCANCODE_RSHIFT:
        return 0xA1;  // RightShift
    case SDL_SCANCODE_LCTRL:
        return 0xA2;  // LeftControl
    case SDL_SCANCODE_RCTRL:
        return 0xA3;  // RightControl
    case SDL_SCANCODE_LALT:
        return 0xA4;  // LeftAlt
    case SDL_SCANCODE_RALT:
        return 0xA5;  // RightAlt

    // Browser/media keys
    case SDL_SCANCODE_AC_BACK:
        return 0xA6;  // BrowserBack
    case SDL_SCANCODE_AC_FORWARD:
        return 0xA7;  // BrowserForward
    case SDL_SCANCODE_AC_REFRESH:
        return 0xA8;  // BrowserRefresh
    case SDL_SCANCODE_AC_STOP:
        return 0xA9;  // BrowserStop
    case SDL_SCANCODE_AC_SEARCH:
        return 0xAA;  // BrowserSearch
    case SDL_SCANCODE_AC_BOOKMARKS:
        return 0xAB;  // BrowserFavorites
    case SDL_SCANCODE_AC_HOME:
        return 0xAC;  // BrowserHome
    case SDL_SCANCODE_AUDIOMUTE:
        return 0xAD;  // VolumeMute
    case SDL_SCANCODE_VOLUMEDOWN:
        return 0xAE;  // VolumeDown
    case SDL_SCANCODE_VOLUMEUP:
        return 0xAF;  // VolumeUp
    case SDL_SCANCODE_AUDIONEXT:
        return 0xB0;  // MediaNextTrack
    case SDL_SCANCODE_AUDIOPREV:
        return 0xB1;  // MediaPreviousTrack
    case SDL_SCANCODE_AUDIOSTOP:
        return 0xB2;  // MediaStop
    case SDL_SCANCODE_AUDIOPLAY:
        return 0xB3;  // MediaPlayPause
    case SDL_SCANCODE_MAIL:
        return 0xB4;  // LaunchMail
    case SDL_SCANCODE_MEDIASELECT:
        return 0xB5;  // SelectMedia
    case SDL_SCANCODE_APP1:
        return 0xB6;  // LaunchApplication1
    case SDL_SCANCODE_APP2:
        return 0xB7;  // LaunchApplication2

    // OEM keys
    case SDL_SCANCODE_SEMICOLON:
        return 0xBA;  // OemSemicolon
    case SDL_SCANCODE_EQUALS:
        return 0xBB;  // OemPlus
    case SDL_SCANCODE_COMMA:
        return 0xBC;  // OemComma
    case SDL_SCANCODE_MINUS:
        return 0xBD;  // OemMinus
    case SDL_SCANCODE_PERIOD:
        return 0xBE;  // OemPeriod
    case SDL_SCANCODE_SLASH:
        return 0xBF;  // OemQuestion
    case SDL_SCANCODE_GRAVE:
        return 0xC0;  // OemTilde
    case SDL_SCANCODE_LEFTBRACKET:
        return 0xDB;  // OemOpenBrackets
    case SDL_SCANCODE_BACKSLASH:
        return 0xDC;  // OemPipe
    case SDL_SCANCODE_RIGHTBRACKET:
        return 0xDD;  // OemCloseBrackets
    case SDL_SCANCODE_APOSTROPHE:
        return 0xDE;  // OemQuotes
    case SDL_SCANCODE_NONUSBACKSLASH:
        return 0xE2;  // OemBackslash

    // IME keys
    case SDL_SCANCODE_LANG1:
        return 0x15;  // HangulMode/KanaMode
    case SDL_SCANCODE_LANG2:
        return 0x19;  // HanjaMode/KanjiMode
    case SDL_SCANCODE_LANG3:
        return 0x146;  // Katakana
    case SDL_SCANCODE_LANG4:
        return 0x147;  // Hiragana
    case SDL_SCANCODE_LANG5:
        return 0x148;  // Zenkaku/Hankaku

    // Special keys
    case SDL_SCANCODE_MENU:
        return 0x118;  // Menu
    case SDL_SCANCODE_SYSREQ:
        return 0x154;  // SysReq
    case SDL_SCANCODE_CANCEL:
        return 0x03;  // Cancel
    case SDL_SCANCODE_EXECUTE:
        return 0x116;  // Execute
    case SDL_SCANCODE_STOP:
        return 0x120;  // Stop
    case SDL_SCANCODE_AGAIN:
        return 0x121;  // Again
    case SDL_SCANCODE_UNDO:
        return 0x122;  // Undo
    case SDL_SCANCODE_CUT:
        return 0x123;  // Cut
    case SDL_SCANCODE_COPY:
        return 0x124;  // Copy
    case SDL_SCANCODE_PASTE:
        return 0x125;  // Paste
    case SDL_SCANCODE_FIND:
        return 0x126;  // Find
    case SDL_SCANCODE_MUTE:
        return 0x127;  // Mute

    // Fallback: unmapped keys
    default:
        return 0;  // None
    }
}
}  // namespace

class KeyboardSDL2
{
public:
    static KeyboardSDL2& get()
    {
        static KeyboardSDL2 instance;
        return instance;
    }

    Keyboard::State getState() const
    {
        std::lock_guard lock( m_Mutex );

        Keyboard::State state {};
        int             numKeys  = 0;
        const Uint8*    sdlState = SDL_GetKeyboardState( &numKeys );

        for ( int scancode = 0; scancode < numKeys; ++scancode )
        {
            if ( sdlState[scancode] )
            {
                int vk = SDLScancodeToVirtualKey( static_cast<SDL_Scancode>( scancode ) );
                KeyDown( vk, state );
            }
        }

        state.AltKey     = state.LeftAlt || state.RightAlt;
        state.ControlKey = state.LeftControl || state.RightControl;
        state.ShiftKey   = state.LeftShift || state.RightShift;

        return state;
    }

    bool isConnected() const
    {
        // SDL2 always has a keyboard device
        return true;
    }

    KeyboardSDL2( const KeyboardSDL2& )            = delete;
    KeyboardSDL2( KeyboardSDL2&& )                 = delete;
    KeyboardSDL2& operator=( const KeyboardSDL2& ) = delete;
    KeyboardSDL2& operator=( KeyboardSDL2&& )      = delete;

private:
    KeyboardSDL2()  = default;
    ~KeyboardSDL2() = default;

    mutable std::mutex m_Mutex;
};

namespace input::Keyboard
{
State getState()
{
    return KeyboardSDL2::get().getState();
}

void reset()
{}

bool isConnected()
{
    return KeyboardSDL2::get().isConnected();
}
}  // namespace input::Keyboard
