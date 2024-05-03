#include "sdl_application.h"
#include "SDL.h"
#include "def.h"
#include "virtual_device.h"
#include <chrono>
#include <cstdint>

const std::unordered_map<nes::KeyCode, SDL_KeyCode> KEY_CODE_MAP = 
{
    {nes::KeyCode::Backspace,    SDLK_BACKSPACE   },
    {nes::KeyCode::Tab,          SDLK_TAB         },
    {nes::KeyCode::Clear,        SDLK_CLEAR       },
    {nes::KeyCode::Return,       SDLK_RETURN      },
    {nes::KeyCode::Pause,        SDLK_PAUSE       },
    {nes::KeyCode::Escape,       SDLK_ESCAPE      },
    {nes::KeyCode::Space,        SDLK_SPACE       },
    {nes::KeyCode::Exclaim,      SDLK_EXCLAIM     },
    {nes::KeyCode::Quotedbl,     SDLK_QUOTEDBL    },
    {nes::KeyCode::Hash,         SDLK_HASH        },
    {nes::KeyCode::Dollar,       SDLK_DOLLAR      },
    {nes::KeyCode::Ampersand,    SDLK_AMPERSAND   },
    {nes::KeyCode::Quote,        SDLK_QUOTE       },
    {nes::KeyCode::Leftparen,    SDLK_LEFTPAREN   },
    {nes::KeyCode::Rightparen,   SDLK_RIGHTPAREN  },
    {nes::KeyCode::Asterisk,     SDLK_ASTERISK    },
    {nes::KeyCode::Plus,         SDLK_PLUS        },
    {nes::KeyCode::Comma,        SDLK_COMMA       },
    {nes::KeyCode::Minus,        SDLK_MINUS       },
    {nes::KeyCode::Period,       SDLK_PERIOD      },
    {nes::KeyCode::Slash,        SDLK_SLASH       },
    {nes::KeyCode::Key0,         SDLK_0           },
    {nes::KeyCode::Key1,         SDLK_1           },
    {nes::KeyCode::Key2,         SDLK_2           },
    {nes::KeyCode::Key3,         SDLK_3           },
    {nes::KeyCode::Key4,         SDLK_4           },
    {nes::KeyCode::Key5,         SDLK_5           },
    {nes::KeyCode::Key6,         SDLK_6           },
    {nes::KeyCode::Key7,         SDLK_7           },
    {nes::KeyCode::Key8,         SDLK_8           },
    {nes::KeyCode::Key9,         SDLK_9           },
    {nes::KeyCode::Colon,        SDLK_COLON       },
    {nes::KeyCode::Semicolon,    SDLK_SEMICOLON   },
    {nes::KeyCode::Less,         SDLK_LESS        },
    {nes::KeyCode::Equals,       SDLK_EQUALS      },
    {nes::KeyCode::Greater,      SDLK_GREATER     },
    {nes::KeyCode::Question,     SDLK_QUESTION    },
    {nes::KeyCode::At,           SDLK_AT          },
    {nes::KeyCode::Leftbracket,  SDLK_LEFTBRACKET },
    {nes::KeyCode::Backslash,    SDLK_BACKSLASH   },
    {nes::KeyCode::Rightbracket, SDLK_RIGHTBRACKET},
    {nes::KeyCode::Caret,        SDLK_CARET       },
    {nes::KeyCode::Underscore,   SDLK_UNDERSCORE  },
    {nes::KeyCode::Backquote,    SDLK_BACKQUOTE   },
    {nes::KeyCode::A,            SDLK_a           },
    {nes::KeyCode::B,            SDLK_b           },
    {nes::KeyCode::C,            SDLK_c           },
    {nes::KeyCode::D,            SDLK_d           },
    {nes::KeyCode::E,            SDLK_e           },
    {nes::KeyCode::F,            SDLK_f           },
    {nes::KeyCode::G,            SDLK_g           },
    {nes::KeyCode::H,            SDLK_h           },
    {nes::KeyCode::I,            SDLK_i           },
    {nes::KeyCode::J,            SDLK_j           },
    {nes::KeyCode::K,            SDLK_k           },
    {nes::KeyCode::L,            SDLK_l           },
    {nes::KeyCode::M,            SDLK_m           },
    {nes::KeyCode::N,            SDLK_n           },
    {nes::KeyCode::O,            SDLK_o           },
    {nes::KeyCode::P,            SDLK_p           },
    {nes::KeyCode::Q,            SDLK_q           },
    {nes::KeyCode::R,            SDLK_r           },
    {nes::KeyCode::S,            SDLK_s           },
    {nes::KeyCode::T,            SDLK_t           },
    {nes::KeyCode::U,            SDLK_u           },
    {nes::KeyCode::V,            SDLK_v           },
    {nes::KeyCode::W,            SDLK_w           },
    {nes::KeyCode::X,            SDLK_x           },
    {nes::KeyCode::Y,            SDLK_y           },
    {nes::KeyCode::Z,            SDLK_z           },
    {nes::KeyCode::Delete,       SDLK_DELETE      },
    {nes::KeyCode::KP0,          SDLK_KP_0        },
    {nes::KeyCode::KP1,          SDLK_KP_1        },
    {nes::KeyCode::KP2,          SDLK_KP_2        },
    {nes::KeyCode::KP3,          SDLK_KP_3        },
    {nes::KeyCode::KP4,          SDLK_KP_4        },
    {nes::KeyCode::KP5,          SDLK_KP_5        },
    {nes::KeyCode::KP6,          SDLK_KP_6        },
    {nes::KeyCode::KP7,          SDLK_KP_7        },
    {nes::KeyCode::KP8,          SDLK_KP_8        },
    {nes::KeyCode::KP9,          SDLK_KP_9        },
    {nes::KeyCode::KPPeriod,     SDLK_KP_PERIOD   },
    {nes::KeyCode::KPDivide,     SDLK_KP_DIVIDE   },
    {nes::KeyCode::KPMultiply,   SDLK_KP_MULTIPLY },
    {nes::KeyCode::KPMinus,      SDLK_KP_MINUS    },
    {nes::KeyCode::KPPlus,       SDLK_KP_PLUS     },
    {nes::KeyCode::KPEnter,      SDLK_KP_ENTER    },
    {nes::KeyCode::KPEquals,     SDLK_KP_EQUALS   },
    {nes::KeyCode::Up,           SDLK_UP          },
    {nes::KeyCode::Down,         SDLK_DOWN        },
    {nes::KeyCode::Right,        SDLK_RIGHT       },
    {nes::KeyCode::Left,         SDLK_LEFT        },
    {nes::KeyCode::Insert,       SDLK_INSERT      },
    {nes::KeyCode::Home,         SDLK_HOME        },
    {nes::KeyCode::End,          SDLK_END         },
    {nes::KeyCode::Pageup,       SDLK_PAGEUP      },
    {nes::KeyCode::Pagedown,     SDLK_PAGEDOWN    },
    {nes::KeyCode::F1,           SDLK_F1          },
    {nes::KeyCode::F2,           SDLK_F2          },
    {nes::KeyCode::F3,           SDLK_F3          },
    {nes::KeyCode::F4,           SDLK_F4          },
    {nes::KeyCode::F5,           SDLK_F5          },
    {nes::KeyCode::F6,           SDLK_F6          },
    {nes::KeyCode::F7,           SDLK_F7          },
    {nes::KeyCode::F8,           SDLK_F8          },
    {nes::KeyCode::F9,           SDLK_F9          },
    {nes::KeyCode::F10,          SDLK_F10         },
    {nes::KeyCode::F11,          SDLK_F11         },
    {nes::KeyCode::F12,          SDLK_F12         },
    {nes::KeyCode::F13,          SDLK_F13         },
    {nes::KeyCode::F14,          SDLK_F14         },
    {nes::KeyCode::F15,          SDLK_F15         },
    {nes::KeyCode::Numlock,      SDLK_NUMLOCKCLEAR},
    {nes::KeyCode::Capslock,     SDLK_CAPSLOCK    },
    {nes::KeyCode::Scrolllock,   SDLK_SCROLLLOCK  },
    {nes::KeyCode::Rshift,       SDLK_RSHIFT      },
    {nes::KeyCode::Lshift,       SDLK_LSHIFT      },
    {nes::KeyCode::Rctrl,        SDLK_RCTRL       },
    {nes::KeyCode::Lctrl,        SDLK_LCTRL       },
    {nes::KeyCode::Ralt,         SDLK_RALT        },
    {nes::KeyCode::Lalt,         SDLK_LALT        },
    {nes::KeyCode::Mode,         SDLK_MODE        },
    {nes::KeyCode::Help,         SDLK_HELP        },
    {nes::KeyCode::Print,        SDLK_PRINTSCREEN },
    {nes::KeyCode::Sysreq,       SDLK_SYSREQ      },
    {nes::KeyCode::Menu,         SDLK_MENU        },
    {nes::KeyCode::Power,        SDLK_POWER       },
};

