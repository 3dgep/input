#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <string>

// Window dimensions
constexpr int WINDOW_WIDTH  = 1920;
constexpr int WINDOW_HEIGHT = 1080;

// Helper to load a texture from file
SDL_Texture* LoadTexture(SDL_Renderer* renderer, const std::string& path)
{
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4); // force RGBA
    if (!data)
    {
        std::cerr << "Failed to load image: " << path << " stb_image Error: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
    {
        std::cerr << "Failed to create SDL_Surface: " << SDL_GetError() << std::endl;
        stbi_image_free(data);
        return nullptr;
    }

    std::memcpy(surface->pixels, data, width * height * 4);
    stbi_image_free(data);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main( int argc, char* argv[] )
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER ) != 0 )
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Simple DirectMedia Layer (SDL2)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN );
    if ( !window )
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if ( !renderer )
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow( window );
        SDL_Quit();
        return -1;
    }

    // Load assets (example: keyboard, mouse, gamepad images)
    SDL_Texture* keyboardTexture = LoadTexture( renderer, "assets/ANSI_Keyboard_Layout.png" );
    SDL_Texture* mouseTexture    = LoadTexture( renderer, "assets/Mouse.png" );
    SDL_Texture* gamepadTexture  = LoadTexture( renderer, "assets/XBox Controller.png" );

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

        // Example: Clear screen and draw loaded textures
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderClear( renderer );

        // Draw keyboard image at bottom center
        if ( keyboardTexture )
        {
            int texW = 0, texH = 0;
            SDL_QueryTexture( keyboardTexture, nullptr, nullptr, &texW, &texH );
            SDL_Rect dst = { ( WINDOW_WIDTH - texW ) / 2, WINDOW_HEIGHT - texH, texW, texH };
            SDL_RenderCopy( renderer, keyboardTexture, nullptr, &dst );
        }

        // Draw mouse image at center
        if ( mouseTexture )
        {
            int texW = 0, texH = 0;
            SDL_QueryTexture( mouseTexture, nullptr, nullptr, &texW, &texH );
            SDL_Rect dst = { ( WINDOW_WIDTH - texW ) / 2, ( WINDOW_HEIGHT - texH ) / 2, texW, texH };
            SDL_RenderCopy( renderer, mouseTexture, nullptr, &dst );
        }

        // Draw gamepad image at top left
        if ( gamepadTexture )
        {
            int texW = 0, texH = 0;
            SDL_QueryTexture( gamepadTexture, nullptr, nullptr, &texW, &texH );
            SDL_Rect dst = { 32, 32, texW, texH };
            SDL_RenderCopy( renderer, gamepadTexture, nullptr, &dst );
        }

        // TODO: Draw state panels and highlight pressed keys/buttons using SDL2 primitives

        SDL_RenderPresent( renderer );
    }

    // Cleanup
    SDL_DestroyTexture( keyboardTexture );
    SDL_DestroyTexture( mouseTexture );
    SDL_DestroyTexture( gamepadTexture );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
    return 0;
}