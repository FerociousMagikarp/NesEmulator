#pragma once

#include "virtual_device.h"
#include <memory>

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
        SDL_Window* m_window;
        SDL_Renderer* m_renderer;
        SDL_Texture* m_texture;

        std::shared_ptr<nes::VirtualDevice> m_device;
};
