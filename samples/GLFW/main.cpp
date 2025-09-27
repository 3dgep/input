#include <input/Input.hpp>

#define NOMINMAX
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numbers>
#include <print>
#include <unordered_map>
#include <vector>

using namespace input;

struct Font
{
    stbtt_bakedchar cdata[96] {};  // ASCII 32...126
    stbtt_fontinfo  fontInfo {};
    GLuint          fontTexture = 0;
    float           fontHeight  = 0.0f;
    float           ascent      = 0.0f;
    float           descent     = 0.0f;
    float           lineGap     = 0.0f;
};

struct Point
{
    float x, y;
};

struct Rect
{
    float x, y, w, h;
};

struct Color
{
    float r, g, b, a;
};

const Color RED { 1.0f, 0.0f, 0.0f, 0.5f };
const Color BLACK { 0.0f, 0.0f, 0.0f, 1.0f };
const Color WHITE { 1.0f, 1.0f, 1.0f, 1.0f };
const Color PANEL_BACKGROUND { 0.95f, 0.94f, 0.94f, 0.85f };
const Color PANEL_ACCENT { 0.25f, 0.25f, 0.25f, 0.85f };

constexpr int   WINDOW_WIDTH               = 1920;
constexpr int   WINDOW_HEIGHT              = 1080;
constexpr int   KEY_SIZE                   = 50.0f;   // The size of a key in the keyboard image (in pixels).
constexpr float PANEL_WIDTH                = 360.0f;  // The width of the state panels.
constexpr float MOUSE_STATE_PANEL_HEIGHT   = 280.0f;  // THe height of the mouse state panel.
constexpr float GAMEPAD_STATE_PANEL_HEIGHT = 550.0f;  // The height of the gamepad state panel.
constexpr float PI                         = std::numbers::pi_v<float>;

GLFWwindow* g_pWindow             = nullptr;
GLuint      g_pKeyboardTexture    = 0;
GLuint      g_pMouseTexture       = 0;
GLuint      g_pLMBTexture         = 0;
GLuint      g_pRMBTexture         = 0;
GLuint      g_pMMBTexture         = 0;
GLuint      g_pScrollUpTexture    = 0;
GLuint      g_pScrollDownTexture  = 0;
GLuint      g_pGamepadTexture     = 0;
GLuint      g_pLeftBumperTexture  = 0;
GLuint      g_pRightBumperTexture = 0;
Font*       g_pFont               = nullptr;
Font*       g_pFontMono           = nullptr;

float             g_WindowXScale = 1.0f;
float             g_WindowYScale = 1.0f;
MouseStateTracker g_MouseStateTracker;
Point             g_MousePosition { 0, 0 };
float             g_fMouseRotation = 0.0f;

// Forward-declare callback functions.
void Mouse_ScrollCallback( GLFWwindow*, double, double );
void Mouse_CursorPosCallback( GLFWwindow*, double, double );
void Mouse_ButtonCallback( GLFWwindow*, int, int, int );
void Keyboard_Callback( GLFWwindow* window, int key, int scancode, int action, int mods );

// Construct a RECT from x, y, width, height.
constexpr Rect r( float x, float y, float width = static_cast<float>( KEY_SIZE ), float height = static_cast<float>( KEY_SIZE ) )
{
    return { .x = x, .y = y, .w = width, .h = height };
}

using K = Keyboard::Keys;

