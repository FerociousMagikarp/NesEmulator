#pragma once

#include "def.h"
#include "virtual_device.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "SDL_keycode.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class SDLApplication
{
    public:
        SDLApplication() = default;
        ~SDLApplication();

        bool Init(int width, int height);
        void Run(bool& running);

        void SetVirtualDevice(std::shared_ptr<nes::VirtualDevice> device);

    private:

        void KeyDown(nes::InputKey key, int player);
        void KeyUp(nes::InputKey key, int player);
        void TurboTick();

        SDL_Window* m_window;
        SDL_Renderer* m_renderer;
        SDL_Texture* m_texture;

        std::shared_ptr<nes::VirtualDevice> m_device;

        struct KeyInfo
        {
            nes::InputKey key;
            int player;
        };
        std::unordered_map<SDL_Keycode, KeyInfo> m_keyboard_map;
        std::uint32_t m_joy = 0;
        std::uint32_t m_turbo_time = 30;

        struct TurboKey
        {
            nes::InputKey key;
            int player;
            bool enable = false;
            std::chrono::system_clock::time_point time;

            TurboKey(nes::InputKey key, int player) : key(key), player(player) {}
        };

        TurboKey m_turbo_keys[4] = 
        { 
            TurboKey(nes::InputKey::TurboA, 0), TurboKey(nes::InputKey::TurboB, 0), 
            TurboKey(nes::InputKey::TurboA, 1), TurboKey(nes::InputKey::TurboB, 1)
        };

        std::chrono::system_clock::time_point m_current_time;
};
