#include <input/Input.hpp>

#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <print>

using namespace input;

constexpr int WINDOW_WIDTH  = 1920;
constexpr int WINDOW_HEIGHT = 1080;

GLFWwindow* g_pWindow          = nullptr;
GLuint      g_pKeyboardTexture = 0;
GLuint      g_pMouseTexture    = 0;
GLuint      g_pGamepadTexture  = 0;

// Forward-declare callback functions.
void Mouse_ScrollCallback( GLFWwindow*, double, double );
void Mouse_CursorPosCallback( GLFWwindow*, double, double );
void Mouse_ButtonCallback( GLFWwindow*, int, int, int );

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

    // Gamepad vibration test
    for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
    {
        Gamepad        gamepad { i };
        Gamepad::State gamepadState = gamepad.getState();
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

            gamepad.setVibration( leftMotor, rightMotor, leftTrigger, rightTrigger );
        }
    }
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
    int rtW = 0, rtH = 0;
    glfwGetWindowSize( g_pWindow, &rtW, &rtH );

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
    glTexCoord2f( 0, 0 );
    glVertex2f( x, y );
    glTexCoord2f( 1, 0 );
    glVertex2f( x + texWidth, y );
    glTexCoord2f( 1, 1 );
    glVertex2f( x + texWidth, y + texHeight );
    glTexCoord2f( 0, 1 );
    glVertex2f( x, y + texHeight );
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
    int rtW = 0, rtH = 0;
    glfwGetWindowSize( g_pWindow, &rtW, &rtH );

    // Get keyboard texture size
    float texWidth = 0, texHeight = 0;
    getTextureSize( g_pKeyboardTexture, texWidth, texHeight );

    // Position texture at the bottom center
    float x = ( rtW - texWidth ) * 0.5f;
    float y = rtH - texHeight;

    renderTexture( g_pKeyboardTexture, x, y );
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
    float x = mouseState.x - texWidth * 0.5f;
    float y = mouseState.y - texHeight * 0.5f;

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

    // Register window with input system
    Mouse::get().setWindow( g_pWindow );

    // Register mouse callback functions.
    glfwSetScrollCallback( g_pWindow, Mouse_ScrollCallback );
    glfwSetCursorPosCallback( g_pWindow, Mouse_CursorPosCallback );
    glfwSetMouseButtonCallback( g_pWindow, Mouse_ButtonCallback );

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

    glfwDestroyWindow( g_pWindow );
    glfwTerminate();

    return 0;
}