std::unordered_map<K, Rect> g_KeyRects = {
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

Font* loadFont( const std::filesystem::path& fontFile, float pixelHeight = 20.0f )
{
    constexpr size_t BMP_WIDTH  = 512;
    constexpr size_t BMP_HEIGHT = 512;

    std::vector<unsigned char> ttBuffer;
    std::ifstream              file( fontFile, std::ios::binary | std::ios::ate );
    if ( !file )
    {
        std::cerr << "Failed to open font file: " << fontFile << std::endl;
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg( 0, std::ios::beg );
    ttBuffer.resize( size );
    if ( !file.read( reinterpret_cast<char*>( ttBuffer.data() ), size ) )
    {
        std::cerr << "Failed to read font file: " << fontFile << std::endl;
        return nullptr;
    }

    std::vector<unsigned char> bitmap( BMP_WIDTH * BMP_HEIGHT );  // How do we know how big the bitmap has to be?

    Font* font = new Font();
    stbtt_InitFont( &font->fontInfo, ttBuffer.data(), stbtt_GetFontOffsetForIndex( ttBuffer.data(), 0 ) );
    int ascent, decent, lineGap;
    stbtt_GetFontVMetrics( &font->fontInfo, &ascent, &decent, &lineGap );
    float scale   = stbtt_ScaleForPixelHeight( &font->fontInfo, pixelHeight );
    font->ascent  = ascent * scale;
    font->descent = decent * scale;
    font->lineGap = lineGap * scale;

    stbtt_BakeFontBitmap( ttBuffer.data(), 0, pixelHeight, bitmap.data(), BMP_WIDTH, BMP_HEIGHT, 32, 96, font->cdata );

    glGenTextures( 1, &font->fontTexture );
    glBindTexture( GL_TEXTURE_2D, font->fontTexture );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, BMP_WIDTH, BMP_WIDTH, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap.data() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glBindTexture( GL_TEXTURE_2D, 0 );

    font->fontHeight = pixelHeight;
    return font;
}

void deleteFont( Font*& font )
{
    if ( !font )
        return;
    if ( font->fontTexture )
        glDeleteTextures( 1, &font->fontTexture );

    delete font;
    font = nullptr;
}

GLuint loadTexture( const std::filesystem::path& fileName )
{
    int            width, height, channels;
    unsigned char* data = stbi_load( fileName.string().c_str(), &width, &height, &channels, STBI_rgb_alpha );
    if ( !data )
    {
        std::cerr << "Failed to load image: " << fileName << std::endl;
        return 0;
    }

    GLuint textureID;

    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
    glGenerateMipmap( GL_TEXTURE_2D );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    // Set border color to transparent white
    GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    stbi_image_free( data );
    glBindTexture( GL_TEXTURE_2D, 0 );

    return textureID;
}

void getRenderTargetSize( float& w, float& h )
{
    int rtW = 0, rtH = 0;
    glfwGetWindowSize( g_pWindow, &rtW, &rtH );

    w = rtW / g_WindowXScale;
    h = rtH / g_WindowYScale;
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

    float rtW = 0, rtH = 0;
    getRenderTargetSize( rtW, rtH );

    switch ( mouseState.positionMode )
    {
    case Absolute:
        g_MousePosition  = Point { .x = mouseState.x / g_WindowXScale, .y = mouseState.y / g_WindowYScale };
        g_fMouseRotation = 0.0f;
        break;
    case Relative:
        g_MousePosition = Point { .x = rtW / 2.0f, .y = rtH / 2.0f };
        g_fMouseRotation += mouseState.x + mouseState.y;
        break;
    }
}

void measureText( const Font* font, const char* text, float& outWidth, float& outHeight )
{
    outWidth  = 0.0f;
    outHeight = font ? font->fontHeight : 0.0f;
    if ( !font || !text )
        return;

    float width = 0.0f;
    float maxY  = -FLT_MAX;
    for ( const char* p = text; *p; ++p )
    {
        unsigned char c = static_cast<unsigned char>( *p );
        if ( c < 32 || c >= 128 )
            continue;
        const stbtt_bakedchar& bc = font->cdata[c - 32];
        width += bc.xadvance;
        float glyphHeight = static_cast<float>( bc.y1 - bc.y0 );
        maxY              = std::max( glyphHeight, maxY );
    }

    outWidth = width;
    if ( maxY > 0 )
        outHeight = maxY;
}

// Draw text at (x, y) in screen coordinates using a Font
void drawText( const Font* font, const char* text, float x, float y, const Color& color )
{
    if ( !font || !text || !font->fontTexture )
        return;

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, font->fontTexture );

    // Set up orthographic projection
    float rtW = 0, rtH = 0;
    getRenderTargetSize( rtW, rtH );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, rtW, rtH, 0, -1, 1 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glColor4fv( &color.r );

    float         px       = x;
    float         py       = y;
    constexpr int bmpWidth = 512, bmpHeight = 512;

    glBegin( GL_QUADS );
    for ( const char* p = text; *p; ++p )
    {
        unsigned char c = static_cast<unsigned char>( *p );
        if ( c < 32 || c >= 128 )
            continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad( font->cdata, bmpWidth, bmpHeight, c - 32, &px, &py, &q, 1 );

        glTexCoord2f( q.s0, q.t0 );
        glVertex2f( q.x0, q.y0 );
        glTexCoord2f( q.s1, q.t0 );
        glVertex2f( q.x1, q.y0 );
        glTexCoord2f( q.s1, q.t1 );
        glVertex2f( q.x1, q.y1 );
        glTexCoord2f( q.s0, q.t1 );
        glVertex2f( q.x0, q.y1 );
    }
    glEnd();

    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
}

void getTextureSize( GLuint texId, float& w, float& h )
{
    glGetTextureLevelParameterfv( texId, 0, GL_TEXTURE_WIDTH, &w );
    glGetTextureLevelParameterfv( texId, 0, GL_TEXTURE_HEIGHT, &h );
}

void renderTexture( GLuint texId, float x, float y )
{
    // Bind keyboard texture
    glBindTexture( GL_TEXTURE_2D, texId );

    // Get texture size.
    float texWidth = 0, texHeight = 0;
    getTextureSize( texId, texWidth, texHeight );

    // Get render target size.
    float rtW = 0, rtH = 0;
    getRenderTargetSize( rtW, rtH );

    // Enable 2D texturing and set up simple orthographic projection
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, rtW, rtH, 0, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    // Draw textured quad
    glEnable( GL_TEXTURE_2D );
    glColor4f( 1, 1, 1, 1 );
    glBegin( GL_QUADS );
    {
        glTexCoord2f( 0, 0 );
        glVertex2f( x, y );
        glTexCoord2f( 1, 0 );
        glVertex2f( x + texWidth, y );
        glTexCoord2f( 1, 1 );
        glVertex2f( x + texWidth, y + texHeight );
        glTexCoord2f( 0, 1 );
        glVertex2f( x, y + texHeight );
    }
    glEnd();
    glDisable( GL_TEXTURE_2D );

    // Restore matrices
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    glBindTexture( GL_TEXTURE_2D, 0 );
}

void renderTextureRotated( GLuint texId, Point center, float angle )
{
    // Get texture size
    float texWidth = 0, texHeight = 0;
    getTextureSize( texId, texWidth, texHeight );

    // Get render target size
    float rtW = 0, rtH = 0;
    getRenderTargetSize( rtW, rtH );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, rtW, rtH, 0, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    // Move to center, rotate, move back
    glTranslatef( center.x, center.y, 0.0f );
    glRotatef( angle, 0.0f, 0.0f, 1.0f );
    glTranslatef( -texWidth * 0.5f, -texHeight * 0.5f, 0.0f );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texId );
    glColor4f( 1, 1, 1, 1 );

    glBegin( GL_QUADS );
    {
        glTexCoord2f( 0, 0 );
        glVertex2f( 0, 0 );
        glTexCoord2f( 1, 0 );
        glVertex2f( texWidth, 0 );
        glTexCoord2f( 1, 1 );
        glVertex2f( texWidth, texHeight );
        glTexCoord2f( 0, 1 );
        glVertex2f( 0, texHeight );
    }
    glEnd();

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );

    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
}

