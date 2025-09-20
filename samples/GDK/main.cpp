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
ComPtr<ID2D1SolidColorBrush>  g_pTextBrush;

MouseStateTracker mouseStateTracker;
D2D1_POINT_2F g_MousePosition { 0, 0 };
float         g_fMouseRotation = 0.0f;

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

void update()
{
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
        case Mouse::Mode::Absolute:
            mouse.setMode( Mouse::Mode::Relative );
            break;
        case Mouse::Mode::Relative:
            mouse.setMode( Mouse::Mode::Absolute );
            break;
        }
    }

    auto rtSize = g_pRenderTarget->GetSize();

    switch ( mouseState.positionMode )
    {
    case Mouse::Mode::Absolute:
        g_MousePosition = D2D1_POINT_2F { mouseState.x, mouseState.y };
        g_fMouseRotation = 0.0f;
        break;
    case Mouse::Mode::Relative:
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
        }

        if ( g_pMouseBitmap )
        {
            DrawRotatedBitmap( g_pRenderTarget.Get(), g_pMouseBitmap.Get(), g_MousePosition, g_fMouseRotation );
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
