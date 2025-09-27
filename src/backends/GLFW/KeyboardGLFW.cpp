#include <input/Keyboard.hpp>
#include <GLFW/glfw3.h>
#include <mutex>

using namespace input;

// NOTE: You must register the Keyboard_Callback function with GLFW using glfwSetKeyCallback.
// Example:
// void Keyboard_Callback( GLFWwindow* window, int key, int scancode, int action, int mods );
// glfwSetKeyCallback(window, Keyboard_Callback);

void Keyboard_Callback( GLFWwindow* window, int key, int scancode, int action, int mods );

namespace
{
void KeyDown(int key, Keyboard::State& state) noexcept
{
    if (key < 0 || key > 0xfe)
        return;

    auto ptr = reinterpret_cast<uint32_t*>(&state);
    const unsigned int bf = 1u << (key & 0x1f);
    ptr[(key >> 5)] |= bf;
}

void KeyUp(int key, Keyboard::State& state) noexcept
{
    if (key < 0 || key > 0xfe)
        return;

    auto ptr = reinterpret_cast<uint32_t*>(&state);
    const unsigned int bf = 1u << (key & 0x1f);
    ptr[(key >> 5)] &= ~bf;
}

// Map GLFW key to Win32 virtual key code
int GLFWKeyToVirtualKey(int glfwKey)
{
    using K = Keyboard::Keys;
    switch (glfwKey)
    {
    // Printable keys
    case GLFW_KEY_SPACE: return static_cast<int>(K::Space);
    case GLFW_KEY_APOSTROPHE: return static_cast<int>(K::OemQuotes);
    case GLFW_KEY_COMMA: return static_cast<int>(K::OemComma);
    case GLFW_KEY_MINUS: return static_cast<int>(K::OemMinus);
    case GLFW_KEY_PERIOD: return static_cast<int>(K::OemPeriod);
    case GLFW_KEY_SLASH: return static_cast<int>(K::OemQuestion);
    case GLFW_KEY_0: return static_cast<int>(K::D0);
    case GLFW_KEY_1: return static_cast<int>(K::D1);
    case GLFW_KEY_2: return static_cast<int>(K::D2);
    case GLFW_KEY_3: return static_cast<int>(K::D3);
    case GLFW_KEY_4: return static_cast<int>(K::D4);
    case GLFW_KEY_5: return static_cast<int>(K::D5);
    case GLFW_KEY_6: return static_cast<int>(K::D6);
    case GLFW_KEY_7: return static_cast<int>(K::D7);
    case GLFW_KEY_8: return static_cast<int>(K::D8);
    case GLFW_KEY_9: return static_cast<int>(K::D9);
    case GLFW_KEY_SEMICOLON: return static_cast<int>(K::OemSemicolon);
    case GLFW_KEY_EQUAL: return static_cast<int>(K::OemPlus);
    case GLFW_KEY_A: return static_cast<int>(K::A);
    case GLFW_KEY_B: return static_cast<int>(K::B);
    case GLFW_KEY_C: return static_cast<int>(K::C);
    case GLFW_KEY_D: return static_cast<int>(K::D);
    case GLFW_KEY_E: return static_cast<int>(K::E);
    case GLFW_KEY_F: return static_cast<int>(K::F);
    case GLFW_KEY_G: return static_cast<int>(K::G);
    case GLFW_KEY_H: return static_cast<int>(K::H);
    case GLFW_KEY_I: return static_cast<int>(K::I);
    case GLFW_KEY_J: return static_cast<int>(K::J);
    case GLFW_KEY_K: return static_cast<int>(K::K);
    case GLFW_KEY_L: return static_cast<int>(K::L);
    case GLFW_KEY_M: return static_cast<int>(K::M);
    case GLFW_KEY_N: return static_cast<int>(K::N);
    case GLFW_KEY_O: return static_cast<int>(K::O);
    case GLFW_KEY_P: return static_cast<int>(K::P);
    case GLFW_KEY_Q: return static_cast<int>(K::Q);
    case GLFW_KEY_R: return static_cast<int>(K::R);
    case GLFW_KEY_S: return static_cast<int>(K::S);
    case GLFW_KEY_T: return static_cast<int>(K::T);
    case GLFW_KEY_U: return static_cast<int>(K::U);
    case GLFW_KEY_V: return static_cast<int>(K::V);
    case GLFW_KEY_W: return static_cast<int>(K::W);
    case GLFW_KEY_X: return static_cast<int>(K::X);
    case GLFW_KEY_Y: return static_cast<int>(K::Y);
    case GLFW_KEY_Z: return static_cast<int>(K::Z);
    case GLFW_KEY_LEFT_BRACKET: return static_cast<int>(K::OemOpenBrackets);
    case GLFW_KEY_BACKSLASH: return static_cast<int>(K::OemPipe);
    case GLFW_KEY_RIGHT_BRACKET: return static_cast<int>(K::OemCloseBrackets);
    case GLFW_KEY_GRAVE_ACCENT: return static_cast<int>(K::OemTilde);
    case GLFW_KEY_WORLD_1: return static_cast<int>(K::None);
    case GLFW_KEY_WORLD_2: return static_cast<int>(K::None);

    // Function keys
    case GLFW_KEY_ESCAPE: return static_cast<int>(K::Escape);
    case GLFW_KEY_ENTER: return static_cast<int>(K::Enter);
    case GLFW_KEY_TAB: return static_cast<int>(K::Tab);
    case GLFW_KEY_BACKSPACE: return static_cast<int>(K::Back);
    case GLFW_KEY_INSERT: return static_cast<int>(K::Insert);
    case GLFW_KEY_DELETE: return static_cast<int>(K::Delete);
    case GLFW_KEY_RIGHT: return static_cast<int>(K::Right);
    case GLFW_KEY_LEFT: return static_cast<int>(K::Left);
    case GLFW_KEY_DOWN: return static_cast<int>(K::Down);
    case GLFW_KEY_UP: return static_cast<int>(K::Up);
    case GLFW_KEY_PAGE_UP: return static_cast<int>(K::PageUp);
    case GLFW_KEY_PAGE_DOWN: return static_cast<int>(K::PageDown);
    case GLFW_KEY_HOME: return static_cast<int>(K::Home);
    case GLFW_KEY_END: return static_cast<int>(K::End);
    case GLFW_KEY_CAPS_LOCK: return static_cast<int>(K::CapsLock);
    case GLFW_KEY_SCROLL_LOCK: return static_cast<int>(K::Scroll);
    case GLFW_KEY_NUM_LOCK: return static_cast<int>(K::NumLock);
    case GLFW_KEY_PRINT_SCREEN: return static_cast<int>(K::PrintScreen);
    case GLFW_KEY_PAUSE: return static_cast<int>(K::Pause);
    case GLFW_KEY_F1: return static_cast<int>(K::F1);
    case GLFW_KEY_F2: return static_cast<int>(K::F2);
    case GLFW_KEY_F3: return static_cast<int>(K::F3);
    case GLFW_KEY_F4: return static_cast<int>(K::F4);
    case GLFW_KEY_F5: return static_cast<int>(K::F5);
    case GLFW_KEY_F6: return static_cast<int>(K::F6);
    case GLFW_KEY_F7: return static_cast<int>(K::F7);
    case GLFW_KEY_F8: return static_cast<int>(K::F8);
    case GLFW_KEY_F9: return static_cast<int>(K::F9);
    case GLFW_KEY_F10: return static_cast<int>(K::F10);
    case GLFW_KEY_F11: return static_cast<int>(K::F11);
    case GLFW_KEY_F12: return static_cast<int>(K::F12);
    case GLFW_KEY_F13: return static_cast<int>(K::F13);
    case GLFW_KEY_F14: return static_cast<int>(K::F14);
    case GLFW_KEY_F15: return static_cast<int>(K::F15);
    case GLFW_KEY_F16: return static_cast<int>(K::F16);
    case GLFW_KEY_F17: return static_cast<int>(K::F17);
    case GLFW_KEY_F18: return static_cast<int>(K::F18);
    case GLFW_KEY_F19: return static_cast<int>(K::F19);
    case GLFW_KEY_F20: return static_cast<int>(K::F20);
    case GLFW_KEY_F21: return static_cast<int>(K::F21);
    case GLFW_KEY_F22: return static_cast<int>(K::F22);
    case GLFW_KEY_F23: return static_cast<int>(K::F23);
    case GLFW_KEY_F24: return static_cast<int>(K::F24);

    // Keypad
    case GLFW_KEY_KP_0: return static_cast<int>(K::NumPad0);
    case GLFW_KEY_KP_1: return static_cast<int>(K::NumPad1);
    case GLFW_KEY_KP_2: return static_cast<int>(K::NumPad2);
    case GLFW_KEY_KP_3: return static_cast<int>(K::NumPad3);
    case GLFW_KEY_KP_4: return static_cast<int>(K::NumPad4);
    case GLFW_KEY_KP_5: return static_cast<int>(K::NumPad5);
    case GLFW_KEY_KP_6: return static_cast<int>(K::NumPad6);
    case GLFW_KEY_KP_7: return static_cast<int>(K::NumPad7);
    case GLFW_KEY_KP_8: return static_cast<int>(K::NumPad8);
    case GLFW_KEY_KP_9: return static_cast<int>(K::NumPad9);
    case GLFW_KEY_KP_DECIMAL: return static_cast<int>(K::Decimal);
    case GLFW_KEY_KP_DIVIDE: return static_cast<int>(K::Divide);
    case GLFW_KEY_KP_MULTIPLY: return static_cast<int>(K::Multiply);
    case GLFW_KEY_KP_SUBTRACT: return static_cast<int>(K::Subtract);
    case GLFW_KEY_KP_ADD: return static_cast<int>(K::Add);
    case GLFW_KEY_KP_ENTER: return static_cast<int>(K::Enter); // Numpad Enter
    case GLFW_KEY_KP_EQUAL: return static_cast<int>(K::OemPlus);

    // Modifier keys
    case GLFW_KEY_LEFT_SHIFT: return static_cast<int>(K::LeftShift);
    case GLFW_KEY_LEFT_CONTROL: return static_cast<int>(K::LeftControl);
    case GLFW_KEY_LEFT_ALT: return static_cast<int>(K::LeftAlt);
    case GLFW_KEY_LEFT_SUPER: return static_cast<int>(K::LeftSuper);
    case GLFW_KEY_RIGHT_SHIFT: return static_cast<int>(K::RightShift);
    case GLFW_KEY_RIGHT_CONTROL: return static_cast<int>(K::RightControl);
    case GLFW_KEY_RIGHT_ALT: return static_cast<int>(K::RightAlt);
    case GLFW_KEY_RIGHT_SUPER: return static_cast<int>(K::RightSuper);
    case GLFW_KEY_MENU: return static_cast<int>(K::Apps);

    // Unmapped/undefined keys
    default:
        return static_cast<int>(K::None);
    }
}
} // namespace

