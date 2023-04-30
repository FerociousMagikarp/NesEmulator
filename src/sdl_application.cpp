#include "sdl_application.h"
#include "SDL.h"
#include "def.h"
#include "virtual_device.h"
#include <chrono>
#include <cstdint>

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

void SDLApplication::KeyDown(nes::InputKey key, int player)
{
    if (key != nes::InputKey::TurboA && key != nes::InputKey::TurboB)
    {
        m_joy |= 1 << (static_cast<int>(key) + player * 8);
    }
    else
    {
        int key_index = static_cast<int>(key != nes::InputKey::TurboA);
        int turbo_index = key_index + player * 2;
        if (!m_turbo_keys[turbo_index].enable)
        {
            m_turbo_keys[turbo_index].enable = true;
            m_turbo_keys[turbo_index].time = m_current_time;
            m_joy |= 1 << (key_index + player * 8);
        }
    }
}

void SDLApplication::KeyUp(nes::InputKey key, int player)
{
    if (key != nes::InputKey::TurboA && key != nes::InputKey::TurboB)
    {
        m_joy &= ~(1 << (static_cast<int>(key) + player * 8));
    }
    else
    {
        int key_index = static_cast<int>(key != nes::InputKey::TurboA);
        m_joy &= ~(1 << (key_index + player * 8));
        m_turbo_keys[key_index + player * 2].enable = false;
    }
}

void SDLApplication::TurboTick()
{
    for (int i = 0; i < 4; i++)
    {
        if (!m_turbo_keys[i].enable)
            continue;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_current_time - m_turbo_keys[i].time).count();
        if (ms % m_turbo_time % 2 == 0)
            m_joy |= 1 << ((i & 0x01) | ((i & 0x02) << 2));
        else
            m_joy &= ~(1 << ((i & 0x01) | ((i & 0x02) << 2)));
    }
}

void SDLApplication::Run(bool &running)
{
    m_current_time = std::chrono::steady_clock::now();

    m_keyboard_map[SDLK_k] = {nes::InputKey::A, 0};               m_keyboard_map[SDLK_j] = {nes::InputKey::B, 0};
    m_keyboard_map[SDLK_SEMICOLON] = {nes::InputKey::Select, 0};  m_keyboard_map[SDLK_RETURN] = {nes::InputKey::Start, 0};
    m_keyboard_map[SDLK_w] = {nes::InputKey::Up, 0};              m_keyboard_map[SDLK_s] = {nes::InputKey::Down, 0};
    m_keyboard_map[SDLK_a] = {nes::InputKey::Left, 0};            m_keyboard_map[SDLK_d] = {nes::InputKey::Right, 0};
    m_keyboard_map[SDLK_i] = {nes::InputKey::TurboA, 0};          m_keyboard_map[SDLK_u] = {nes::InputKey::TurboB, 0};
    m_keyboard_map[SDLK_KP_PERIOD] = {nes::InputKey::A, 1};       m_keyboard_map[SDLK_KP_0] = {nes::InputKey::B, 1};
    m_keyboard_map[SDLK_KP_PLUS] = {nes::InputKey::Select, 1};    m_keyboard_map[SDLK_KP_ENTER] = {nes::InputKey::Start, 1};
    m_keyboard_map[SDLK_UP] = {nes::InputKey::Up, 1};             m_keyboard_map[SDLK_DOWN] = {nes::InputKey::Down, 1};
    m_keyboard_map[SDLK_LEFT] = {nes::InputKey::Left, 1};         m_keyboard_map[SDLK_RIGHT] = {nes::InputKey::Right, 1};
    m_keyboard_map[SDLK_KP_2] = {nes::InputKey::TurboA, 1};       m_keyboard_map[SDLK_KP_1] = {nes::InputKey::TurboB, 1};

    while (running)
    {
        SDL_Event event;
        m_current_time = std::chrono::steady_clock::now();
        while (SDL_PollEvent(&event))
        {
            std::unordered_map<SDL_Keycode, KeyInfo>::iterator key_iter;

            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                return;
            case SDL_KEYDOWN:
                key_iter = m_keyboard_map.find(event.key.keysym.sym);
                if (key_iter != m_keyboard_map.end())
                    KeyDown(key_iter->second.key, key_iter->second.player);
                break;
            case SDL_KEYUP:
                key_iter = m_keyboard_map.find(event.key.keysym.sym);
                if (key_iter != m_keyboard_map.end())
                    KeyUp(key_iter->second.key, key_iter->second.player);
                break;
            default:
                break;
            }
        }

        TurboTick();

        m_device->ApplicationSetControllers(static_cast<std::uint8_t>(m_joy & 0xff), static_cast<std::uint8_t>(m_joy >> 8));
        m_device->ApplicationUpdate();
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }
}
