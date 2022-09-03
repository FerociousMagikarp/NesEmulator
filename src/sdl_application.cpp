#include "sdl_application.h"
#include "SDL_events.h"
#include "SDL_pixels.h"
#include "def.h"
#include "device.h"
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <iostream>

SDLApplication::SDLApplication()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "SDL Init Error : " << SDL_GetError() << std::endl;
    }
    m_window = SDL_CreateWindow("NesEmulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, nes::WIDTH * 2, nes::HEIGHT * 2, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, nes::WIDTH, nes::HEIGHT);
}

SDLApplication::~SDLApplication()
{
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void SDLApplication::Run(nes::Device* device, bool& is_running)
{
    while (is_running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                is_running = false;
                break;
            case SDL_KEYDOWN:
                break;
            default:
                break;
            }
        }
        SDL_UpdateTexture(m_texture, nullptr, device->GetScreenPointer(), nes::WIDTH * 4);
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(2);
    }
}
