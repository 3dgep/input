#include <input/Input.hpp>

#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <string>

using namespace input;

// Window dimensions
constexpr int WINDOW_WIDTH  = 1920;
constexpr int WINDOW_HEIGHT = 1080;

SDL_Renderer* g_pRenderer = nullptr;
SDL_Window*   g_pWindow   = nullptr;

// Helper to load a texture from file
SDL_Texture* LoadTexture( SDL_Renderer* renderer, const std::string& path )
{
    int            width, height, channels;
    unsigned char* data = stbi_load( path.c_str(), &width, &height, &channels, 4 );  // force RGBA
    if ( !data )
    {
        std::cerr << "Failed to load image: " << path << " stb_image Error: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    SDL_Surface* surface = SDL_CreateSurface( width, height, SDL_PIXELFORMAT_RGBA32 );
    if ( !surface )
    {
        std::cerr << "Failed to create SDL_Surface: " << SDL_GetError() << std::endl;
        stbi_image_free( data );
        return nullptr;
    }

    std::memcpy( surface->pixels, data, width * height * 4 );
    stbi_image_free( data );

    SDL_Texture* texture = SDL_CreateTextureFromSurface( renderer, surface );
    SDL_DestroySurface( surface );
    return texture;
}

int main( int argc, char* argv[] )
{
    if ( !SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if ( !SDL_CreateWindowAndRenderer( "Simple DirectMedia Layer (SDL3)", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &g_pWindow, &g_pRenderer ) )
    {
        SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Failed to create window and renderer: %s", SDL_GetError() );
        throw std::runtime_error( SDL_GetError() );
    }

    // Associate the window with the mouse
    Mouse::get().setWindow( g_pWindow );

    // Load assets (example: keyboard, mouse, gamepad images)
    SDL_Texture* keyboardTexture = LoadTexture( g_pRenderer, "assets/ANSI_Keyboard_Layout.png" );
    SDL_Texture* mouseTexture    = LoadTexture( g_pRenderer, "assets/Mouse.png" );
    SDL_Texture* gamepadTexture  = LoadTexture( g_pRenderer, "assets/XBox Controller.png" );

    bool      running = true;
    SDL_Event event;
    while ( running )
    {
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_EVENT_QUIT )
                running = false;
            // Handle keyboard, mouse, and gamepad events here
        }

        // Example: Clear screen and draw loaded textures
        SDL_SetRenderDrawColor( g_pRenderer, 255, 255, 255, 255 );
        SDL_RenderClear( g_pRenderer );

        // Draw keyboard image at bottom center
        if ( keyboardTexture )
        {
            float texW = 0, texH = 0;
            SDL_GetTextureSize( keyboardTexture, &texW, &texH );
            SDL_FRect dst = { ( WINDOW_WIDTH - texW ) / 2.0f, WINDOW_HEIGHT - texH, texW, texH };
            SDL_RenderTexture( g_pRenderer, keyboardTexture, nullptr, &dst );
        }

        // Draw mouse image at center
        if ( mouseTexture )
        {
            Mouse::State mouseState = Mouse::get().getState();
            float        texW = 0, texH = 0;
            SDL_GetTextureSize( mouseTexture, &texW, &texH );
            // Center the texture at the mouse position
            SDL_FRect dst = { mouseState.x - texW / 2.0f, mouseState.y - texH / 2.0f, texW, texH };
            SDL_RenderTexture( g_pRenderer, mouseTexture, nullptr, &dst );
        }

        // Draw gamepad image at top left
        if ( gamepadTexture )
        {
            float texW = 0, texH = 0;
            SDL_GetTextureSize( gamepadTexture, &texW, &texH );
            SDL_FRect dst = { 32, 32, texW, texH };
            SDL_RenderTexture( g_pRenderer, gamepadTexture, nullptr, &dst );
        }

        // TODO: Draw state panels and highlight pressed keys/buttons using SDL3 primitives

        SDL_RenderPresent( g_pRenderer );
    }

    // Cleanup
    SDL_DestroyTexture( keyboardTexture );
    SDL_DestroyTexture( mouseTexture );
    SDL_DestroyTexture( gamepadTexture );
    SDL_DestroyRenderer( g_pRenderer );
    SDL_DestroyWindow( g_pWindow );
    SDL_Quit();

    return 0;
}