#pragma once

#include "key_board_def.h"

namespace nes
{
    enum class MirroringType
    {
        Horizontal,
        Vertical,
        OneScreenLowerBank,
        OneScreenUpperBank,
    };

    enum class InputKey
    {
        A,
        B,
        Select,
        Start,
        Up,
        Down,
        Left,
        Right,
        TurboA,
        TurboB,
    };

    enum class Player
    {
        Player1,
        Player2,
    };

    constexpr int AUDIO_FREQ = 44100;
    constexpr int AUDIO_BUFFER_SAMPLES = 2048;
    constexpr int NTSC_CPU_FREQUENCY = 1789773;
    constexpr int NTSC_FRAME_FREQUENCY = 240;

    struct InputConfig
    {
        KeyCode A;
        KeyCode B;
        KeyCode Select;
        KeyCode Start;
        KeyCode Up;
        KeyCode Down;
        KeyCode Left;
        KeyCode Right;

        KeyCode TurboA;
        KeyCode TurboB;
    };

    struct Config
    {
        using enum KeyCode;
        InputConfig Player1 = {K, J, Semicolon, Return, W, S, A, D, I, U};
        InputConfig Player2 = {KPPeriod, KP0, KPPlus, KPEnter, Up, Down, Left, Right, KP2, KP1};
    };
}