// Draws a filled rectangle
void renderRect( const Rect& rect, const Color& color )
{
    // Get render target size for correct projection
    float rtW = 0, rtH = 0;
    getRenderTargetSize( rtW, rtH );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, rtW, rtH, 0, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glDisable( GL_TEXTURE_2D );
    glColor4fv( &color.r );

    glBegin( GL_QUADS );
    glVertex2f( rect.x, rect.y );
    glVertex2f( rect.x + rect.w, rect.y );
    glVertex2f( rect.x + rect.w, rect.y + rect.h );
    glVertex2f( rect.x, rect.y + rect.h );
    glEnd();

    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
}

void renderKeyboard()
{
    // Get render target size
    float rtW = 0, rtH = 0;
    getRenderTargetSize( rtW, rtH );

    // Get keyboard texture size
    float texWidth = 0, texHeight = 0;
    getTextureSize( g_pKeyboardTexture, texWidth, texHeight );

    // Position texture at the bottom center
    float x = ( rtW - texWidth ) * 0.5f;
    float y = rtH - texHeight;

    renderTexture( g_pKeyboardTexture, x, y );

    if ( g_pFont )
    {
        // Draw attribution text centered at the bottom of the screen
        const char* attribution = "By Rumudiez - Created in Adobe Illustrator, CC BY-SA 3.0, https://commons.wikimedia.org/w/index.php?curid=26015253";

        float textWidth = 0.0f, textHeight = 0.0f;
        measureText( g_pFont, attribution, textWidth, textHeight );
        float textX = ( rtW - textWidth ) * 0.5f;
        float textY = rtH + g_pFont->descent;

        drawText( g_pFont, attribution, textX, textY, BLACK );
    }

    // Draw rectangles over pressed keys, offset by bitmap position
    Keyboard::State keyboardState = Keyboard::get().getState();
    for ( const auto& [key, rect]: g_KeyRects )
    {
        if ( keyboardState.isKeyDown( key ) )
        {
            // Draw a rectangle over the pressed key.
            Rect highlightRect = {
                rect.x + x,
                rect.y + y,
                rect.w, rect.h
            };
            renderRect( highlightRect, RED );
        }
    }
}

