#include <input/Input.hpp>

#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>

#include <wincodec.h>
#include <windows.h>
#include <wrl.h>

#pragma comment( lib, "d2d1" )
#pragma comment( lib, "dwrite" )
#pragma comment( lib, "windowscodecs" )

#include "input/Gamepad.hpp"

#include <cassert>
#include <iostream>
#include <unordered_map>

using namespace input;
using Microsoft::WRL::ComPtr;

enum class FillMode
{
    Solid,
    Outline
};

// Commonly used colors.
const D2D1_COLOR_F RED   = D2D1::ColorF( D2D1::ColorF::Red, 0.5f );
const D2D1_COLOR_F BLACK = D2D1::ColorF( D2D1::ColorF::Black );
const D2D1_COLOR_F WHITE = D2D1::ColorF( D2D1::ColorF::White );
const D2D1_COLOR_F PANEL_BACKGROUND = D2D1::ColorF( 0.94f, 0.94f, 0.95f, 0.85f );
const D2D1_COLOR_F PANEL_ACCENT     = D2D1::ColorF( 0.25f, 0.25f, 0.25f, 0.85f );

constexpr float KEY_SIZE                   = 50.0f;   // The size of a key in the keyboard image (in pixels).
constexpr float GAMEPAD_STATE_PANEL_HEIGHT = 550.0f;  // The height of the gamepad state panel.
constexpr float MOUSE_STATE_PANEL_HEIGHT   = 280.0f;  // The height of the moue state panel.
constexpr float PANEL_WIDTH                = 340.0f;  // The width of the state panels.

// Forward declare callback functions.
void Keyboard_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

// Globals
ComPtr<ID2D1Factory1>         g_pD2DFactory;
ComPtr<ID2D1HwndRenderTarget> g_pRenderTarget;
ComPtr<ID2D1Bitmap>           g_pKeyboardBitmap;
ComPtr<ID2D1Bitmap>           g_pMouseBitmap;
ComPtr<ID2D1Bitmap>           g_pLMBBitmap;
ComPtr<ID2D1Bitmap>           g_pRMBBitmap;
ComPtr<ID2D1Bitmap>           g_pMMBBitmap;
ComPtr<ID2D1Bitmap>           g_pScrollUpBitmap;
ComPtr<ID2D1Bitmap>           g_pScrollDownBitmap;
ComPtr<ID2D1Bitmap>           g_pXBoxControllerBitmap;
ComPtr<ID2D1Bitmap>           g_pLeftBumperBitmap;
ComPtr<ID2D1Bitmap>           g_pRightBumperBitmap;
ComPtr<IDWriteFactory>        g_pDWriteFactory;
ComPtr<IDWriteTextFormat>     g_pCenterTextFormat;
ComPtr<IDWriteTextFormat>     g_pLeftTextFormat;
ComPtr<ID2D1SolidColorBrush>  g_pTextBrush;

MouseStateTracker mouseStateTracker;
D2D1_POINT_2F     g_MousePosition { 0, 0 };
float             g_fMouseRotation = 0.0f;

template<typename T>
void SafeRelease( ComPtr<T>& ptr )
{
    if ( ptr )
        ptr.Reset();
}

// Construct a RECT from x, y, width, height.
constexpr D2D1_RECT_F r( float x, float y, float width = KEY_SIZE, float height = KEY_SIZE )
{
    return { x, y, x + width, y + height };
}

using K = Keyboard::Keys;

