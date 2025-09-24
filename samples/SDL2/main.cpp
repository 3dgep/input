#include <input/Input.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <format>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace input;

// Window dimensions
constexpr int WINDOW_WIDTH  = 1920;
constexpr int WINDOW_HEIGHT = 1080;
constexpr int KEY_SIZE      = 50.0f;  // The size of a key in the keyboard image (in pixels).

SDL_Renderer* g_pRenderer = nullptr;
SDL_Window*   g_pWindow   = nullptr;

SDL_Texture* g_pKeyboardTexture       = nullptr;
SDL_Texture* g_pMouseTexture          = nullptr;
SDL_Texture* g_pLMBTexture            = nullptr;
SDL_Texture* g_pRMBTexture            = nullptr;
SDL_Texture* g_pMMBTexture            = nullptr;
SDL_Texture* g_pScrollUpTexture       = nullptr;
SDL_Texture* g_pScrollDownTexture     = nullptr;
SDL_Texture* g_pXBoxControllerTexture = nullptr;
SDL_Texture* g_pLeftBumperTexture     = nullptr;
SDL_Texture* g_pRightBumperTexture    = nullptr;
TTF_Font*    g_pFont                  = nullptr;
TTF_Font*    g_pFontMono              = nullptr;

MouseStateTracker g_MouseStateTracker;
SDL_FPoint        g_MousePosition { 0, 0 };
float             g_fMouseRotation = 0.0f;

// Construct a RECT from x, y, width, height.
constexpr SDL_Rect r( int x, int y, int width = KEY_SIZE, int height = KEY_SIZE )
{
    return { x, y, width, height };
}

using K = Keyboard::Keys;

