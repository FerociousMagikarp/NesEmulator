#include "sdl_application.h"
#include "SDL.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_scancode.h"
#include "virtual_device.h"
#include <cstdint>
#include <unordered_map>

SDLApplication::~SDLApplication()
{
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool SDLApplication::Init(int width, int height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return false;
    m_window = SDL_CreateWindow("NesEmulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, nes::NES_WIDTH, nes::NES_HEIGHT);
    return true;
}

void SDLApplication::SetVirtualDevice(std::shared_ptr<nes::VirtualDevice> device)
{
    device->SetApplicationUpdateCallback([this]()->void
    {
        SDL_UpdateTexture(m_texture, nullptr, m_device->GetScreenPointer(), nes::NES_WIDTH * 4);
    });
    m_device = std::move(device);
}

void SDLApplication::Run(bool &running)
{
    // 先这么写，之后再改
    std::uint16_t controller = 0;

    std::unordered_map<SDL_Keycode, int> keyboard_map;
    keyboard_map[SDLK_KP_PERIOD] = 0; keyboard_map[SDLK_KP_0] = 1; keyboard_map[SDLK_KP_PLUS] = 2;   keyboard_map[SDLK_KP_ENTER] = 3;
    keyboard_map[SDLK_UP] = 4;        keyboard_map[SDLK_DOWN] = 5; keyboard_map[SDLK_LEFT] = 6;      keyboard_map[SDLK_RIGHT] = 7;
    keyboard_map[SDLK_k] = 8;         keyboard_map[SDLK_j] = 9;    keyboard_map[SDLK_SEMICOLON] = 10; keyboard_map[SDLK_RETURN] = 11;
    keyboard_map[SDLK_w] = 12;        keyboard_map[SDLK_s] = 13;   keyboard_map[SDLK_a] = 14;        keyboard_map[SDLK_d] = 15;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            std::unordered_map<SDL_Keycode, int>::iterator key_iter;

            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                return;
            case SDL_KEYDOWN:
                key_iter = keyboard_map.find(event.key.keysym.sym);
                if (key_iter != keyboard_map.end())
                    controller |= (1 << key_iter->second);
                break;
            case SDL_KEYUP:
                key_iter = keyboard_map.find(event.key.keysym.sym);
                if (key_iter != keyboard_map.end())
                    controller &= ~(1 << key_iter->second);
                break;
            default:
                break;
            }
        }
        m_device->ApplicationSetControllers(static_cast<std::uint8_t>(controller >> 8), static_cast<std::uint8_t>(controller));
        m_device->ApplicationUpdate();
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }
}