SDLApplication::~SDLApplication()
{
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool SDLApplication::Init(int width, int height)
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
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

void SDLApplication::SetControl(nes::KeyCode key, nes::InputKey input, nes::Player player)
{
    if (key == nes::KeyCode::Unknown)
        return;
    // 这里一定能找到
    auto sdl_key = KEY_CODE_MAP.find(key)->second;
    m_keyboard_map[sdl_key] = KeyInfo{input, player};
}

void SDLApplication::SetControl(nes::KeyCode key, std::function<void()> func)
{
    if (key == nes::KeyCode::Unknown)
        return;
    // 这里一定能找到
    auto sdl_key = KEY_CODE_MAP.find(key)->second;
    m_keyboard_map[sdl_key] = std::move(func);
}

void SDLApplication::Run(bool &running)
{
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if (auto iter = m_keyboard_map.find(event.key.keysym.sym); iter != m_keyboard_map.end())
                {
                    std::visit([this](auto&& val) -> void
                    {
                        using type = std::decay_t<decltype(val)>;
                        if constexpr (std::is_same_v<type, KeyInfo>)
                            m_device->ApplicationKeyDown(val.player, val.key);
                        else
                            val();
                    }, iter->second);
                }
                break;
            case SDL_KEYUP:
                if (auto iter = m_keyboard_map.find(event.key.keysym.sym); iter != m_keyboard_map.end())
                {
                    if (std::holds_alternative<KeyInfo>(iter->second))
                    {
                        decltype(auto) info = std::get<KeyInfo>(iter->second);
                        m_device->ApplicationKeyUp(info.player, info.key);
                    }
                }
                break;
            default:
                break;
            }
        }

        if (running == false)
            break;

        m_device->ApplicationUpdate();
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }
    SDL_CloseAudio();
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyTexture(m_texture);
    SDL_DestroyWindow(m_window);
}

void SDLApplication::FillAudioBuffer(unsigned char* stream, int len)
{
    if (!m_device)
        return;
    m_device->FillAudioSamples(stream, len);
}