std::unordered_map<K, D2D1_RECT_F> g_KeyRects = {
    // Row 1
    { K::Escape, r( 24, 25 ) },
    { K::F1, r( 121, 25 ) },
    { K::F2, r( 176, 25 ) },
    { K::F3, r( 232, 25 ) },
    { K::F4, r( 287, 25 ) },
    { K::F5, r( 373, 25 ) },
    { K::F6, r( 428, 25 ) },
    { K::F7, r( 484, 25 ) },
    { K::F8, r( 539, 25 ) },
    { K::F9, r( 625, 25 ) },
    { K::F10, r( 680, 25 ) },
    { K::F11, r( 736, 25 ) },
    { K::F12, r( 791, 25 ) },
    { K::PrintScreen, r( 877, 25 ) },
    { K::Scroll, r( 933, 25 ) },
    { K::Pause, r( 988, 25 ) },

    // Row 2
    { K::OemTilde, r( 24, 98 ) },
    { K::D1, r( 79, 98 ) },
    { K::D2, r( 135, 98 ) },
    { K::D3, r( 190, 98 ) },
    { K::D4, r( 245, 98 ) },
    { K::D5, r( 301, 98 ) },
    { K::D6, r( 356, 98 ) },
    { K::D7, r( 412, 98 ) },
    { K::D8, r( 467, 98 ) },
    { K::D9, r( 522, 98 ) },
    { K::D0, r( 578, 98 ) },
    { K::OemMinus, r( 633, 98 ) },
    { K::OemPlus, r( 689, 98 ) },
    { K::Back, r( 745, 98, 97 ) },
    { K::Insert, r( 877, 98 ) },
    { K::Home, r( 933, 98 ) },
    { K::PageUp, r( 988, 98 ) },

    // Row 3
    { K::Tab, r( 24, 154, 73 ) },
    { K::Q, r( 104, 154 ) },
    { K::W, r( 159, 154 ) },
    { K::E, r( 215, 154 ) },
    { K::R, r( 270, 154 ) },
    { K::T, r( 325, 154 ) },
    { K::Y, r( 381, 154 ) },
    { K::U, r( 436, 154 ) },
    { K::I, r( 491, 154 ) },
    { K::O, r( 547, 154 ) },
    { K::P, r( 602, 154 ) },
    { K::OemOpenBrackets, r( 658, 154 ) },
    { K::OemCloseBrackets, r( 713, 154 ) },
    { K::OemPipe, r( 769, 154, 73 ) },
    { K::Delete, r( 877, 154 ) },
    { K::End, r( 932, 154 ) },
    { K::PageDown, r( 988, 154 ) },

    // Row 4
    { K::CapsLock, r( 24, 210, 97 ) },
    { K::A, r( 128, 210 ) },
    { K::S, r( 184, 210 ) },
    { K::D, r( 240, 210 ) },
    { K::F, r( 296, 210 ) },
    { K::G, r( 352, 210 ) },
    { K::H, r( 408, 210 ) },
    { K::J, r( 464, 210 ) },
    { K::K, r( 520, 210 ) },
    { K::L, r( 576, 210 ) },
    { K::OemSemicolon, r( 632, 210 ) },
    { K::OemQuotes, r( 688, 210 ) },
    { K::Enter, r( 744, 210, 98 ) },

    // Row 5
    { K::LeftShift, r( 24, 266, 122 ) },
    { K::Z, r( 152, 266 ) },
    { K::X, r( 206, 266 ) },
    { K::C, r( 261, 266 ) },
    { K::V, r( 315, 266 ) },
    { K::B, r( 369, 266 ) },
    { K::N, r( 423, 266 ) },
    { K::M, r( 477, 266 ) },
    { K::OemComma, r( 532, 266 ) },
    { K::OemPeriod, r( 586, 266 ) },
    { K::OemQuestion, r( 640, 266 ) },
    { K::RightShift, r( 696, 266, 146 ) },

    // Row 6
    { K::LeftControl, r( 24, 322, 61 ) },
    { K::LeftSuper, r( 92, 322, 61 ) },
    { K::LeftAlt, r( 160, 322, 61 ) },
    { K::Space, r( 228, 322, 340 ) },
    { K::RightAlt, r( 575, 322, 61 ) },
    { K::RightSuper, r( 643, 322, 61 ) },
    { K::Apps, r( 712, 322, 61 ) },
    { K::RightControl, r( 780, 322, 61 ) },

    // Arrow keys
    { K::Up, r( 932, 266 ) },
    { K::Left, r( 877, 322 ) },
    { K::Down, r( 932, 322 ) },
    { K::Right, r( 988, 322 ) },

    // Numpad
    { K::NumLock, r( 1074, 98 ) },
    { K::Divide, r( 1129, 98 ) },
    { K::Multiply, r( 1185, 98 ) },
    { K::Subtract, r( 1240, 98 ) },
    { K::Add, r( 1240, 154, 50, 106 ) },
    { K::Separator, r( 1240, 266, 50, 106 ) },
    { K::Decimal, r( 1184, 322 ) },
    { K::NumPad0, r( 1074, 322, 106 ) },
    { K::NumPad1, r( 1074, 266 ) },
    { K::NumPad2, r( 1129, 266 ) },
    { K::NumPad3, r( 1184, 266 ) },
    { K::NumPad4, r( 1074, 210 ) },
    { K::NumPad5, r( 1129, 210 ) },
    { K::NumPad6, r( 1184, 210 ) },
    { K::NumPad7, r( 1074, 154 ) },
    { K::NumPad8, r( 1129, 154 ) },
    { K::NumPad9, r( 1184, 154 ) },
};

