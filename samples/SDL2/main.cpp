#include <input/Input.hpp>
#include <input_test.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <format>
#include <iostream>
#include <numbers>
#include <string>
#include <unordered_map>

using namespace input;

// Window dimensions
constexpr int   WINDOW_WIDTH               = 1920;
constexpr int   WINDOW_HEIGHT              = 1080;
constexpr int   KEY_SIZE                   = 50.0f;   // The size of a key in the keyboard image (in pixels).
constexpr float PANEL_WIDTH                = 360.0f;  // The width of the state panels.
constexpr float MOUSE_STATE_PANEL_HEIGHT   = 280.0f;  // THe height of the mouse state panel.
constexpr float GAMEPAD_STATE_PANEL_HEIGHT = 550.0f;  // The height of the gamepad state panel.
constexpr float PI                         = std::numbers::pi_v<float>;

enum class FillMode
{
    Solid,
    Outline
};

const SDL_Color RED              = SDL_Color { 255, 0, 0, 127 };
const SDL_Color BLACK            = SDL_Color { 0, 0, 0, 255 };
const SDL_Color WHITE            = SDL_Color { 255, 255, 255, 255 };
const SDL_Color PANEL_BACKGROUND = SDL_Color { 240, 240, 240, 216 };
const SDL_Color PANEL_ACCENT     = SDL_Color { 64, 64, 64, 216 };

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

constexpr SDL_FRect r( float x, float y, float width = static_cast<float>( KEY_SIZE ), float height = static_cast<float>( KEY_SIZE ) )
{
    return { x, y, width, height };
}

