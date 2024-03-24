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

    // 音频相关部分。如果这部分初始化失败了就不播放声音。
    SDL_AudioSpec spec
    {
        .freq = nes::AUDIO_FREQ,
        .format = AUDIO_U8,
        .channels = 1,
        .silence = 0,
        .samples = nes::AUDIO_BUFFER_SAMPLES,
        .userdata = this,
    };
    spec.callback = [](void* userdata, Uint8* stream, int len)->void
    {
        static_cast<SDLApplication*>(userdata)->FillAudioBuffer(stream, len);
    };

    if (SDL_OpenAudio(&spec, nullptr) == 0)
    {
        SDL_PauseAudio(0);
    }

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
    m_keyboard_map[SDLK_k] = {nes::InputKey::A, nes::Player::Player1};               m_keyboard_map[SDLK_j] = {nes::InputKey::B, nes::Player::Player1};
    m_keyboard_map[SDLK_SEMICOLON] = {nes::InputKey::Select, nes::Player::Player1};  m_keyboard_map[SDLK_RETURN] = {nes::InputKey::Start, nes::Player::Player1};
    m_keyboard_map[SDLK_w] = {nes::InputKey::Up, nes::Player::Player1};              m_keyboard_map[SDLK_s] = {nes::InputKey::Down, nes::Player::Player1};
    m_keyboard_map[SDLK_a] = {nes::InputKey::Left, nes::Player::Player1};            m_keyboard_map[SDLK_d] = {nes::InputKey::Right, nes::Player::Player1};
    m_keyboard_map[SDLK_i] = {nes::InputKey::TurboA, nes::Player::Player1};          m_keyboard_map[SDLK_u] = {nes::InputKey::TurboB, nes::Player::Player1};
    m_keyboard_map[SDLK_KP_PERIOD] = {nes::InputKey::A, nes::Player::Player2};       m_keyboard_map[SDLK_KP_0] = {nes::InputKey::B, nes::Player::Player2};
    m_keyboard_map[SDLK_KP_PLUS] = {nes::InputKey::Select, nes::Player::Player2};    m_keyboard_map[SDLK_KP_ENTER] = {nes::InputKey::Start, nes::Player::Player2};
    m_keyboard_map[SDLK_UP] = {nes::InputKey::Up, nes::Player::Player2};             m_keyboard_map[SDLK_DOWN] = {nes::InputKey::Down, nes::Player::Player2};
    m_keyboard_map[SDLK_LEFT] = {nes::InputKey::Left, nes::Player::Player2};         m_keyboard_map[SDLK_RIGHT] = {nes::InputKey::Right, nes::Player::Player2};
    m_keyboard_map[SDLK_KP_2] = {nes::InputKey::TurboA, nes::Player::Player2};       m_keyboard_map[SDLK_KP_1] = {nes::InputKey::TurboB, nes::Player::Player2};

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                return;
            case SDL_KEYDOWN:
                if (auto iter = m_keyboard_map.find(event.key.keysym.sym); iter != m_keyboard_map.end())
                    m_device->ApplicationKeyDown(iter->second.player, iter->second.key);
                break;
            case SDL_KEYUP:
                if (auto iter = m_keyboard_map.find(event.key.keysym.sym); iter != m_keyboard_map.end())
                    m_device->ApplicationKeyUp(iter->second.player, iter->second.key);
                break;
            default:
                break;
            }
        }

        m_device->ApplicationUpdate();
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }
    SDL_CloseAudio();
}

void SDLApplication::FillAudioBuffer(unsigned char* stream, int len)
{
    if (!m_device)
        return;
    m_device->FillAudioSamples(stream, len);
}