// Helper: Load a bitmap from file using WIC
HRESULT LoadBitmapFromFile(
    ID2D1RenderTarget*  renderTarget,
    IWICImagingFactory* wicFactory,
    PCWSTR              uri,
    ID2D1Bitmap**       ppBitmap )
{
    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT                   hr = wicFactory->CreateDecoderFromFilename(
        uri, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder );
    if ( FAILED( hr ) )
        return hr;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame( 0, &frame );
    if ( FAILED( hr ) )
        return hr;

    ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter( &converter );
    if ( FAILED( hr ) )
        return hr;

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.f,
        WICBitmapPaletteTypeCustom );
    if ( FAILED( hr ) )
        return hr;

    return renderTarget->CreateBitmapFromWicBitmap( converter.Get(), nullptr, ppBitmap );
}

void DrawRotatedBitmap(
    ID2D1RenderTarget* renderTarget,
    ID2D1Bitmap*       bitmap,
    D2D1_POINT_2F      center,
    float              angleDegrees )
{
    // Save the current transform
    D2D1_MATRIX_3X2_F oldTransform;
    renderTarget->GetTransform( &oldTransform );

    // Set the rotation transform (angle in degrees, around center)
    D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation( angleDegrees, center );
    renderTarget->SetTransform( rotation * oldTransform );

    // Get bitmap size
    D2D1_SIZE_F bmpSize = bitmap->GetSize();

    // Draw bitmap centered at 'center'
    renderTarget->DrawBitmap(
        bitmap,
        D2D1::RectF(
            center.x - bmpSize.width / 2.0f,
            center.y - bmpSize.height / 2.0f,
            center.x + bmpSize.width / 2.0f,
            center.y + bmpSize.height / 2.0f ) );

    // Restore the previous transform
    renderTarget->SetTransform( oldTransform );
}

// Helper to convert Mouse::Mode to string
const wchar_t* MouseModeToString( Mouse::Mode mode )
{
    switch ( mode )
    {
    case Mouse::Mode::Absolute:
        return L"Absolute";
    case Mouse::Mode::Relative:
        return L"Relative";
    default:
        return L"Unknown";
    }
}

void DrawPanel(ID2D1RenderTarget* renderTarget, D2D1_RECT_F panelRect)
{
    // Draw panel background
    ComPtr<ID2D1SolidColorBrush> panelBrush;
    renderTarget->CreateSolidColorBrush( PANEL_BACKGROUND, panelBrush.GetAddressOf() );

    // Draw panel background with rounded corners
    D2D1_ROUNDED_RECT roundedPanelRect = {
        panelRect,
        16.0f, // radiusX
        16.0f  // radiusY
    };
    renderTarget->FillRoundedRectangle( &roundedPanelRect, panelBrush.Get() );

    // Draw accent rectangle
    ComPtr<ID2D1SolidColorBrush> accentBrush;
    renderTarget->CreateSolidColorBrush( PANEL_ACCENT, accentBrush.GetAddressOf() );
    renderTarget->DrawRoundedRectangle( &roundedPanelRect, accentBrush.Get(), 8.0f );
}

void DrawMouseStatePanel( float x, float y, ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* textBrush )
{
    D2D1_RECT_F panelRect = D2D1::RectF(
        x,
        y,
        x + PANEL_WIDTH,
        y + MOUSE_STATE_PANEL_HEIGHT );

    DrawPanel( renderTarget, panelRect );

    // Prepare mouse state text
    Mouse::State   mouseState = Mouse::get().getState();
    const wchar_t* modeStr    = MouseModeToString( mouseState.positionMode );

    wchar_t mouseText[256];
    swprintf( mouseText, 256,
              L"Mouse State\n"
              L"Mode:\t%s\n"
              L"Position:\t(%.1f, %.1f)\n"
              L"Left:\t%s\n"
              L"Middle:\t%s\n"
              L"Right:\t%s\n"
              L"X1:\t%s\n"
              L"X2:\t%s\n"
              L"Scroll:\t%d",
              modeStr,
              mouseState.x, mouseState.y,
              mouseState.leftButton ? L"Down" : L"Up",
              mouseState.middleButton ? L"Down" : L"Up",
              mouseState.rightButton ? L"Down" : L"Up",
              mouseState.xButton1 ? L"Down" : L"Up",
              mouseState.xButton2 ? L"Down" : L"Up",
              mouseState.scrollWheelValue );

    // Draw mouse state text
    D2D1_RECT_F textRect = D2D1::RectF(
        panelRect.left + 20.0f,
        panelRect.top + 20.0f,
        panelRect.right - 20.0f,
        panelRect.bottom - 20.0f );
    renderTarget->DrawText(
        mouseText,
        static_cast<UINT32>( wcslen( mouseText ) ),
        textFormat,
        textRect,
        textBrush );
}