using K = Keyboard::Key;

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
    using ButtonState::Pressed;
    using ButtonState::Released;

    Mouse::State mouseState = Mouse::getState();

    g_MouseStateTracker.update( mouseState );

    if ( g_MouseStateTracker.rightButton == Released )
    {
        switch ( mouseState.positionMode )
        {
        case Absolute:
            Mouse::setMode( Relative );
            break;
        case Relative:
            Mouse::setMode( Absolute );
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
        Keyboard::State keyboardState = Keyboard::getState();
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
        Mouse::State state = Mouse::getState();

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

// Helper function to draw a thick line using SDL_Renderer
void drawThickLineF( SDL_Color color, float x1, float y1, float x2, float y2, int thickness )
{
    // Calculate direction vector
    float dx     = x2 - x1;
    float dy     = y2 - y1;
    float length = std::sqrt( dx * dx + dy * dy );
    if ( length == 0.0f ) return;

    // Normalize perpendicular vector
    float px = -dy / length;
    float py = dx / length;

    // Half thickness
    float half = thickness / 2.0f;

    // Four corners of the thick line quad
    SDL_FPoint points[4] = {
        { x1 + px * half, y1 + py * half },
        { x1 - px * half, y1 - py * half },
        { x2 - px * half, y2 - py * half },
        { x2 + px * half, y2 + py * half }
    };

    // Convert to SDL_Vertex for filled polygon
    SDL_Vertex verts[4];
    for ( int i = 0; i < 4; ++i )
    {
        verts[i].position  = points[i];
        verts[i].color     = color;
        verts[i].tex_coord = SDL_FPoint { 0, 0 };
    }
    int indices[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry( g_pRenderer, nullptr, verts, 4, indices, 6 );
}
// Helper function to draw a circle using SDL_Renderer
void drawCircle( SDL_Color color, SDL_FPoint center, float radius )
{
    SDL_SetRenderDrawColor( g_pRenderer, color.r, color.g, color.b, color.a );

    // Midpoint ellipse algorithm (simple version)
    for ( float y = -radius; y <= radius; ++y )
    {
        float dx = radius * std::sqrt( 1.0f - ( y * y ) / ( radius * radius ) );
        SDL_RenderDrawLineF( g_pRenderer, center.x - dx, center.y + y, center.x + dx, center.y + y );
    }
}

void drawOutlineCircle( SDL_Color color, SDL_FPoint center, float radius )
{
    drawCircle( BLACK, center, radius );
    drawCircle( color, center, radius - 4.0f );
}

SDL_FPoint operator+( const SDL_FPoint& lhs, const SDL_FPoint& rhs )
{
    return { lhs.x + rhs.x, lhs.y + rhs.y };
}

void renderThumbstick( float x, float y, bool pressed, SDL_FPoint center )
{
    constexpr float thumbstickRadius = 55.0f;
    auto            offset           = SDL_FPoint { x * thumbstickRadius, y * thumbstickRadius };
    if ( pressed )
        drawCircle( RED, center, thumbstickRadius );

    drawOutlineCircle( WHITE, center + offset, 30.0f );
}

void drawRectangle( SDL_Color color, SDL_FRect rect )
{
    SDL_SetRenderDrawColor( g_pRenderer, color.r, color.g, color.b, color.a );
    SDL_RenderFillRectF( g_pRenderer, &rect );
}

void drawOutlineRectangle( SDL_Color color, SDL_FRect rect )
{
    if ( rect.w > 0.0f && rect.h > 0.0f )
        drawRectangle( color, rect );
    drawThickLineF( BLACK, rect.x, rect.y, rect.x + rect.w, rect.y, 4 );
    drawThickLineF( BLACK, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h, 4 );
    drawThickLineF( BLACK, rect.x + rect.w, rect.y + rect.h, rect.x, rect.y + rect.h, 4 );
    drawThickLineF( BLACK, rect.x, rect.y + rect.h, rect.x, rect.y, 4 );
}

void renderGamepad( const Gamepad::State& state, float x, float y )
{
    // Draw gamepad image at top left
    if ( g_pXBoxControllerTexture && g_pLeftBumperTexture && g_pRightBumperTexture && state.connected )
    {
        int texW = 0, texH = 0;
        SDL_QueryTexture( g_pXBoxControllerTexture, nullptr, nullptr, &texW, &texH );
        SDL_FRect dst = { x, y, static_cast<float>( texW ), static_cast<float>( texH ) };
        SDL_RenderCopyF( g_pRenderer, g_pXBoxControllerTexture, nullptr, &dst );

        if ( state.buttons.leftShoulder )
            SDL_RenderCopyF( g_pRenderer, g_pLeftBumperTexture, nullptr, &dst );

        if ( state.buttons.rightShoulder )
            SDL_RenderCopyF( g_pRenderer, g_pRightBumperTexture, nullptr, &dst );

        if ( state.buttons.a )
            drawCircle( RED, { x + 503, y + 177 }, 23.0f );
        if ( state.buttons.b )
            drawCircle( RED, { x + 549, y + 133 }, 23.0f );
        if ( state.buttons.x )
            drawCircle( RED, { x + 457, y + 133 }, 23.0f );
        if ( state.buttons.y )
            drawCircle( RED, { x + 505, y + 88 }, 23.0f );
        if ( state.buttons.view )
            drawCircle( RED, { x + 287, y + 133 }, 16.0f );
        if ( state.buttons.menu )
            drawCircle( RED, { x + 381, y + 133 }, 16.0f );
        if ( state.dPad.up )
            drawRectangle( RED, r( x + 233.0f, y + 193.0f, 30.0f, 30.0f ) );
        if ( state.dPad.down )
            drawRectangle( RED, r( x + 233.0f, y + 251.0f, 30.0f, 30.0f ) );
        if ( state.dPad.left )
            drawRectangle( RED, r( x + 203.0f, y + 223.0f, 30.0f, 30.0f ) );
        if ( state.dPad.right )
            drawRectangle( RED, r( x + 261.0f, y + 223.0f, 32.0f, 27.0f ) );

        renderThumbstick( state.thumbSticks.leftX, state.thumbSticks.leftY, state.buttons.leftStick, { x + 168.0f, y + 134.0f } );
        renderThumbstick( state.thumbSticks.rightX, state.thumbSticks.rightY, state.buttons.rightStick, { x + 420.0f, y + 236.0f } );

        // Triggers
        drawOutlineRectangle( RED, r( x, y, 40.0f, state.triggers.left * 130.0f ) );
        drawOutlineRectangle( RED, r( x + texW - 40.0f, y, 40.0f, state.triggers.right * 130.0f ) );
    }
}

void renderGamepads()
{
    if ( g_pXBoxControllerTexture )
    {
        int rtW = 0, rtH = 0;
        SDL_GetRendererOutputSize( g_pRenderer, &rtW, &rtH );

        int texW = 0, texH = 0;
        SDL_QueryTexture( g_pXBoxControllerTexture, nullptr, nullptr, &texW, &texH );
        float margin = 32.0f;
        float x      = margin;
        float y      = margin;

        for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
        {
            auto state = Gamepad::getState( i );
            if ( state.connected )
            {
                renderGamepad( state, x, y );

                x += texW + margin;
                if ( x + texW > rtW - PANEL_WIDTH - margin * 2 )
                {
                    x = margin;
                    y += texH + margin;
                }
            }
        }
    }
}

void renderPanel( SDL_FRect panelRect )
{
    SDL_SetRenderDrawColor( g_pRenderer, PANEL_BACKGROUND.r, PANEL_BACKGROUND.g, PANEL_BACKGROUND.b, PANEL_BACKGROUND.a );
    SDL_SetRenderDrawBlendMode( g_pRenderer, SDL_BLENDMODE_BLEND );
    SDL_RenderFillRectF( g_pRenderer, &panelRect );

    // Draw panel border
    SDL_SetRenderDrawColor( g_pRenderer, PANEL_ACCENT.r, PANEL_ACCENT.g, PANEL_ACCENT.b, PANEL_ACCENT.a );
    constexpr int borderThickness = 8;
    for ( int i = 0; i < borderThickness; ++i )
    {
        SDL_FRect borderRect = { panelRect.x - i, panelRect.y - i, panelRect.w + 2 * i, panelRect.h + 2 * i };
        SDL_RenderDrawRectF( g_pRenderer, &borderRect );
    }
}

// Draws a mouse state panel on the right side of the screen.
void renderMousePanel()
{
    if ( !g_pFontMono || !g_pRenderer )
        return;

    Mouse::State state = Mouse::getState();

    int rtW = 0, rtH = 0;
    SDL_GetRendererOutputSize( g_pRenderer, &rtW, &rtH );

    // Panel dimensions
    const float panelWidth  = PANEL_WIDTH;
    const float panelHeight = MOUSE_STATE_PANEL_HEIGHT;
    const float panelX      = rtW - panelWidth - 32;
    const float panelY      = 32;

    // Draw panel background
    SDL_FRect panelRect = { panelX, panelY, panelWidth, panelHeight };
    renderPanel( panelRect );

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
        "Position: ({:.1f}, {:.1f})\n"
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
        state.scrollWheelValue );

    SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped( g_pFontMono, text.c_str(), BLACK, panelWidth - 32 );
    if ( surf )
    {
        SDL_Texture* tex     = SDL_CreateTextureFromSurface( g_pRenderer, surf );
        SDL_FRect    dstRect = { panelX + 16, panelY + 16, static_cast<float>( surf->w ), static_cast<float>( surf->h ) };
        SDL_RenderCopyF( g_pRenderer, tex, nullptr, &dstRect );
        SDL_DestroyTexture( tex );
        SDL_FreeSurface( surf );
    }
}

void renderGamepadStatePanel( float x, float y, const Gamepad::State& gamepadState, int playerIndex )
{
    if ( !gamepadState.connected )
        return;

    // Panel dimensions
    const float panelWidth  = PANEL_WIDTH;
    const float panelHeight = GAMEPAD_STATE_PANEL_HEIGHT;
    const float panelX      = x;
    const float panelY      = y;

    // Draw panel background and border
    SDL_FRect panelRect = { panelX, panelY, panelWidth, panelHeight };
    renderPanel( panelRect );

    // Prepare gamepad state text
    std::string text = std::format(
        "Gamepad State {}\n"
        "A:          {}\n"
        "B:          {}\n"
        "X:          {}\n"
        "Y:          {}\n"
        "View:       {}\n"
        "Menu:       {}\n"
        "LS:         {}\n"
        "RS:         {}\n"
        "LB:         {}\n"
        "RB:         {}\n"
        "DPad Up:    {}\n"
        "DPad Down:  {}\n"
        "DPad Left:  {}\n"
        "DPad Right: {}\n"
        "LT:         {:.2f}\n"
        "RT:         {:.2f}\n"
        "LS:         ({:.2f}, {:.2f})\n"
        "RS:         ({:.2f}, {:.2f})\n",
        playerIndex,
        gamepadState.buttons.a ? "Down" : "Up",
        gamepadState.buttons.b ? "Down" : "Up",
        gamepadState.buttons.x ? "Down" : "Up",
        gamepadState.buttons.y ? "Down" : "Up",
        gamepadState.buttons.view ? "Down" : "Up",
        gamepadState.buttons.menu ? "Down" : "Up",
        gamepadState.buttons.leftStick ? "Down" : "Up",
        gamepadState.buttons.rightStick ? "Down" : "Up",
        gamepadState.buttons.leftShoulder ? "Down" : "Up",
        gamepadState.buttons.rightShoulder ? "Down" : "Up",
        gamepadState.dPad.up ? "Down" : "Up",
        gamepadState.dPad.down ? "Down" : "Up",
        gamepadState.dPad.left ? "Down" : "Up",
        gamepadState.dPad.right ? "Down" : "Up",
        gamepadState.triggers.left,
        gamepadState.triggers.right,
        gamepadState.thumbSticks.leftX,
        gamepadState.thumbSticks.leftY,
        gamepadState.thumbSticks.rightX,
        gamepadState.thumbSticks.rightY );

    if ( g_pFontMono && g_pRenderer )
    {
        SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped( g_pFontMono, text.c_str(), BLACK, panelWidth - 32 );
        if ( surf )
        {
            SDL_Texture* tex     = SDL_CreateTextureFromSurface( g_pRenderer, surf );
            SDL_FRect    dstRect = { panelX + 16, panelY + 16, static_cast<float>( surf->w ), static_cast<float>( surf->h ) };
            SDL_RenderCopyF( g_pRenderer, tex, nullptr, &dstRect );
            SDL_DestroyTexture( tex );
            SDL_FreeSurface( surf );
        }
    }
}

void renderGamepadStatePanels()
{
    float margin = 32.0f;
    int   rtW = 0, rtH = 0;
    SDL_GetRendererOutputSize( g_pRenderer, &rtW, &rtH );
    float left = rtW - margin - PANEL_WIDTH;
    float top  = margin * 2 + MOUSE_STATE_PANEL_HEIGHT;  // Start below the mouse state panel.

    for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
    {
        auto gamepadState = Gamepad::getState( i );

        if ( gamepadState.connected )
        {
            renderGamepadStatePanel( left, top, gamepadState, i );
            top += margin + GAMEPAD_STATE_PANEL_HEIGHT;
        }
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
    renderGamepadStatePanels();

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

        input_test();

        // Call this at the end of the frame to reset the relative mouse position.
        Mouse::resetRelativeMotion();
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