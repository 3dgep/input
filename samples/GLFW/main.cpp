#include <input/Input.hpp>

#define NOMINMAX
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../out/build/vs17/_deps/harfbuzz-src/src/OT/Layout/GSUB/AlternateSet.hh"
#include "stb_truetype.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
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

constexpr int WINDOW_WIDTH  = 1920;
constexpr int WINDOW_HEIGHT = 1080;

GLFWwindow* g_pWindow          = nullptr;
GLuint      g_pKeyboardTexture = 0;
GLuint      g_pMouseTexture    = 0;
GLuint      g_pGamepadTexture  = 0;
Font*       g_pFont            = nullptr;
Font*       g_pFontMono        = nullptr;

float g_WindowXScale = 1.0f;
float g_WindowYScale = 1.0f;

// Forward-declare callback functions.
void Mouse_ScrollCallback( GLFWwindow*, double, double );
void Mouse_CursorPosCallback( GLFWwindow*, double, double );
void Mouse_ButtonCallback( GLFWwindow*, int, int, int );

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
    float scale = stbtt_ScaleForPixelHeight( &font->fontInfo, pixelHeight );
    font->ascent = ascent * scale;
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

void update()
{
    Mouse&       mouse      = Mouse::get();
    Mouse::State mouseState = mouse.getState();

    static MouseStateTracker mouseStateTracker;
    mouseStateTracker.update( mouseState );

    if ( mouseStateTracker.rightButton == MouseStateTracker::ButtonState::Released )
    {
        if ( mouseState.positionMode == Mouse::Mode::Absolute )
            mouse.setMode( Mouse::Mode::Relative );
        else
            mouse.setMode( Mouse::Mode::Absolute );
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

void getRenderTargetSize( float& w, float& h )
{
    int rtW = 0, rtH = 0;
    glfwGetWindowSize( g_pWindow, &rtW, &rtH );

    w = rtW / g_WindowXScale;
    h = rtH / g_WindowYScale;
}

// Draw text at (x, y) in screen coordinates using a Font
void drawText( const Font* font, const char* text, float x, float y, const float color[4] = nullptr )
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

    if ( color )
        glColor4fv( color );
    else
        glColor4f( 0, 0, 0, 1 );  // Default: black

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
        const char* attribution  = "By Rumudiez - Created in Adobe Illustrator, CC BY-SA 3.0, https://commons.wikimedia.org/w/index.php?curid=26015253";
        float       textColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // Black

        float textWidth = 0.0f, textHeight = 0.0f;
        measureText( g_pFont, attribution, textWidth, textHeight );
        float textX = ( rtW - textWidth ) * 0.5f;
        float textY = rtH + g_pFont->descent;

        drawText( g_pFont, attribution, textX, textY, textColor );
    }
}

void renderMouse()
{
    // Get current mouse position
    Mouse&       mouse      = Mouse::get();
    Mouse::State mouseState = mouse.getState();

    // Get mouse texture size
    float texWidth = 0, texHeight = 0;
    getTextureSize( g_pMouseTexture, texWidth, texHeight );

    // Center texture at mouse position
    float x = mouseState.x / g_WindowXScale - texWidth * 0.5f;
    float y = mouseState.y / g_WindowYScale - texHeight * 0.5f;

    renderTexture( g_pMouseTexture, x, y );
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

    g_pKeyboardTexture = loadTexture( "assets/ANSI_Keyboard_Layout.png" );
    g_pMouseTexture    = loadTexture( "assets/Mouse.png" );
    g_pGamepadTexture  = loadTexture( "assets/XBox Controller.png" );

    g_pFont     = loadFont( "assets/Roboto/Regular.ttf", 22 );
    g_pFontMono = loadFont( "assets/RobotoMono/Regular.ttf", 22 );

    // Register window with input system
    Mouse::get().setWindow( g_pWindow );

    // Register mouse callback functions.
    glfwSetScrollCallback( g_pWindow, Mouse_ScrollCallback );
    glfwSetCursorPosCallback( g_pWindow, Mouse_CursorPosCallback );
    glfwSetMouseButtonCallback( g_pWindow, Mouse_ButtonCallback );

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