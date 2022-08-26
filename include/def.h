#pragma once

namespace nes
{
    using uint8 = unsigned char;
    using uint16 = unsigned short;
    using uint32 = unsigned int;

    using byte = unsigned char;

    enum class CPU6502InterruptType
    {
        IRQ,
        BRK,
        NMI
    };
}
