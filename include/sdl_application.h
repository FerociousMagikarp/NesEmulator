#pragma once


struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class SDLApplication
{
public:
    SDLApplication();
    ~SDLApplication();

    void Run();
private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_texture = nullptr;
};