class KeyboardGLFW
{
public:
    static KeyboardGLFW& get()
    {
        static KeyboardGLFW instance;
        return instance;
    }

    Keyboard::State getState()
    {
        std::lock_guard lock(m_Mutex);

        m_State.AltKey     = m_State.LeftAlt || m_State.RightAlt;
        m_State.ControlKey = m_State.LeftControl || m_State.RightControl;
        m_State.ShiftKey   = m_State.LeftShift || m_State.RightShift;

        return m_State;
    }

    void reset() noexcept
    {
        std::lock_guard lock(m_Mutex);
        std::memset(&m_State, 0, sizeof(Keyboard::State));
    }

    bool isConnected() const
    {
        return true;
    }

    KeyboardGLFW(const KeyboardGLFW&) = delete;
    KeyboardGLFW(KeyboardGLFW&&) = delete;
    KeyboardGLFW& operator=(const KeyboardGLFW&) = delete;
    KeyboardGLFW& operator=(KeyboardGLFW&&) = delete;

private:
    friend void Keyboard_Callback( GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/ );

    KeyboardGLFW() = default;
    ~KeyboardGLFW() = default;

    mutable std::mutex m_Mutex;
    Keyboard::State m_State{};
};

// GLFW key callback function (outside the class)
void Keyboard_Callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    auto& impl = KeyboardGLFW::get();
    int vk = GLFWKeyToVirtualKey(key);
    if (vk == 0)
        return;

    std::lock_guard lock(impl.m_Mutex);

    if (action == GLFW_PRESS)
    {
        KeyDown(vk, impl.m_State);
    }
    else if (action == GLFW_RELEASE)
    {
        KeyUp(vk, impl.m_State);
    }
    // GLFW_REPEAT is ignored for state
}

// Bridge to Keyboard interface
Keyboard::State Keyboard::getState() const
{
    return KeyboardGLFW::get().getState();
}

void Keyboard::reset()
{
    KeyboardGLFW::get().reset();
}

bool Keyboard::isConnected()
{
    return KeyboardGLFW::get().isConnected();
}
