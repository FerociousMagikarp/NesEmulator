#pragma once

#include "def.h"
#include "virtual_device.h"
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
        void FillAudioBuffer(unsigned char* stream, int len);

    private:
        SDL_Window* m_window;
        SDL_Renderer* m_renderer;
        SDL_Texture* m_texture;

        std::shared_ptr<nes::VirtualDevice> m_device;

        struct KeyInfo
        {
            nes::InputKey key;
            nes::Player player;
        };
        std::unordered_map<SDL_Keycode, KeyInfo> m_keyboard_map;
};
