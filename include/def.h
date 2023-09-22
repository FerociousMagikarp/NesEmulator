#pragma once

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

    constexpr int AUDIO_FREQ = 44100;
    constexpr int AUDIO_BUFFER_SAMPLES = 2048;
    constexpr int NTSC_CPU_FREQUENCY = 1789773;
    constexpr int NTSC_FRAME_FREQUENCY = 240;
}
