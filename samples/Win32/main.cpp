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

#include <iostream>
#include <unordered_map>

using namespace input;
using Microsoft::WRL::ComPtr;

// Forward declare callback functions.
void Keyboard_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );
void Mouse_ProcessMessage( UINT message, WPARAM wParam, LPARAM lParam );

// Globals
ComPtr<ID2D1Factory1>         g_pD2DFactory;
ComPtr<ID2D1HwndRenderTarget> g_pRenderTarget;
ComPtr<ID2D1Bitmap>           g_pKeyboardBitmap;
ComPtr<ID2D1Bitmap>           g_pMouseBitmap;
ComPtr<IDWriteFactory>        g_pDWriteFactory;
ComPtr<IDWriteTextFormat>     g_pTextFormat;
ComPtr<IDWriteTextFormat>     g_pMousePanelTextFormat;
ComPtr<ID2D1SolidColorBrush>  g_pTextBrush;

MouseStateTracker mouseStateTracker;
D2D1_POINT_2F     g_MousePosition { 0, 0 };
float             g_fMouseRotation = 0.0f;

constexpr int KEY_SIZE = 50;  // The size of a key in the keyboard image (in pixels).

// Construct a RECT from x, y, width, height.
consteval D2D1_RECT_F r( int x, int y, int width = KEY_SIZE, int height = KEY_SIZE )
{
    return { static_cast<float>( x ), static_cast<float>( y ), static_cast<float>( x + width ), static_cast<float>( y + height ) };
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

void DrawMouseStatePanel( ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* textBrush )
{
    // Panel size and margin
    const float panelWidth  = 320.0f;
    const float panelHeight = 280.0f;
    const float margin      = 20.0f;

    D2D1_SIZE_F rtSize = renderTarget->GetSize();

    // Position panel at top-right
    D2D1_RECT_F panelRect = D2D1::RectF(
        rtSize.width - panelWidth - margin,
        margin,
        rtSize.width - margin,
        margin + panelHeight );

    // Draw panel background
    ComPtr<ID2D1SolidColorBrush> panelBrush;
    renderTarget->CreateSolidColorBrush( D2D1::ColorF( D2D1::ColorF::LightGray, 0.85f ), panelBrush.GetAddressOf() );

    // Draw panel background with rounded corners
    D2D1_ROUNDED_RECT roundedPanelRect = {
        panelRect,
        16.0f,  // radiusX
        16.0f   // radiusY
    };
    renderTarget->FillRoundedRectangle( &roundedPanelRect, panelBrush.Get() );

    // Draw accent rectangle (darker, smaller, inset)
    const float accentInset = 8.0f;
    D2D1_RECT_F accentRect  = D2D1::RectF(
        panelRect.left + accentInset,
        panelRect.top + accentInset,
        panelRect.right - accentInset,
        panelRect.bottom - accentInset );
    ComPtr<ID2D1SolidColorBrush> accentBrush;
    renderTarget->CreateSolidColorBrush( D2D1::ColorF( D2D1::ColorF::Gray, 0.85f ), accentBrush.GetAddressOf() );

    // Draw accent rectangle with rounded corners
    D2D1_ROUNDED_RECT roundedAccentRect = {
        accentRect,
        12.0f,  // radiusX
        12.0f   // radiusY
    };
    renderTarget->FillRoundedRectangle( &roundedAccentRect, accentBrush.Get() );

    // Prepare mouse state text
    Mouse::State   mouseState = Mouse::get().getState();
    const wchar_t* modeStr    = MouseModeToString( mouseState.positionMode );

    wchar_t mouseText[256];
    swprintf( mouseText, 256,
              L"Mouse State\n"
              L"Position:\t(%.0f, %.0f)\n"
              L"Mode:\t%s\n"
              L"Left:\t%s\n"
              L"Middle:\t%s\n"
              L"Right:\t%s\n"
              L"X1:\t%s\n"
              L"X2:\t%s\n"
              L"Scroll:\t%d",
              mouseState.x, mouseState.y,
              modeStr,
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
}

void render()
{
    if ( g_pRenderTarget )
    {
        g_pRenderTarget->BeginDraw();
        g_pRenderTarget->Clear( D2D1::ColorF( D2D1::ColorF::White ) );

        if ( g_pKeyboardBitmap )
        {
            D2D1_SIZE_F rtSize  = g_pRenderTarget->GetSize();
            D2D1_SIZE_F bmpSize = g_pKeyboardBitmap->GetSize();

            float left = ( rtSize.width - bmpSize.width ) / 2.0f;
            float top  = rtSize.height - bmpSize.height;

            g_pRenderTarget->DrawBitmap(
                g_pKeyboardBitmap.Get(),
                D2D1::RectF( left, top, left + bmpSize.width, top + bmpSize.height ) );

            if ( g_pTextFormat && g_pTextBrush )
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
                    g_pTextFormat.Get(),
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

        if ( g_pMouseBitmap )
        {
            DrawRotatedBitmap( g_pRenderTarget.Get(), g_pMouseBitmap.Get(), g_MousePosition, g_fMouseRotation );
        }

        // Draw mouse state panel
        if ( g_pMousePanelTextFormat && g_pTextBrush )
        {
            DrawMouseStatePanel( g_pRenderTarget.Get(), g_pMousePanelTextFormat.Get(), g_pTextBrush.Get() );
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
        &g_pTextFormat );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create text format." << std::endl;
        return -8;
    }

    // Center align text horizontally
    g_pTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
    // Align text vertically to the top of the layout rectangle
    g_pTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_NEAR );

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
        &g_pMousePanelTextFormat );
    if ( FAILED( hr ) )
    {
        std::cerr << "Failed to create mouse panel text format." << std::endl;
        return -10;
    }
    g_pMousePanelTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_LEADING );
    g_pMousePanelTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_NEAR );

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
    }

    // Cleanup
    if ( g_pTextBrush )
        g_pTextBrush.Reset();
    if ( g_pTextFormat )
        g_pTextFormat.Reset();
    if ( g_pDWriteFactory )
        g_pDWriteFactory.Reset();
    if ( g_pKeyboardBitmap )
        g_pKeyboardBitmap.Reset();
    if ( g_pMouseBitmap )
        g_pMouseBitmap.Reset();
    if ( g_pRenderTarget )
        g_pRenderTarget.Reset();
    if ( g_pD2DFactory )
        g_pD2DFactory.Reset();

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