void renderMouse()
{
    // Get current mouse position
    Mouse&       mouse = Mouse::get();
    Mouse::State state = mouse.getState();

    renderTextureRotated( g_pMouseTexture, g_MousePosition, g_fMouseRotation );
    if ( state.rightButton )
        renderTextureRotated( g_pRMBTexture, g_MousePosition, g_fMouseRotation );
    if ( state.leftButton )
        renderTextureRotated( g_pLMBTexture, g_MousePosition, g_fMouseRotation );
    if ( state.middleButton )
        renderTextureRotated( g_pMMBTexture, g_MousePosition, g_fMouseRotation );
    if ( g_MouseStateTracker.scrollWheelDelta > 0 )
        renderTextureRotated( g_pScrollUpTexture, g_MousePosition, g_fMouseRotation );
    if ( g_MouseStateTracker.scrollWheelDelta < 0 )
        renderTextureRotated( g_pScrollDownTexture, g_MousePosition, g_fMouseRotation );
}

void render()
{
    // Clear screen
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    renderKeyboard();
    renderMouse();

    glfwSwapBuffers( g_pWindow );
}

// Callback function for window resizing
void Window_SizeCallback( GLFWwindow* window, int width, int height )
{
    // Update OpenGL viewport to match new window size
    glViewport( 0, 0, width, height );
}

void Window_ContentScaleCallback( GLFWwindow* window, float xscale, float yscale )
{
    g_WindowXScale = xscale;
    g_WindowYScale = yscale;
}

int main()
{
    if ( glfwInit() != GLFW_TRUE )
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    // Enable 4x MSAA before window creation
    glfwWindowHint( GLFW_SAMPLES, 4 );

    g_pWindow = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Framework (GLFW)", nullptr, nullptr );
    if ( !g_pWindow )
    {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return -2;
    }

    glfwMakeContextCurrent( g_pWindow );

    int version = gladLoadGL( glfwGetProcAddress );
    if ( version == 0 )
    {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -3;
    }

    std::println( "Loaded OpenGL: {}.{}", GLAD_VERSION_MAJOR( version ), GLAD_VERSION_MINOR( version ) );

    // Enable MSAA in OpenGL
    glEnable( GL_MULTISAMPLE );

    // Enable alpha blending
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    g_pKeyboardTexture    = loadTexture( "assets/ANSI_Keyboard_Layout.png" );
    g_pMouseTexture       = loadTexture( "assets/Mouse.png" );
    g_pGamepadTexture     = loadTexture( "assets/XBox Controller.png" );
    g_pLMBTexture         = loadTexture( "assets/LMB.png" );
    g_pRMBTexture         = loadTexture( "assets/RMB.png" );
    g_pMMBTexture         = loadTexture( "assets/MMB.png" );
    g_pScrollUpTexture    = loadTexture( "assets/Scroll_Up.png" );
    g_pScrollDownTexture  = loadTexture( "assets/Scroll_Down.png" );
    g_pLeftBumperTexture  = loadTexture( "assets/Left_Bumper.png" );
    g_pRightBumperTexture = loadTexture( "assets/Right_Bumper.png" );

    g_pFont     = loadFont( "assets/Roboto/Regular.ttf", 22 );
    g_pFontMono = loadFont( "assets/RobotoMono/Regular.ttf", 22 );

    // Register window with input system
    Mouse::get().setWindow( g_pWindow );

    // Register mouse callback functions.
    glfwSetScrollCallback( g_pWindow, Mouse_ScrollCallback );
    glfwSetCursorPosCallback( g_pWindow, Mouse_CursorPosCallback );
    glfwSetMouseButtonCallback( g_pWindow, Mouse_ButtonCallback );
    glfwSetKeyCallback( g_pWindow, Keyboard_Callback );

    glfwSetFramebufferSizeCallback( g_pWindow, Window_SizeCallback );
    glfwSetWindowContentScaleCallback( g_pWindow, Window_ContentScaleCallback );

    while ( !glfwWindowShouldClose( g_pWindow ) )
    {
        glfwPollEvents();

        update();
        render();

        Mouse::get().resetRelativeMotion();
    }

    GLuint textures[] = {
        g_pKeyboardTexture,
        g_pMouseTexture,
        g_pGamepadTexture
    };

    glDeleteTextures( std::size( textures ), textures );

    deleteFont( g_pFont );
    deleteFont( g_pFontMono );

    glfwDestroyWindow( g_pWindow );
    glfwTerminate();

    return 0;
}