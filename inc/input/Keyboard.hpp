#pragma once

#include <cstdint>

namespace input
{
// Source (September 15, 2025): https://github.com/microsoft/DirectXTK/blob/main/Inc/Keyboard.h
namespace Keyboard
{
enum class Key : uint8_t
{
    None        = 0x00,         // No key was pressed
    LeftButton  = 0x01,         // Left mouse button
    RightButton = 0x02,         // Right mouse button
    Cancel      = 0x03,         // Control-break processing.
    MButton     = 0x04,         // Middle mouse button
    XButton1    = 0x05,         // X1 mouse button
    XButton2    = 0x06,         // X2 mouse button
                                // 0x07 is undefined
    Back = 0x08,                // The Backspace key.
    Tab  = 0x09,                // The Tab key.
                                // 0x0A-0B are reserved
    Clear = 0x0c,               // The Clear key
    Enter = 0x0d,               // The Enter key
                                // 0x0E-0F are undefined
    ShiftKey      = 0x10,       // The Shift key
    ControlKey    = 0x11,       // The Ctrl key
    AltKey        = 0x12,       // The Alt key
    Pause         = 0x13,       // The Pause key
    Capital       = 0x14,       // The Caps Lock key
    CapsLock      = 0x14,       // The Caps Lock key
    KanaMode      = 0x15,       // IME Kana mode
    HanguelMode   = 0x15,       // IME Hanguel mode (Use HangulMode)
    HangulMode    = 0x15,       // IME Hangul mode
    ImeOn         = 0x16,       // IME On
    JunjaMode     = 0x17,       // IME Junja mode
    FinalMode     = 0x18,       // IME Final mode
    HanjaMode     = 0x19,       // IME Hanja mode
    KanjiMode     = 0x19,       // IME Kanji mode
    ImeOff        = 0x1a,       // IME Off
    Escape        = 0x1b,       // The Escape key
    IMEConvert    = 0x1c,       // IME convert key
    IMENoConvert  = 0x1d,       // IME noconvert key
    IMEAccept     = 0x1e,       // IME accept key
    IMEModeChange = 0x1f,       // IME mode change key
    Space         = 0x20,       // The Space key
    Prior         = 0x21,       // The Page Up key
    PageUp        = 0x21,       // The Page Up key
    Next          = 0x22,       // The Page Down key
    PageDown      = 0x22,       // The Page Down key
    End           = 0x23,       // The End key
    Home          = 0x24,       // The Home key
    Left          = 0x25,       // The Left arrow key
    Up            = 0x26,       // The Up arrow key
    Right         = 0x27,       // The Right arrow key
    Down          = 0x28,       // The Down arrow key
    Select        = 0x29,       // The Select key
    Print         = 0x2a,       // The Print key
    Execute       = 0x2b,       // The Execute key
    PrintScreen   = 0x2c,       // The Print Screen key
    Snapshot      = 0x2c,       // The Print Screen key
    Insert        = 0x2d,       // The Insert key
    Delete        = 0x2e,       // The Delete key
    Help          = 0x2f,       // The Help key
    D0            = 0x30,       // 0
    D1            = 0x31,       // 1
    D2            = 0x32,       // 2
    D3            = 0x33,       // 3
    D4            = 0x34,       // 4
    D5            = 0x35,       // 5
    D6            = 0x36,       // 6
    D7            = 0x37,       // 7
    D8            = 0x38,       // 8
    D9            = 0x39,       // 9
                                // 0x3A - 40 are undefined
    A          = 0x41,          // A
    B          = 0x42,          // B
    C          = 0x43,          // C
    D          = 0x44,          // D
    E          = 0x45,          // E
    F          = 0x46,          // F
    G          = 0x47,          // G
    H          = 0x48,          // H
    I          = 0x49,          // I
    J          = 0x4a,          // J
    K          = 0x4b,          // K
    L          = 0x4c,          // L
    M          = 0x4d,          // M
    N          = 0x4e,          // N
    O          = 0x4f,          // O
    P          = 0x50,          // P
    Q          = 0x51,          // Q
    R          = 0x52,          // R
    S          = 0x53,          // S
    T          = 0x54,          // T
    U          = 0x55,          // U
    V          = 0x56,          // V
    W          = 0x57,          // W
    X          = 0x58,          // X
    Y          = 0x59,          // Y
    Z          = 0x5a,          // Z
    LeftSuper  = 0x5b,          // Left Windows key
    RightSuper = 0x5c,          // Right Windows key
    Apps       = 0x5d,          // Apps key
                                // 0x5E is reserved
    Sleep     = 0x5f,           // The Sleep key
    NumPad0   = 0x60,           // The Numeric keypad 0 key
    NumPad1   = 0x61,           // The Numeric keypad 1 key
    NumPad2   = 0x62,           // The Numeric keypad 2 key
    NumPad3   = 0x63,           // The Numeric keypad 3 key
    NumPad4   = 0x64,           // The Numeric keypad 4 key
    NumPad5   = 0x65,           // The Numeric keypad 5 key
    NumPad6   = 0x66,           // The Numeric keypad 6 key
    NumPad7   = 0x67,           // The Numeric keypad 7 key
    NumPad8   = 0x68,           // The Numeric keypad 8 key
    NumPad9   = 0x69,           // The Numeric keypad 9 key
    Multiply  = 0x6a,           // The Multiply key
    Add       = 0x6b,           // The Add key
    Separator = 0x6c,           // The Separator key
    Subtract  = 0x6d,           // The Subtract key
    Decimal   = 0x6e,           // The Decimal key
    Divide    = 0x6f,           // The Divide key
    F1        = 0x70,           // The F1 key
    F2        = 0x71,           // The F2 key
    F3        = 0x72,           // The F3 key
    F4        = 0x73,           // The F4 key
    F5        = 0x74,           // The F5 key
    F6        = 0x75,           // The F6 key
    F7        = 0x76,           // The F7 key
    F8        = 0x77,           // The F8 key
    F9        = 0x78,           // The F9 key
    F10       = 0x79,           // The F10 key
    F11       = 0x7a,           // The F11 key
    F12       = 0x7b,           // The F12 key
    F13       = 0x7c,           // The F13 key
    F14       = 0x7d,           // The F14 key
    F15       = 0x7e,           // The F15 key
    F16       = 0x7f,           // The F16 key
    F17       = 0x80,           // The F17 key
    F18       = 0x81,           // The F18 key
    F19       = 0x82,           // The F19 key
    F20       = 0x83,           // The F20 key
    F21       = 0x84,           // The F21 key
    F22       = 0x85,           // The F22 key
    F23       = 0x86,           // The F23 key
    F24       = 0x87,           // The F24 key
                                // 0x88 - 0x8f are unassigned
    NumLock = 0x90,             // The Num Lock key
    Scroll  = 0x91,             // The Scroll Lock key
                                // 0x92 - 96 are OEM specific
                                // 0x97 - 9f are unassigned
    LeftShift          = 0xa0,  // The Left Shift key
    RightShift         = 0xa1,  // The Right Shift key
    LeftControl        = 0xa2,  // The Left Control key
    RightControl       = 0xa3,  // The Right Control key
    LeftAlt            = 0xa4,  // The Left Alt key
    RightAlt           = 0xa5,  // The Right Alt key
    BrowserBack        = 0xa6,  // The Browser Back key
    BrowserForward     = 0xa7,  // The Browser Forward key
    BrowserRefresh     = 0xa8,  // The Browser Refresh key
    BrowserStop        = 0xa9,  // The Browser Stop key
    BrowserSearch      = 0xaa,  // The Browser Search key
    BrowserFavorites   = 0xab,  // The Browser Favorites key
    BrowserHome        = 0xac,  // The Browser Home key
    VolumeMute         = 0xad,  // The Volume Mute key
    VolumeDown         = 0xae,  // The Volume Down key
    VolumeUp           = 0xaf,  // The Volume Up key
    MediaNextTrack     = 0xb0,  // The Next Track key
    MediaPreviousTrack = 0xb1,  // The Previous Track key
    MediaStop          = 0xb2,  // The Stop Media key
    MediaPlayPause     = 0xb3,  // The Play/Pause Media key
    LaunchMail         = 0xb4,  // The Start Mail key
    SelectMedia        = 0xb5,  // The Select Media key
    LaunchApplication1 = 0xb6,  // The Launch Application 1 key.
    LaunchApplication2 = 0xb7,  // The Launch Application 2 key.
                                // 0xB8 - B9 are reserved
    OemSemicolon = 0xba,        // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
    Oem1         = 0xba,        // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
    OemPlus      = 0xbb,        // For any country/region, the '+' key
    OemComma     = 0xbc,        // For any country/region, the ',' key
    OemMinus     = 0xbd,        // For any country/region, the '-' key
    OemPeriod    = 0xbe,        // For any country/region, the '.' key
    OemQuestion  = 0xbf,        // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
    Oem2         = 0xbf,        // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
    OemTilde     = 0xc0,        // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
    Oem3         = 0xc0,        // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
                                // 0xC1 - D7 are reserved
                                // 0xD8 - DA are unassigned
    OemOpenBrackets  = 0xdb,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
    Oem4             = 0xdb,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
    OemPipe          = 0xdc,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
    Oem5             = 0xdc,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
    OemCloseBrackets = 0xdd,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
    Oem6             = 0xdd,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
    OemQuotes        = 0xde,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
    Oem7             = 0xde,    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
    Oem8             = 0xdf,    // Used for miscellaneous characters; it can vary by keyboard.
                                // 0xE0 is reserved
                                // 0xE1 is OEM specific
    OemBackslash = 0xe2,        // Either the angle bracket key or the backslash key on the RT 102-key keyboard
    Oem102       = 0xe2,        // Either the angle bracket key or the backslash key on the RT 102-key keyboard
                                // 0xE3 - E4 OEM specific
    Process = 0xe5,             // IME Process key
                                // 0xE6 is OEM specific
    Packet = 0xe7,              // Used to pass Unicode characters as if they were keystrokes. The Packet key value is the low word of a 32-bit virtual-key value used for non-keyboard input methods.
                                // 0xE8 is unassigned
                                // 0xE9 - F5 OEM specific
    Attn     = 0xf6,            // The Attn key
    CrSel    = 0xf7,            // The CrSel key
    ExSel    = 0xf8,            // The ExSel key
    EraseEof = 0xf9,            // The Erase EOF key
    Play     = 0xfa,            // The Play key
    Zoom     = 0xfb,            // The Zoom key
    NoName   = 0xfc,            // Reserved
    Pa1      = 0xfd,            // The PA1 key
    OemClear = 0xfe,            // The Clear key
};

struct State
{
    // clang-format off
    bool Reserved0 : 1;           // Unused
    bool LeftButton : 1;          // Left mouse button, 0x01
    bool RightButton : 1;         // Right mouse button, 0x02
    bool Cancel : 1;              // Cancel key, 0x03
    bool MButton : 1;             // Middle mouse button, 0x04
    bool XButton1 : 1;            // X1 mouse button, 0x05
    bool XButton2 : 1;            // X2 mouse button, 0x06
    bool Reserved1 : 1;           // 0x07 is undefined.
    // ============ ( 1 byte ) ============================
    bool Back : 1;                // The back button, 0x08
    bool Tab : 1;                 // The Tab key, 0x09
    bool Reserved2 : 2;           // 0x0A-0B are undefined
    bool Clear : 1;               // The Clear key, 0x0C
    bool Enter : 1;               // The Enter key, 0x0D
    bool Reserved3 : 2;           // 0x0E-0F are undefined
    // ============ ( 2 bytes ) ===========================
    bool ShiftKey : 1;            // The Shift key, 0x10
    bool ControlKey : 1;          // The Ctrl key, 0x11
    bool AltKey : 1;              // The Alt key, 0x12
    bool Pause : 1;               // The Pause key, 0x13
    bool CapsLock : 1;            // The Caps Lock key, 0x14
    bool KanaMode : 1;            // IMI Kana mode, 0x15
    bool ImeOn : 1;               // IME On, 0x16
    bool JunjaMode : 1;           // IME Janja mode, 0x17
    // ============ ( 3 bytes ) ===========================
    bool FinalMode : 1;           // IME Final mode, 0x18
    bool KanjiMode : 1;           // IME Kanji mode, 0x19
    bool ImeOff : 1;              // IME Off, 0x1A
    bool Escape : 1;              // The Escape key, 0x1B
    bool IMEConvert : 1;          // IME convert key, 0x1C
    bool IMENoConvert : 1;        // IME no convert key, 0x1D
    bool IMEAccept : 1;           // IME accept key, 0x1E
    bool IMEModeChange : 1;       // IME mode change key, 0x1F
    // ============ ( 4 bytes ) ===========================
    bool Space : 1;               // The Space key, 0x20
    bool PageUp : 1;              // The Page Up key, 0x21
    bool PageDown : 1;            // The Page Down key, 0x22
    bool End : 1;                 // The End key, 0x23
    bool Home : 1;                // The Home key, 0x24
    bool Left : 1;                // The Left arrow key, 0x25
    bool Up : 1;                  // The Up arrow key, 0x26
    bool Right : 1;               // The Right arrow key, 0x27
    // ============ ( 5 bytes ) ===========================
    bool Down : 1;                // The Down arrow key, 0x28
    bool Select : 1;              // The Select key, 0x29
    bool Print : 1;               // The Print key, 0x2A
    bool Execute : 1;             // The Execute key, 0x2B
    bool PrintScreen : 1;         // The Print screen key, 0x2C
    bool Insert : 1;              // The Insert key, 0x2D
    bool Delete : 1;              // The Delete key, 0x2E
    bool Help : 1;                // The Help key, 0x2F
    // ============ ( 6 bytes ) ===========================
    bool D0 : 1;                  // 0, 0x30
    bool D1 : 1;                  // 1, 0x31
    bool D2 : 1;                  // 2, 0x32
    bool D3 : 1;                  // 3, 0x33
    bool D4 : 1;                  // 4, 0x34
    bool D5 : 1;                  // 5, 0x35
    bool D6 : 1;                  // 6, 0x36
    bool D7 : 1;                  // 7, 0x37
    // ============ ( 7 bytes ) ===========================
    bool D8 : 1;                  // 8, 0x38
    bool D9 : 1;                  // 9, 0x39
    bool Reserved4 : 6;           // 0x3A - 3F are undefined.
    // ============ ( 8 bytes ) ===========================
    bool Reserved5 : 1;           // 0x40 is undefined.
    bool A : 1;                   // A, 0x41
    bool B : 1;                   // B, 0x42
    bool C : 1;                   // C, 0x43
    bool D : 1;                   // D, 0x44
    bool E : 1;                   // E, 0x45
    bool F : 1;                   // F, 0x46
    bool G : 1;                   // G, 0x47
    // ============ ( 9 bytes ) ===========================
    bool H : 1;                   // H, 0x48
    bool I : 1;                   // I, 0x49
    bool J : 1;                   // J, 0x4A
    bool K : 1;                   // K, 0x4B
    bool L : 1;                   // L, 0x4C
    bool M : 1;                   // M, 0x4D
    bool N : 1;                   // N, 0x4E
    bool O : 1;                   // O, 0x4F
    // ============ ( 10 bytes ) ===========================
    bool P : 1;                   // P, 0x50
    bool Q : 1;                   // Q, 0x51
    bool R : 1;                   // R, 0x52
    bool S : 1;                   // S, 0x53
    bool T : 1;                   // T, 0x54
    bool U : 1;                   // U, 0x55
    bool V : 1;                   // V, 0x56
    bool W : 1;                   // W, 0x57
    // ============ ( 11 bytes ) ===========================
    bool X : 1;                   // X, 0x58
    bool Y : 1;                   // Y, 0x59
    bool Z : 1;                   // Z, 0x5A
    bool LeftWindows : 1;         // Left Windows key, 0x5B
    bool RightWindows : 1;        // Right Windows key, 0x5C
    bool Apps : 1;                // Apps key, 0x5D
    bool Reserved6 : 1;           // 0x5E is reserved
    bool Sleep : 1;               // The Sleep key, 0x5F
    // ============ ( 12 bytes ) ===========================
    bool NumPad0 : 1;             // The Numeric keybad 0 key, 0x60
    bool NumPad1 : 1;             // The Numeric keybad 1 key, 0x61
    bool NumPad2 : 1;             // The Numeric keybad 2 key, 0x62
    bool NumPad3 : 1;             // The Numeric keybad 3 key, 0x63
    bool NumPad4 : 1;             // The Numeric keybad 4 key, 0x64
    bool NumPad5 : 1;             // The Numeric keybad 5 key, 0x65
    bool NumPad6 : 1;             // The Numeric keybad 6 key, 0x66
    bool NumPad7 : 1;             // The Numeric keybad 7 key, 0x67
    // ============ ( 13 bytes ) ===========================
    bool NumPad8 : 1;             // The Numeric keybad 8 key, 0x68
    bool NumPad9 : 1;             // The Numeric keybad 9 key, 0x69
    bool Multiply : 1;            // The Multiply key, 0x6A
    bool Add : 1;                 // The Add key, 0x6B
    bool Separator : 1;           // The Separator key, 0x6C
    bool Subtract : 1;            // The Subtract key, 0x6D
    bool Decimal : 1;             // The Decimal key, 0x6E
    bool Divide : 1;              // The Divide key, 0x6F
    // ============ ( 14 bytes ) ===========================
    bool F1 : 1;                  // The F1 key, 0x70
    bool F2 : 1;                  // The F2 key, 0x71
    bool F3 : 1;                  // The F3 key, 0x72
    bool F4 : 1;                  // The F4 key, 0x73
    bool F5 : 1;                  // The F5 key, 0x74
    bool F6 : 1;                  // The F6 key, 0x75
    bool F7 : 1;                  // The F7 key, 0x76
    bool F8 : 1;                  // The F8 key, 0x77
    // ============ ( 15 bytes ) ===========================
    bool F9 : 1;                  // The F9 key, 0x78
    bool F10 : 1;                 // The F10 key, 0x79
    bool F11 : 1;                 // The F11 key, 0x7A
    bool F12 : 1;                 // The F12 key, 0x7B
    bool F13 : 1;                 // The F13 key, 0x7C
    bool F14 : 1;                 // The F14 key, 0x7D
    bool F15 : 1;                 // The F15 key, 0x7E
    bool F16 : 1;                 // The F16 key, 0x7F
    // ============ ( 16 bytes ) ===========================
    bool F17 : 1;                 // The F17 key, 0x80
    bool F18 : 1;                 // The F18 key, 0x81
    bool F19 : 1;                 // The F19 key, 0x82
    bool F20 : 1;                 // The F20 key, 0x83
    bool F21 : 1;                 // The F21 key, 0x84
    bool F22 : 1;                 // The F22 key, 0x85
    bool F23 : 1;                 // The F23 key, 0x86
    bool F24 : 1;                 // The F24 key, 0x87
    // ============ ( 17 bytes ) ===========================
    bool Reserved7 : 8;           // 0x88 - 8F are unassigend
    // ============ ( 18 bytes ) ===========================
    bool NumLock : 1;             // The Num Lock key, 0x90
    bool Scroll : 1;              // The Scroll Lock key, 0x91
    bool Reserved9 : 6;           // 0x92 - 0x97 are OEM specific
    // ============ ( 19 bytes ) ===========================
    bool Reserved10 : 8;          // 0x98 - 9F are unassigned
    // ============ ( 20 bytes ) ===========================
    bool LeftShift : 1;           // The Left Shift key, 0xA0
    bool RightShift : 1;          // The Right Shift key, 0xA1
    bool LeftControl : 1;         // The Left Control key, 0xA2
    bool RightControl : 1;        // The Right Control key, 0xA3
    bool LeftAlt : 1;             // The Left Alt key, 0xA4
    bool RightAlt : 1;            // The Right Alt key, 0xA5
    bool BrowserBack : 1;         // The Browser Back key, 0xA6
    bool BrowserForward : 1;      // The Browser Forward key, 0xA7
    // ============ ( 21 bytes ) ===========================
    bool BrowserRefresh : 1;      // The Browser Refresh key, 0xA8
    bool BrowserStop : 1;         // The Browser Stop key, 0xA9
    bool BrowserSearch : 1;       // The Browser Search key, 0xAA
    bool BrowserFavorites : 1;    // The Browser Favorites key, 0xAB
    bool BrowserHome : 1;         // The Browser Home key, 0xAC
    bool VolumeMute : 1;          // The Volume Mute key, 0xAD
    bool VolumeDown : 1;          // The Volume Down key, 0xAE
    bool VolumeUp : 1;            // The Volume Up key, 0xAF
    // ============ ( 22 bytes ) ===========================
    bool MediaNextTrack : 1;      // The Next Track key, 0xB0
    bool MediaPreviousTrack : 1;  // The Previous Track key, 0xB1
    bool MediaStop : 1;           // The Stop Media key, 0xB2
    bool MediaPlayPause : 1;      // The Play/Pause Media key, 0xB3
    bool LaunchMail : 1;          // The Start Mail key, 0xB4
    bool SelectMedia : 1;         // The Select Media key, 0xB5
    bool LaunchApplication1 : 1;  // Start Application 1 key, 0xB6
    bool LaunchApplication2 : 1;  // Start Application 2 key, 0xB7
    // ============ ( 23 bytes ) ===========================
    bool Reserved12 : 2;          // 0xB8 - B9 are reserved.
    bool OemSemicolon : 1;        // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key, 0xBA
    bool OemPlus : 1;             // For any country/region, the '+' key, 0xBB
    bool OemComma : 1;            // For any country/region, the ',' key, 0xBC
    bool OemMinus : 1;            // For any country/region, the '-' key, 0xBD
    bool OemPeriod : 1;           // For any country/region, the '.' key, 0xBE
    bool OemQuestion : 1;         // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key, 0xBF
    // ============ ( 24 bytes ) ===========================
    bool OemTilde : 1;            // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key, 0xC0
    bool Reserved13 : 7;          // 0xC1 - C7 are reserved.
    // ============ ( 25 bytes ) ===========================
    bool Reserved15 : 8;          // 0xC9 - CF are reserved.
    // ============ ( 26 bytes ) ===========================
    bool Reserved16 : 8;          // 0xD0 - D7 are reserved.
    // ============ ( 27 bytes ) ===========================
    bool Reserved17 : 3;          // 0xD8 - DA are unassigned.
    bool OemOpenBrackets : 1;     // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key, 0xDB
    bool OemPipe : 1;             // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key, 0xDC
    bool OemCloseBrackets : 1;    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key, 0xDD
    bool OemQuotes : 1;           // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key, 0xDE
    bool Oem8 : 1;                // Used for miscellaneous characters; it can vary by keyboard, 0xDF
    // ============ ( 28 bytes ) ===========================
    bool Reserved18 : 2;          // 0xE0 - E1 are reserved
    bool OemBackslash : 1;        // Either the angle bracket key or the backslash key on the RT 102-key keyboard, 0xE2
    bool Reserved19 : 2;          // 0xE3 - E4 are OEM specific
    bool Process : 1;             // IME Process key, 0xE5
    bool Reserved20 : 1;          // 0xE6 is OEM specific
    bool Packet : 1;              // Used to pass Unicode characters as if they were keystrokes. The Packet key value is the low word of a 32-bit virtual-key value used for non-keyboard input methods, 0xE7
    // ============ ( 29 bytes ) ===========================
    bool Reserved21 : 1;          // 0xE8 unassigned.
    bool Reserved22 : 7;          // 0xE9 - EF unassigned.
    // ============ ( 30 bytes ) ===========================
    bool Reserved23 : 6;          // 0xF0 - F5 is OEM specific.
    bool Attn : 1;                // The Attn key, 0xF6
    bool CrSel : 1;               // The CrSel key, 0xF7
    // ============ ( 31 bytes ) ===========================
    bool ExSel : 1;               // The ExSel key, 0xF8
    bool EraseEof : 1;            // The Erase EOF key, 0xF9
    bool Play : 1;                // The Play key, 0xFA
    bool Zoom : 1;                // The Zoom key, 0xFB
    bool NoName : 1;              // Reserved, 0xFC
    bool Pa1 : 1;                 // The PA1 key, 0xFD
    bool OemClear : 1;            // The Clear key, 0xFE
    bool Reserved25: 1;           // 0xFF is reserved.
    // ============ ( 32 bytes ) ===========================
    // clang-format on

    bool isKeyDown( Key key ) const noexcept;

    bool isKeyUp( Key key ) const noexcept;

    bool operator==( const State& ) const = default;
    bool operator!=( const State& ) const = default;
};  // struct State

State getState();

void reset();

bool isConnected();
};  // namespace Keyboard

class KeyboardStateTracker
{
public:
    Keyboard::State pressed;
    Keyboard::State released;
    Keyboard::State lastState;

    KeyboardStateTracker()
    {
        reset();
    }

    void update( const Keyboard::State& state );

    void reset();

    bool isKeyPressed( Keyboard::Key key ) const noexcept
    {
        return pressed.isKeyDown( key );
    }

    bool isKeyReleased( Keyboard::Key key ) const noexcept
    {
        return released.isKeyDown( key );
    }

    Keyboard::State getLastState() const noexcept
    {
        return lastState;
    }

};  // class KeyboardStateTracker

}  // namespace input