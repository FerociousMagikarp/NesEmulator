#pragma once

#include "def.h"
#include <memory>
#include <functional>
#include <variant>
#include <unordered_map>
#include "SDL_keycode.h"
#include "SDL_joystick.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace nes
{
    class VirtualDevice;
}

class SDLApplication
{
    public:
        SDLApplication() = default;
        ~SDLApplication();

        bool Init(int width, int height);
        void Run(bool& running);
        void Terminate();

        void SetVirtualDevice(std::shared_ptr<nes::VirtualDevice> device);
        void FillAudioBuffer(unsigned char* stream, int len);

        void SetControl(nes::KeyCode key, nes::InputKey input, nes::Player player);
        void SetControl(nes::KeyCode key, std::function<void()> func);

    private:
        void SDLJoystickHat(SDL_JoystickID id, int hat_value);
        void SDLJoystickButtonDown(SDL_JoystickID id, int button);
        void SDLJoystickButtonUp(SDL_JoystickID id, int button);
        void SDLJoystickAxis(SDL_JoystickID id, int axis, int value);
        nes::Player GetPlayerByJoyID(SDL_JoystickID id);

        SDL_Window* m_window;
        SDL_Renderer* m_renderer;
        SDL_Texture* m_texture;
        // 现在就支持两个手柄就可以了
        struct Joystick
        {
            SDL_Joystick* joy = nullptr;
            SDL_JoystickID id = 0;
            nes::Player player;
        };
        std::array<Joystick, 2> m_joysticks{};
        int m_last_hat_value = 0;

        std::shared_ptr<nes::VirtualDevice> m_device;

        struct KeyInfo
        {
            nes::InputKey key;
            nes::Player player;
        };
        std::unordered_map<SDL_Keycode, std::variant<KeyInfo, std::function<void()>>> m_keyboard_map;
};