std::unordered_map<K, SDL_Rect> g_KeyRects = {
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

// Helper to load a texture from file
SDL_Texture* LoadTexture( SDL_Renderer* renderer, const std::string& path )
{
    int            width, height, channels;
    unsigned char* data = stbi_load( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );  // force RGBA
    if ( !data )
    {
        std::cerr << "Failed to load image: " << path << " stb_image Error: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat( 0, width, height, 32, SDL_PIXELFORMAT_RGBA32 );
    if ( !surface )
    {
        std::cerr << "Failed to create SDL_Surface: " << SDL_GetError() << std::endl;
        stbi_image_free( data );
        return nullptr;
    }

    std::memcpy( surface->pixels, data, width * height * 4 );
    stbi_image_free( data );

    SDL_Texture* texture = SDL_CreateTextureFromSurface( renderer, surface );
    SDL_FreeSurface( surface );
    return texture;
}

void update()
{
    using Mouse::Mode::Absolute;
    using Mouse::Mode::Relative;
    using MouseStateTracker::ButtonState::Pressed;
    using MouseStateTracker::ButtonState::Released;

    Mouse&       mouse      = Mouse::get();
    Mouse::State mouseState = mouse.getState();

    g_MouseStateTracker.update( mouseState );

    if ( g_MouseStateTracker.rightButton == Released )
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

    int rtW = 0, rtH = 0;
    SDL_GetRendererOutputSize( g_pRenderer, &rtW, &rtH );

    switch ( mouseState.positionMode )
    {
    case Absolute:
        g_MousePosition  = SDL_FPoint { mouseState.x, mouseState.y };
        g_fMouseRotation = 0.0f;
        break;
    case Relative:
        g_MousePosition = SDL_FPoint { static_cast<float>( rtW ) / 2.0f, static_cast<float>( rtH ) / 2.0f };
        g_fMouseRotation += mouseState.x + mouseState.y;
        break;
    }
}

void renderKeyboard()
{
    // Draw keyboard image at bottom center
    if ( g_pKeyboardTexture )
    {
        int texW = 0, texH = 0;
        SDL_QueryTexture( g_pKeyboardTexture, nullptr, nullptr, &texW, &texH );
        int rtW = 0, rtH = 0;
        SDL_GetRendererOutputSize( g_pRenderer, &rtW, &rtH );
        int left = ( rtW - texW ) / 2;
        int top  = rtH - texH;

        SDL_Rect dst = { left, top, texW, texH };
        SDL_RenderCopy( g_pRenderer, g_pKeyboardTexture, nullptr, &dst );

        if ( g_pFont )
        {
            const char*  attribution = "By Rumudiez - Created in Adobe Illustrator, CC BY-SA 3.0, https://commons.wikimedia.org/w/index.php?curid=26015253";
            SDL_Color    color       = { 0, 0, 0, 255 };
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended( g_pFont, attribution, color );
            if ( textSurface )
            {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface( g_pRenderer, textSurface );
                int          textW       = textSurface->w;
                int          textH       = textSurface->h;
                SDL_FreeSurface( textSurface );

                SDL_Rect textRect = {
                    ( rtW - textW ) / 2,
                    rtH - textH,
                    textW,
                    textH
                };
                SDL_RenderCopy( g_pRenderer, textTexture, nullptr, &textRect );
                SDL_DestroyTexture( textTexture );
            }
        }

        // Set semi-transparent red color
        SDL_SetRenderDrawColor( g_pRenderer, 255, 0, 0, 128 );
        SDL_SetRenderDrawBlendMode( g_pRenderer, SDL_BLENDMODE_BLEND );

        // Draw rectangles over pressed keys, offset by bitmap position
        Keyboard::State keyboardState = Keyboard::get().getState();
        for ( const auto& [key, rect]: g_KeyRects )
        {
            if ( keyboardState.isKeyDown( key ) )
            {
                // Draw a rectangle over the pressed key.
                SDL_Rect highlightRect = {
                    rect.x + left,
                    rect.y + top,
                    rect.w, rect.h
                };
                SDL_RenderFillRect( g_pRenderer, &highlightRect );
            }
        }
    }
}

void renderMouse()
{
    if ( g_pMouseTexture && g_pLMBTexture && g_pRMBTexture && g_pMMBTexture && g_pScrollUpTexture && g_pScrollDownTexture )
    {
        Mouse::State state = Mouse::get().getState();

        int texW = 0, texH = 0;
        SDL_QueryTexture( g_pMouseTexture, nullptr, nullptr, &texW, &texH );
        SDL_Rect dst = { static_cast<int>( g_MousePosition.x ) - texW / 2, static_cast<int>( g_MousePosition.y ) - texH / 2, texW, texH };
        SDL_RenderCopyEx( g_pRenderer, g_pMouseTexture, nullptr, &dst, g_fMouseRotation, nullptr, SDL_FLIP_NONE );
        if ( state.leftButton )
            SDL_RenderCopyEx( g_pRenderer, g_pLMBTexture, nullptr, &dst, g_fMouseRotation, nullptr, SDL_FLIP_NONE );
        if ( state.rightButton )
            SDL_RenderCopyEx( g_pRenderer, g_pRMBTexture, nullptr, &dst, g_fMouseRotation, nullptr, SDL_FLIP_NONE );
        if ( state.middleButton )
            SDL_RenderCopyEx( g_pRenderer, g_pMMBTexture, nullptr, &dst, g_fMouseRotation, nullptr, SDL_FLIP_NONE );
        if ( g_MouseStateTracker.scrollWheelDelta > 0 )
            SDL_RenderCopyEx( g_pRenderer, g_pScrollUpTexture, nullptr, &dst, g_fMouseRotation, nullptr, SDL_FLIP_NONE );
        if ( g_MouseStateTracker.scrollWheelDelta < 0 )
            SDL_RenderCopyEx( g_pRenderer, g_pScrollDownTexture, nullptr, &dst, g_fMouseRotation, nullptr, SDL_FLIP_NONE );
    }
}

void renderGamepads()
{
    // Draw gamepad image at top left
    if ( g_pXBoxControllerTexture )
    {
        int texW = 0, texH = 0;
        SDL_QueryTexture( g_pXBoxControllerTexture, nullptr, nullptr, &texW, &texH );
        SDL_Rect dst = { 32, 32, texW, texH };
        SDL_RenderCopy( g_pRenderer, g_pXBoxControllerTexture, nullptr, &dst );
    }
}

// Draws a mouse state panel on the right side of the screen.
void renderMousePanel()
{
    if ( !g_pFontMono || !g_pRenderer )
        return;

    Mouse::State state = Mouse::get().getState();

    int rtW = 0, rtH = 0;
    SDL_GetRendererOutputSize( g_pRenderer, &rtW, &rtH );

    // Panel dimensions
    const int panelWidth  = 320;
    const int panelHeight = 280;
    const int panelX      = rtW - panelWidth - 32;
    const int panelY      = 32;

    // Draw panel background
    SDL_Rect panelRect = { panelX, panelY, panelWidth, panelHeight };
    SDL_SetRenderDrawColor( g_pRenderer, 240, 240, 240, 220 );
    SDL_SetRenderDrawBlendMode( g_pRenderer, SDL_BLENDMODE_BLEND );
    SDL_RenderFillRect( g_pRenderer, &panelRect );

    // Draw panel border
    SDL_SetRenderDrawColor( g_pRenderer, 64, 64, 64, 255 );
    const int borderThickness = 8;
    for (int i = 0; i < borderThickness; ++i) {
        SDL_Rect borderRect = { panelRect.x - i, panelRect.y - i, panelRect.w + 2 * i, panelRect.h + 2 * i };
        SDL_RenderDrawRect(g_pRenderer, &borderRect);
    }

    // Prepare mouse state text
    std::string mouseMode = ( state.positionMode == Mouse::Mode::Absolute ) ? "Absolute" : "Relative";
    std::string leftBtn   = state.leftButton ? "Down" : "Up";
    std::string rightBtn  = state.rightButton ? "Down" : "Up";
    std::string middleBtn = state.middleButton ? "Down" : "Up";
    std::string x1Btn     = state.xButton1 ? "Down" : "Up";
    std::string x2Btn     = state.xButton2 ? "Down" : "Up";

    std::string text = std::format(
        "Mouse State\n"
        "Mode:     {}\n"
        "Position: ({:.0f}, {:.0f})\n"
        "Left:     {}\n"
        "Middle:   {}\n"
        "Right:    {}\n"
        "X1:       {}\n"
        "X2:       {}\n"
        "Scroll:   {}",
        mouseMode,
        state.x, state.y,
        leftBtn,
        middleBtn,
        rightBtn,
        x1Btn,
        x2Btn,
        state.scrollWheelValue 
    );

    SDL_Color    textColor = { 32, 32, 32, 255 };
    SDL_Surface* surf      = TTF_RenderUTF8_Blended_Wrapped( g_pFontMono, text.c_str(), textColor, panelWidth - 32 );
    if ( surf )
    {
        SDL_Texture* tex     = SDL_CreateTextureFromSurface( g_pRenderer, surf );
        SDL_Rect     dstRect = { panelX + 16, panelY + 16, surf->w, surf->h };
        SDL_RenderCopy( g_pRenderer, tex, nullptr, &dstRect );
        SDL_DestroyTexture( tex );
        SDL_FreeSurface( surf );
    }
}

void render()
{
    SDL_SetRenderDrawColor( g_pRenderer, 255, 255, 255, 255 );
    SDL_RenderClear( g_pRenderer );

    renderKeyboard();
    renderGamepads();
    renderMouse();
    renderMousePanel();

    SDL_RenderPresent( g_pRenderer );
}

int main( int argc, char* argv[] )
{
    if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if ( TTF_Init() != 0 )
    {
        std::cerr << "TTF_Init error: " << TTF_GetError() << std::endl;
        return -2;
    }

    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

    g_pWindow = SDL_CreateWindow(
        "Simple DirectMedia Layer (SDL2)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    if ( !g_pWindow )
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    g_pRenderer = SDL_CreateRenderer( g_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if ( !g_pRenderer )
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow( g_pWindow );
        SDL_Quit();
        return -1;
    }

    // Load assets (example: keyboard, mouse, gamepad images)
    g_pKeyboardTexture       = LoadTexture( g_pRenderer, "assets/ANSI_Keyboard_Layout.png" );
    g_pMouseTexture          = LoadTexture( g_pRenderer, "assets/Mouse.png" );
    g_pXBoxControllerTexture = LoadTexture( g_pRenderer, "assets/XBox Controller.png" );
    g_pLMBTexture            = LoadTexture( g_pRenderer, "assets/LMB.png" );
    g_pRMBTexture            = LoadTexture( g_pRenderer, "assets/RMB.png" );
    g_pMMBTexture            = LoadTexture( g_pRenderer, "assets/MMB.png" );
    g_pScrollUpTexture       = LoadTexture( g_pRenderer, "assets/Scroll_Up.png" );
    g_pScrollDownTexture     = LoadTexture( g_pRenderer, "assets/Scroll_Down.png" );
    g_pLeftBumperTexture     = LoadTexture( g_pRenderer, "assets/Left_Bumper.png" );
    g_pRightBumperTexture    = LoadTexture( g_pRenderer, "assets/Right_Bumper.png" );
    g_pFont                  = TTF_OpenFont( "assets/Roboto/Regular.ttf", 20 );
    g_pFontMono              = TTF_OpenFont( "assets/RobotoMono/Regular.ttf", 20 );

    bool      running = true;
    SDL_Event event;
    while ( running )
    {
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_QUIT )
                running = false;
            // Handle keyboard, mouse, and gamepad events here
        }

        update();
        render();

        // Call this at the end of the frame to reset the relative mouse position.
        Mouse::get().endOfInputFrame();
    }

    // Cleanup
    SDL_DestroyTexture( g_pKeyboardTexture );
    SDL_DestroyTexture( g_pMouseTexture );
    SDL_DestroyTexture( g_pLMBTexture );
    SDL_DestroyTexture( g_pRMBTexture );
    SDL_DestroyTexture( g_pMMBTexture );
    SDL_DestroyTexture( g_pScrollUpTexture );
    SDL_DestroyTexture( g_pScrollDownTexture );
    SDL_DestroyTexture( g_pXBoxControllerTexture );
    SDL_DestroyTexture( g_pLeftBumperTexture );
    SDL_DestroyTexture( g_pRightBumperTexture );
    TTF_CloseFont( g_pFont );
    TTF_CloseFont( g_pFontMono );
    SDL_DestroyRenderer( g_pRenderer );
    SDL_DestroyWindow( g_pWindow );
    SDL_Quit();
    return 0;
}