void DrawGamepadStatePanel(
    float                 x,
    float                 y,
    const Gamepad::State& gamepadState, int playerIndex )
{
    if ( !gamepadState.connected )
        return;

    // Panel rectangle at (x, y)
    D2D1_RECT_F panelRect = D2D1::RectF(
        x,
        y,
        x + PANEL_WIDTH,
        y + GAMEPAD_STATE_PANEL_HEIGHT );

    DrawPanel( g_pRenderTarget.Get(), panelRect );

    // Prepare gamepad state text
    wchar_t gamepadText[512];
    swprintf( gamepadText, 512,
              L"Gamepad %i\n"
              L"A:\t\t%s\n"
              L"B:\t\t%s\n"
              L"X:\t\t%s\n"
              L"Y:\t\t%s\n"
              L"View:\t\t%s\n"
              L"Menu:\t\t%s\n"
              L"LB:\t\t%s\n"
              L"RB:\t\t%s\n"
              L"Left Stick:\t%s\n"
              L"Right Stick:\t%s\n"
              L"DPad Up:\t%s\n"
              L"DPad Down:\t%s\n"
              L"DPad Left:\t%s\n"
              L"DPad Right:\t%s\n"
              L"LT:\t\t%.2f\n"
              L"RT:\t\t%.2f\n"
              L"Left Stick:\t(%.2f, %.2f)\n"
              L"Right Stick:\t(%.2f, %.2f)",
              playerIndex,
              gamepadState.buttons.a ? L"Down" : L"Up",
              gamepadState.buttons.b ? L"Down" : L"Up",
              gamepadState.buttons.x ? L"Down" : L"Up",
              gamepadState.buttons.y ? L"Down" : L"Up",
              gamepadState.buttons.view ? L"Down" : L"Up",
              gamepadState.buttons.menu ? L"Down" : L"Up",
              gamepadState.buttons.leftShoulder ? L"Down" : L"Up",
              gamepadState.buttons.rightShoulder ? L"Down" : L"Up",
              gamepadState.buttons.leftStick ? L"Down" : L"Up",
              gamepadState.buttons.rightStick ? L"Down" : L"Up",
              gamepadState.dPad.up ? L"Down" : L"Up",
              gamepadState.dPad.down ? L"Down" : L"Up",
              gamepadState.dPad.left ? L"Down" : L"Up",
              gamepadState.dPad.right ? L"Down" : L"Up",
              gamepadState.triggers.left,
              gamepadState.triggers.right,
              gamepadState.thumbSticks.leftX,
              gamepadState.thumbSticks.leftY,
              gamepadState.thumbSticks.rightX,
              gamepadState.thumbSticks.rightY );

    // Draw gamepad state text
    D2D1_RECT_F textRect = D2D1::RectF(
        panelRect.left + 20.0f,
        panelRect.top + 20.0f,
        panelRect.right - 20.0f,
        panelRect.bottom - 20.0f );
    g_pRenderTarget->DrawText(
        gamepadText,
        static_cast<UINT32>( wcslen( gamepadText ) ),
        g_pLeftTextFormat.Get(),
        textRect,
        g_pTextBrush.Get() );
}

