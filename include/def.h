#pragma once

#include "key_board_def.h"

namespace nes
{
    constexpr int NES_WIDTH  = 256;
    constexpr int NES_HEIGHT = 240;

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

    // 存档文件用的魔法数 (其实这个数使用numpy随机生成的)
    constexpr int SAVE_MAGIC_NUMBER = 1098186332;
    constexpr int SAVE_VERSION = 0;

    enum class EmulatorOperation
    {
        None,
        Save,
        Load,
    };

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

    struct FuncConfig
    {
        KeyCode Save;
        KeyCode Load;
    };

    struct BaseConfig
    {
        int Scale = 3;
        int JoystickDeadZone = 8000;
    };

    struct Config
    {
        using enum KeyCode;
        InputConfig Player1 = {K, J, Semicolon, Return, W, S, A, D, I, U};
        InputConfig Player2 = {KPPeriod, KP0, KPPlus, KPEnter, Up, Down, Left, Right, KP2, KP1};
        FuncConfig  ShortcutKeys = { Comma, Period };
        BaseConfig  Base;
    };

    // 保存数据的时候用的，如果之后有大小端问题可以在这里处理
    template <typename T>
    inline char* UnsafeWrite(char* pointer, T&& val)
    {
        using type = std::decay_t<T>;
        auto p = reinterpret_cast<type*>(pointer);
        *p = val;
        p++;
        return reinterpret_cast<char*>(p);
    }

    // 读取数据的时候用的
    template <typename T>
    inline const char* UnsafeRead(const char* pointer, T& val)
    {
        using type = std::decay_t<T>;
        auto p = reinterpret_cast<const type*>(pointer);
        val = *p;
        p++;
        return reinterpret_cast<const char*>(p);
    }
}
