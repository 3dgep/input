#include <input/Keyboard.hpp>

#include <SDL3/SDL_keyboard.h>

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

// Map SDL scancode to Keyboard::Keys enum value or VK code
int SDLScancodeToVirtualKey( SDL_Scancode scancode )
{
    using K = Keyboard::Key;
    switch ( scancode )
    {
    case SDL_SCANCODE_BACKSPACE:
        return static_cast<int>( K::Back );
    case SDL_SCANCODE_TAB:
        return static_cast<int>( K::Tab );
    case SDL_SCANCODE_CLEAR:
        return static_cast<int>( K::Clear );
    case SDL_SCANCODE_RETURN:
        return static_cast<int>( K::Enter );
    case SDL_SCANCODE_PAUSE:
        return static_cast<int>( K::Pause );
    case SDL_SCANCODE_CAPSLOCK:
        return static_cast<int>( K::CapsLock );
    case SDL_SCANCODE_ESCAPE:
        return static_cast<int>( K::Escape );
    case SDL_SCANCODE_SPACE:
        return static_cast<int>( K::Space );
    case SDL_SCANCODE_PAGEUP:
        return static_cast<int>( K::PageUp );
    case SDL_SCANCODE_PAGEDOWN:
        return static_cast<int>( K::PageDown );
    case SDL_SCANCODE_END:
        return static_cast<int>( K::End );
    case SDL_SCANCODE_HOME:
        return static_cast<int>( K::Home );
    case SDL_SCANCODE_LEFT:
        return static_cast<int>( K::Left );
    case SDL_SCANCODE_UP:
        return static_cast<int>( K::Up );
    case SDL_SCANCODE_RIGHT:
        return static_cast<int>( K::Right );
    case SDL_SCANCODE_DOWN:
        return static_cast<int>( K::Down );
    case SDL_SCANCODE_SELECT:
        return static_cast<int>( K::Select );
    case SDL_SCANCODE_PRINTSCREEN:
        return static_cast<int>( K::PrintScreen );
    case SDL_SCANCODE_INSERT:
        return static_cast<int>( K::Insert );
    case SDL_SCANCODE_DELETE:
        return static_cast<int>( K::Delete );
    case SDL_SCANCODE_HELP:
        return static_cast<int>( K::Help );

    case SDL_SCANCODE_0:
        return static_cast<int>( K::D0 );
    case SDL_SCANCODE_1:
        return static_cast<int>( K::D1 );
    case SDL_SCANCODE_2:
        return static_cast<int>( K::D2 );
    case SDL_SCANCODE_3:
        return static_cast<int>( K::D3 );
    case SDL_SCANCODE_4:
        return static_cast<int>( K::D4 );
    case SDL_SCANCODE_5:
        return static_cast<int>( K::D5 );
    case SDL_SCANCODE_6:
        return static_cast<int>( K::D6 );
    case SDL_SCANCODE_7:
        return static_cast<int>( K::D7 );
    case SDL_SCANCODE_8:
        return static_cast<int>( K::D8 );
    case SDL_SCANCODE_9:
        return static_cast<int>( K::D9 );

    case SDL_SCANCODE_A:
        return static_cast<int>( K::A );
    case SDL_SCANCODE_B:
        return static_cast<int>( K::B );
    case SDL_SCANCODE_C:
        return static_cast<int>( K::C );
    case SDL_SCANCODE_D:
        return static_cast<int>( K::D );
    case SDL_SCANCODE_E:
        return static_cast<int>( K::E );
    case SDL_SCANCODE_F:
        return static_cast<int>( K::F );
    case SDL_SCANCODE_G:
        return static_cast<int>( K::G );
    case SDL_SCANCODE_H:
        return static_cast<int>( K::H );
    case SDL_SCANCODE_I:
        return static_cast<int>( K::I );
    case SDL_SCANCODE_J:
        return static_cast<int>( K::J );
    case SDL_SCANCODE_K:
        return static_cast<int>( K::K );
    case SDL_SCANCODE_L:
        return static_cast<int>( K::L );
    case SDL_SCANCODE_M:
        return static_cast<int>( K::M );
    case SDL_SCANCODE_N:
        return static_cast<int>( K::N );
    case SDL_SCANCODE_O:
        return static_cast<int>( K::O );
    case SDL_SCANCODE_P:
        return static_cast<int>( K::P );
    case SDL_SCANCODE_Q:
        return static_cast<int>( K::Q );
    case SDL_SCANCODE_R:
        return static_cast<int>( K::R );
    case SDL_SCANCODE_S:
        return static_cast<int>( K::S );
    case SDL_SCANCODE_T:
        return static_cast<int>( K::T );
    case SDL_SCANCODE_U:
        return static_cast<int>( K::U );
    case SDL_SCANCODE_V:
        return static_cast<int>( K::V );
    case SDL_SCANCODE_W:
        return static_cast<int>( K::W );
    case SDL_SCANCODE_X:
        return static_cast<int>( K::X );
    case SDL_SCANCODE_Y:
        return static_cast<int>( K::Y );
    case SDL_SCANCODE_Z:
        return static_cast<int>( K::Z );

    case SDL_SCANCODE_LGUI:
        return static_cast<int>( K::LeftSuper );
    case SDL_SCANCODE_RGUI:
        return static_cast<int>( K::RightSuper );
    case SDL_SCANCODE_APPLICATION:
        return static_cast<int>( K::Apps );

    case SDL_SCANCODE_KP_0:
        return static_cast<int>( K::NumPad0 );
    case SDL_SCANCODE_KP_1:
        return static_cast<int>( K::NumPad1 );
    case SDL_SCANCODE_KP_2:
        return static_cast<int>( K::NumPad2 );
    case SDL_SCANCODE_KP_3:
        return static_cast<int>( K::NumPad3 );
    case SDL_SCANCODE_KP_4:
        return static_cast<int>( K::NumPad4 );
    case SDL_SCANCODE_KP_5:
        return static_cast<int>( K::NumPad5 );
    case SDL_SCANCODE_KP_6:
        return static_cast<int>( K::NumPad6 );
    case SDL_SCANCODE_KP_7:
        return static_cast<int>( K::NumPad7 );
    case SDL_SCANCODE_KP_8:
        return static_cast<int>( K::NumPad8 );
    case SDL_SCANCODE_KP_9:
        return static_cast<int>( K::NumPad9 );
    case SDL_SCANCODE_KP_MULTIPLY:
        return static_cast<int>( K::Multiply );
    case SDL_SCANCODE_KP_PLUS:
        return static_cast<int>( K::Add );
    case SDL_SCANCODE_KP_ENTER:
        return 0x6C;  // No direct enum
    case SDL_SCANCODE_KP_MINUS:
        return static_cast<int>( K::Subtract );
    case SDL_SCANCODE_KP_DECIMAL:
        return static_cast<int>( K::Decimal );
    case SDL_SCANCODE_KP_DIVIDE:
        return static_cast<int>( K::Divide );

    case SDL_SCANCODE_F1:
        return static_cast<int>( K::F1 );
    case SDL_SCANCODE_F2:
        return static_cast<int>( K::F2 );
    case SDL_SCANCODE_F3:
        return static_cast<int>( K::F3 );
    case SDL_SCANCODE_F4:
        return static_cast<int>( K::F4 );
    case SDL_SCANCODE_F5:
        return static_cast<int>( K::F5 );
    case SDL_SCANCODE_F6:
        return static_cast<int>( K::F6 );
    case SDL_SCANCODE_F7:
        return static_cast<int>( K::F7 );
    case SDL_SCANCODE_F8:
        return static_cast<int>( K::F8 );
    case SDL_SCANCODE_F9:
        return static_cast<int>( K::F9 );
    case SDL_SCANCODE_F10:
        return static_cast<int>( K::F10 );
    case SDL_SCANCODE_F11:
        return static_cast<int>( K::F11 );
    case SDL_SCANCODE_F12:
        return static_cast<int>( K::F12 );
    case SDL_SCANCODE_F13:
        return static_cast<int>( K::F13 );
    case SDL_SCANCODE_F14:
        return static_cast<int>( K::F14 );
    case SDL_SCANCODE_F15:
        return static_cast<int>( K::F15 );
    case SDL_SCANCODE_F16:
        return static_cast<int>( K::F16 );
    case SDL_SCANCODE_F17:
        return static_cast<int>( K::F17 );
    case SDL_SCANCODE_F18:
        return static_cast<int>( K::F18 );
    case SDL_SCANCODE_F19:
        return static_cast<int>( K::F19 );
    case SDL_SCANCODE_F20:
        return static_cast<int>( K::F20 );
    case SDL_SCANCODE_F21:
        return static_cast<int>( K::F21 );
    case SDL_SCANCODE_F22:
        return static_cast<int>( K::F22 );
    case SDL_SCANCODE_F23:
        return static_cast<int>( K::F23 );
    case SDL_SCANCODE_F24:
        return static_cast<int>( K::F24 );

    case SDL_SCANCODE_NUMLOCKCLEAR:
        return static_cast<int>( K::NumLock );
    case SDL_SCANCODE_SCROLLLOCK:
        return static_cast<int>( K::Scroll );

    case SDL_SCANCODE_LSHIFT:
        return static_cast<int>( K::LeftShift );
    case SDL_SCANCODE_RSHIFT:
        return static_cast<int>( K::RightShift );
    case SDL_SCANCODE_LCTRL:
        return static_cast<int>( K::LeftControl );
    case SDL_SCANCODE_RCTRL:
        return static_cast<int>( K::RightControl );
    case SDL_SCANCODE_LALT:
        return static_cast<int>( K::LeftAlt );
    case SDL_SCANCODE_RALT:
        return static_cast<int>( K::RightAlt );

    case SDL_SCANCODE_AC_BACK:
        return static_cast<int>( K::BrowserBack );
    case SDL_SCANCODE_AC_FORWARD:
        return static_cast<int>( K::BrowserForward );
    case SDL_SCANCODE_AC_REFRESH:
        return static_cast<int>( K::BrowserRefresh );
    case SDL_SCANCODE_AC_STOP:
        return static_cast<int>( K::BrowserStop );
    case SDL_SCANCODE_AC_SEARCH:
        return static_cast<int>( K::BrowserSearch );
    case SDL_SCANCODE_AC_BOOKMARKS:
        return static_cast<int>( K::BrowserFavorites );
    case SDL_SCANCODE_AC_HOME:
        return static_cast<int>( K::BrowserHome );
    case SDL_SCANCODE_MUTE:
        return static_cast<int>( K::VolumeMute );
    case SDL_SCANCODE_VOLUMEDOWN:
        return static_cast<int>( K::VolumeDown );
    case SDL_SCANCODE_VOLUMEUP:
        return static_cast<int>( K::VolumeUp );
    case SDL_SCANCODE_MEDIA_NEXT_TRACK:
        return static_cast<int>( K::MediaNextTrack );
    case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK:
        return static_cast<int>( K::MediaPreviousTrack );
    case SDL_SCANCODE_MEDIA_STOP:
        return static_cast<int>( K::MediaStop );
    case SDL_SCANCODE_MEDIA_PLAY:
        return static_cast<int>( K::MediaPlayPause );
    // case SDL_SCANCODE_MAIL: return static_cast<int>(K::LaunchMail); // No matching scancode.
    case SDL_SCANCODE_MEDIA_SELECT:
        return static_cast<int>( K::SelectMedia );
        // case SDL_SCANCODE_APP1: return static_cast<int>(K::LaunchApplication1); // No matching scancode.
        // case SDL_SCANCODE_APP2: return static_cast<int>(K::LaunchApplication2); // No matching scancode.

    case SDL_SCANCODE_SEMICOLON:
        return static_cast<int>( K::OemSemicolon );
    case SDL_SCANCODE_EQUALS:
        return static_cast<int>( K::OemPlus );
    case SDL_SCANCODE_COMMA:
        return static_cast<int>( K::OemComma );
    case SDL_SCANCODE_MINUS:
        return static_cast<int>( K::OemMinus );
    case SDL_SCANCODE_PERIOD:
        return static_cast<int>( K::OemPeriod );
    case SDL_SCANCODE_SLASH:
        return static_cast<int>( K::OemQuestion );
    case SDL_SCANCODE_GRAVE:
        return static_cast<int>( K::OemTilde );
    case SDL_SCANCODE_LEFTBRACKET:
        return static_cast<int>( K::OemOpenBrackets );
    case SDL_SCANCODE_BACKSLASH:
        return static_cast<int>( K::OemPipe );
    case SDL_SCANCODE_RIGHTBRACKET:
        return static_cast<int>( K::OemCloseBrackets );
    case SDL_SCANCODE_APOSTROPHE:
        return static_cast<int>( K::OemQuotes );
    case SDL_SCANCODE_NONUSBACKSLASH:
        return static_cast<int>( K::OemBackslash );

    case SDL_SCANCODE_LANG1:
        return static_cast<int>( K::KanaMode );
    case SDL_SCANCODE_LANG2:
        return static_cast<int>( K::KanjiMode );

    case SDL_SCANCODE_MENU:
        return static_cast<int>( K::Apps );
    case SDL_SCANCODE_CANCEL:
        return static_cast<int>( K::Cancel );
    case SDL_SCANCODE_EXECUTE:
        return static_cast<int>( K::Execute );
    case SDL_SCANCODE_STOP:
        return static_cast<int>( K::BrowserStop );

    default:
        return static_cast<int>( K::None );
    }
}
}  // namespace

class KeyboardSDL3
{
public:
    static KeyboardSDL3& get()
    {
        static KeyboardSDL3 instance;
        return instance;
    }

    Keyboard::State getState() const
    {
        std::scoped_lock lock( m_Mutex );

        Keyboard::State state {};
        int             numKeys  = 0;
        const bool*     sdlState = SDL_GetKeyboardState( &numKeys );

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

    static bool isConnected()
    {
        return SDL_HasKeyboard();
    }

    KeyboardSDL3( const KeyboardSDL3& )            = delete;
    KeyboardSDL3( KeyboardSDL3&& )                 = delete;
    KeyboardSDL3& operator=( const KeyboardSDL3& ) = delete;
    KeyboardSDL3& operator=( KeyboardSDL3&& )      = delete;

private:
    KeyboardSDL3()  = default;
    ~KeyboardSDL3() = default;

    mutable std::mutex m_Mutex;
};

namespace input::Keyboard
{
State getState()
{
    return KeyboardSDL3::get().getState();
}

void reset()
{}

bool isConnected()
{
    return KeyboardSDL3::isConnected();
}
}  // namespace input::Keyboard