void update()
{
    using Mouse::Mode::Absolute;
    using Mouse::Mode::Relative;
    using MouseStateTracker::ButtonState::Pressed;
    using MouseStateTracker::ButtonState::Released;

    if ( !g_pRenderTarget )
        return;

    Mouse&       mouse      = Mouse::get();
    Mouse::State mouseState = mouse.getState();

    mouseStateTracker.update( mouseState );

    if ( mouseStateTracker.rightButton == Released )
    {
        switch ( mouseState.positionMode )
        {
        case Absolute:
            mouse.setMode( Relative );
            break;
        case Relative:
            mouse.setMode( Absolute );
            break;
        }
    }

    auto rtSize = g_pRenderTarget->GetSize();

    switch ( mouseState.positionMode )
    {
    case Absolute:
        g_MousePosition  = D2D1_POINT_2F { mouseState.x, mouseState.y };
        g_fMouseRotation = 0.0f;
        break;
    case Relative:
        g_MousePosition = D2D1_POINT_2F { rtSize.width / 2.0f, rtSize.height / 2.0f };
        g_fMouseRotation += mouseState.x + mouseState.y;
        break;
    }

    for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
    {
        Gamepad        gamepad { i };
        Gamepad::State gamepadState = gamepad.getState();

        // Test vibration.
        if ( gamepadState.connected )
        {
            float lx        = gamepadState.thumbSticks.leftX;
            float ly        = gamepadState.thumbSticks.leftY;
            float leftMotor = std::sqrt( lx * lx + ly * ly );

            float rx         = gamepadState.thumbSticks.rightX;
            float ry         = gamepadState.thumbSticks.rightY;
            float rightMotor = std::sqrt( rx * rx + ry * ry );

            float leftTrigger  = gamepadState.triggers.left;
            float rightTrigger = gamepadState.triggers.right;

            // BUG: Vibrating multiple motors when using GDK causes the controller to start vibrating continuously.
            // gamepad.setVibration( leftMotor, rightMotor, leftTrigger, rightTrigger );
        }
    }
}

void renderRectangle( D2D1_COLOR_F color, D2D1_RECT_F rect, FillMode fillMode = FillMode::Solid )
{
    ComPtr<ID2D1SolidColorBrush> brush;
    g_pRenderTarget->CreateSolidColorBrush( color, brush.GetAddressOf() );

    switch ( fillMode )
    {
    case FillMode::Solid:
        g_pRenderTarget->FillRectangle( rect, brush.Get() );
        break;
    case FillMode::Outline:
        g_pRenderTarget->DrawRectangle( rect, brush.Get(), 4.0f );
        break;
    }
}

void renderOutlineRectangle( D2D1_COLOR_F color, D2D1_RECT_F rect )
{
    renderRectangle( color, rect, FillMode::Solid );
    renderRectangle( BLACK, rect, FillMode::Outline );
}

void renderCircle( D2D1_COLOR_F color, D2D1_POINT_2F center, float radius, FillMode fillMode = FillMode::Solid )
{
    ComPtr<ID2D1SolidColorBrush> brush;
    g_pRenderTarget->CreateSolidColorBrush( color, brush.GetAddressOf() );

    D2D1_ELLIPSE ellipse = { center, radius, radius };
    switch ( fillMode )
    {
    case FillMode::Solid:
        g_pRenderTarget->FillEllipse( ellipse, brush.Get() );
        break;
    case FillMode::Outline:
        g_pRenderTarget->DrawEllipse( ellipse, brush.Get(), 4.0f );
        break;
    }
}

void renderOutlineCircle( D2D1_COLOR_F color, D2D1_POINT_2F center, float radius )
{
    renderCircle( color, center, radius, FillMode::Solid );
    renderCircle( BLACK, center, radius, FillMode::Outline );
}

D2D1_POINT_2F operator+( const D2D1_POINT_2F& lhs, const D2D1_POINT_2F& rhs )
{
    return { lhs.x + rhs.x, lhs.y + rhs.y };
}

void renderThumbStick( float x, float y, bool pressed, const D2D1_POINT_2F& center )
{
    constexpr float thumbstickRadius = 55.0f;
    auto            offset           = D2D1::Point2F( x * thumbstickRadius, -y * thumbstickRadius );
    if ( pressed )
        renderCircle( RED, center, thumbstickRadius );

    renderOutlineCircle( WHITE, center + offset, 30.0f );
}

void renderGamepad( const Gamepad::State& state, float left, float top )
{
    if ( g_pXBoxControllerBitmap && g_pLeftBumperBitmap && g_pRightBumperBitmap && state.connected )
    {
        D2D1_SIZE_F bmpSize = g_pXBoxControllerBitmap->GetSize();

        g_pRenderTarget->DrawBitmap(
            g_pXBoxControllerBitmap.Get(),
            D2D1::RectF(
                left,
                top,
                left + bmpSize.width,
                top + bmpSize.height ) );

        if ( state.buttons.a )
            renderCircle( RED, { left + 503, top + 177 }, 23.0f );
        if ( state.buttons.b )
            renderCircle( RED, { left + 549, top + 133 }, 23.0f );
        if ( state.buttons.x )
            renderCircle( RED, { left + 457, top + 133 }, 23.0f );
        if ( state.buttons.y )
            renderCircle( RED, { left + 505, top + 88 }, 23.0f );
        if ( state.buttons.view )
            renderCircle( RED, { left + 287, top + 133 }, 16.0f );
        if ( state.buttons.menu )
            renderCircle( RED, { left + 381, top + 133 }, 16.0f );
        if ( state.dPad.up )
            renderRectangle( RED, r( left + 233, top + 193, 30, 30 ) );
        if ( state.dPad.down )
            renderRectangle( RED, r( left + 233, top + 251, 30, 30 ) );
        if ( state.dPad.left )
            renderRectangle( RED, r( left + 205, top + 223, 30, 30 ) );
        if ( state.dPad.right )
            renderRectangle( RED, r( left + 261, top + 223, 32, 27 ) );

        if ( state.buttons.leftShoulder )
            g_pRenderTarget->DrawBitmap( g_pLeftBumperBitmap.Get(),
                                         D2D1::RectF(
                                             left,
                                             top,
                                             left + bmpSize.width,
                                             top + bmpSize.height ) );

        if ( state.buttons.rightShoulder )
            g_pRenderTarget->DrawBitmap( g_pRightBumperBitmap.Get(),
                                         D2D1::RectF(
                                             left,
                                             top,
                                             left + bmpSize.width,
                                             top + bmpSize.height ) );

        renderThumbStick( state.thumbSticks.leftX, state.thumbSticks.leftY, state.buttons.leftStick, { left + 168.0f, top + 134.0f } );
        renderThumbStick( state.thumbSticks.rightX, state.thumbSticks.rightY, state.buttons.rightStick, { left + 420.0f, top + 236.0f } );
        // Triggers
        renderOutlineRectangle( RED, r( left, top, 40, state.triggers.left * 130 ) );
        renderOutlineRectangle( RED, r( left + bmpSize.width - 40, top, 40, state.triggers.right * 130 ) );
    }
}

void render()
{
    if ( g_pRenderTarget )
    {
        g_pRenderTarget->BeginDraw();
        g_pRenderTarget->Clear( D2D1::ColorF( D2D1::ColorF::White ) );

        D2D1_SIZE_F rtSize = g_pRenderTarget->GetSize();

        if ( g_pKeyboardBitmap )
        {
            D2D1_SIZE_F bmpSize = g_pKeyboardBitmap->GetSize();

            float left = ( rtSize.width - bmpSize.width ) / 2.0f;
            float top  = rtSize.height - bmpSize.height;

            g_pRenderTarget->DrawBitmap(
                g_pKeyboardBitmap.Get(),
                D2D1::RectF( left, top, left + bmpSize.width, top + bmpSize.height ) );

            if ( g_pCenterTextFormat && g_pTextBrush )
            {
                const wchar_t* text       = L"By Rumudiez - Created in Adobe Illustrator, CC BY-SA 3.0, https://commons.wikimedia.org/w/index.php?curid=26015253";
                float          textHeight = 28.0f;

                D2D1_RECT_F textRect = D2D1::RectF(
                    0,
                    rtSize.height - textHeight,
                    rtSize.width,
                    rtSize.height );

                g_pRenderTarget->DrawText(
                    text,
                    static_cast<UINT32>( wcslen( text ) ),
                    g_pCenterTextFormat.Get(),
                    textRect,
                    g_pTextBrush.Get() );
            }

            ComPtr<ID2D1SolidColorBrush> highlightBrush;
            g_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF( D2D1::ColorF::Red, 0.5f ),  // semi-transparent red
                highlightBrush.GetAddressOf() );

            // Draw rectangles over pressed keys, offset by bitmap position
            Keyboard::State keyboardState = Keyboard::get().getState();
            for ( const auto& [key, rect]: g_KeyRects )
            {
                if ( keyboardState.isKeyDown( key ) )
                {
                    D2D1_RECT_F offsetRect = D2D1::RectF(
                        rect.left + left,
                        rect.top + top,
                        rect.right + left,
                        rect.bottom + top );
                    g_pRenderTarget->FillRectangle( offsetRect, highlightBrush.Get() );
                }
            }
        }

        // Draw XBox Controller bitmap at bottom right corner
        if ( g_pXBoxControllerBitmap )
        {
            auto  bmpSize = g_pXBoxControllerBitmap->GetSize();
            float margin  = 32.0f;
            float left    = margin;
            float top     = margin;

            for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
            {
                auto gamepadState = Gamepad { i }.getState();

                if ( gamepadState.connected )
                {
                    renderGamepad( gamepadState, left, top );

                    left += bmpSize.width + margin;
                    if ( left + bmpSize.width > rtSize.width - PANEL_WIDTH - margin * 2 )
                    {
                        left = margin;
                        top += bmpSize.height + margin;
                    }
                }
            }
        }

        if ( g_pMouseBitmap && g_pLMBBitmap && g_pRMBBitmap && g_pMMBBitmap )
        {
            auto mouseState = Mouse::get().getState();

            DrawRotatedBitmap( g_pRenderTarget.Get(), g_pMouseBitmap.Get(), g_MousePosition, g_fMouseRotation );
            if ( mouseState.leftButton )
                DrawRotatedBitmap( g_pRenderTarget.Get(), g_pLMBBitmap.Get(), g_MousePosition, g_fMouseRotation );
            if ( mouseState.rightButton )
                DrawRotatedBitmap( g_pRenderTarget.Get(), g_pRMBBitmap.Get(), g_MousePosition, g_fMouseRotation );
            if ( mouseState.middleButton )
                DrawRotatedBitmap( g_pRenderTarget.Get(), g_pMMBBitmap.Get(), g_MousePosition, g_fMouseRotation );
            if ( mouseStateTracker.scrollWheelDelta > 0 )
                DrawRotatedBitmap( g_pRenderTarget.Get(), g_pScrollUpBitmap.Get(), g_MousePosition, g_fMouseRotation );
            if ( mouseStateTracker.scrollWheelDelta < 0 )
                DrawRotatedBitmap( g_pRenderTarget.Get(), g_pScrollDownBitmap.Get(), g_MousePosition, g_fMouseRotation );
        }

        // Draw mouse state panel
        if ( g_pLeftTextFormat && g_pTextBrush )
        {
            DrawMouseStatePanel( rtSize.width - 32 - PANEL_WIDTH, 32, g_pRenderTarget.Get(), g_pLeftTextFormat.Get(), g_pTextBrush.Get() );
        }

        // Draw gamepad state panels.
        {
            float margin = 32.0f;
            float left   = rtSize.width - margin - PANEL_WIDTH;
            float top    = margin * 2 + MOUSE_STATE_PANEL_HEIGHT;

            for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
            {
                auto gamepadState = Gamepad { i }.getState();

                if ( gamepadState.connected )
                {
                    DrawGamepadStatePanel( left, top, gamepadState, i );
                    top += margin + GAMEPAD_STATE_PANEL_HEIGHT;
                }
            }
        }

        g_pRenderTarget->EndDraw();
    }
}

// Forward declarations
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
    // Register window class
    WNDCLASS wc      = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "SampleWindowClass";
    RegisterClass( &wc );

    // Desired window size
    int windowWidth  = 1920;
    int windowHeight = 1080;

    // Get screen size
    int screenWidth  = GetSystemMetrics( SM_CXSCREEN );
    int screenHeight = GetSystemMetrics( SM_CYSCREEN );

    // Calculate top-left position to center the window
    int x = ( screenWidth - windowWidth ) / 2;
    int y = ( screenHeight - windowHeight ) / 2;

    // Create window centered on screen
    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        "Game Development Toolkit (GDK)",
        WS_OVERLAPPEDWINDOW,
        x, y, windowWidth, windowHeight,
        nullptr, nullptr, hInstance, nullptr );

    if ( !hwnd )
    {
        std::cerr << "Failed to create window." << std::endl;
        return -1;
    }

    // Register window with mouse:
    Mouse::get().setWindow( hwnd );

    ShowWindow( hwnd, nCmdShow );

    // Initialize Direct2D
    HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS( g_pD2DFactory.GetAddressOf() ) );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to initialize Direct2D factory." << std::endl;
        return -2;
    }

    RECT rc;
    GetClientRect( hwnd, &rc );

    hr = g_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties( hwnd, D2D1::SizeU( rc.right - rc.left, rc.bottom - rc.top ) ),
        g_pRenderTarget.GetAddressOf() );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create render target." << std::endl;
        return -3;
    }

    // Initialize WIC
    ComPtr<IWICImagingFactory> wicFactory;
    hr = CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to initialize COM." << std::endl;
        return -4;
    }

    hr = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS( &wicFactory ) );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create WIC factory." << std::endl;
        return -5;
    }

    // Load keyboard bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/ANSI_Keyboard_Layout.png",
        &g_pKeyboardBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load bitmap." << std::endl;
        return -6;
    }

    // Load mouse bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/Mouse.png",
        &g_pMouseBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load bitmap." << std::endl;
        return -6;
    }

    // Load LMB bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/LMB.png",
        &g_pLMBBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load LMB bitmap." << std::endl;
        return -6;
    }

    // Load RMB bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/RMB.png",
        &g_pRMBBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load RMB bitmap." << std::endl;
        return -6;
    }

    // Load MMB bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/MMB.png",
        &g_pMMBBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load MMB bitmap." << std::endl;
        return -6;
    }

    // Load Scroll Up bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/Scroll_Up.png",
        &g_pScrollUpBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load Scroll Up bitmap." << std::endl;
        return -6;
    }

    // Load Scroll Down bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/Scroll_Down.png",
        &g_pScrollDownBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load Scroll Down bitmap." << std::endl;
        return -6;
    }

    // Load XBox Controller bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/XBox Controller.png",
        &g_pXBoxControllerBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load XBox Controller bitmap." << std::endl;
        return -6;
    }

    // Load Left Bumper bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/Left_Bumper.png",
        &g_pLeftBumperBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load Left Bumper bitmap." << std::endl;
        return -6;
    }

    // Load Right Bumper bitmap
    hr = LoadBitmapFromFile(
        g_pRenderTarget.Get(),
        wicFactory.Get(),
        L"assets/Right_Bumper.png",
        &g_pRightBumperBitmap );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to load Right Bumper bitmap." << std::endl;
        return -6;
    }

    // Initialize DirectWrite
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof( IDWriteFactory ),
        reinterpret_cast<IUnknown**>( g_pDWriteFactory.GetAddressOf() ) );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create DirectWrite factory." << std::endl;
        return -7;
    }

    hr = g_pDWriteFactory->CreateTextFormat(
        L"Segoe UI",  // Font family
        nullptr,      // Font collection
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.0f,  // Font size
        L"en-us",
        &g_pCenterTextFormat );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create text format." << std::endl;
        return -8;
    }

    // Center align text horizontally
    g_pCenterTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
    // Align text vertically to the top of the layout rectangle
    g_pCenterTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_NEAR );

    // Create a solid color brush for text
    hr = g_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF( D2D1::ColorF::Black ),
        &g_pTextBrush );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create text brush." << std::endl;
        return -9;
    }

    // Create a text format for the mouse state panel (left aligned)
    hr = g_pDWriteFactory->CreateTextFormat(
        L"Segoe UI",  // Font family
        nullptr,      // Font collection
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.0f,  // Font size
        L"en-us",
        &g_pLeftTextFormat );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create mouse panel text format." << std::endl;
        return -10;
    }
    g_pLeftTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_LEADING );
    g_pLeftTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_NEAR );

    // Message loop
    MSG  msg     = {};
    bool running = true;
    while ( running )
    {
        // Process all pending messages
        while ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            if ( msg.message == WM_QUIT )
            {
                running = false;
                break;
            }
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        update();
        render();

        Mouse::get().resetRelativeMotion();
    }

    // Cleanup
    SafeRelease( g_pTextBrush );
    SafeRelease( g_pCenterTextFormat );
    SafeRelease( g_pDWriteFactory );
    SafeRelease( g_pKeyboardBitmap );
    SafeRelease( g_pMouseBitmap );
    SafeRelease( g_pLMBBitmap );
    SafeRelease( g_pRMBBitmap );
    SafeRelease( g_pMMBBitmap );
    SafeRelease( g_pScrollUpBitmap );
    SafeRelease( g_pScrollDownBitmap );
    SafeRelease( g_pRenderTarget );
    SafeRelease( g_pD2DFactory );
    SafeRelease( g_pXBoxControllerBitmap );
    SafeRelease( g_pLeftBumperBitmap );
    SafeRelease( g_pRightBumperBitmap );

    CoUninitialize();

    return 0;
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    // Keyboard callback.
    Keyboard_ProcessMessage( msg, wParam, lParam );
    Mouse_ProcessMessage( msg, wParam, lParam );

    switch ( msg )
    {
    case WM_SIZE:
        if ( g_pRenderTarget )
        {
            UINT        width   = LOWORD( lParam );
            UINT        height  = HIWORD( lParam );
            D2D1_SIZE_U newSize = D2D1::SizeU( width, height );
            g_pRenderTarget->Resize( newSize );
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint( hwnd, &ps );
        EndPaint( hwnd, &ps );
    }
    break;
    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;
    default:
        return DefWindowProc( hwnd, msg, wParam, lParam );
    }
    return 0;
}