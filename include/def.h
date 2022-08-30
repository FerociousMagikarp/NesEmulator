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

    enum class CPU6502AddressingType
    {
        Implied,
        Immediate,
        Accumulator,
        Absolute,
        AbsoluteX,
        AbsoluteY,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        Indirect,
        IndirectX,
        IndirectY,
        Relative,
        XXX = -1
    };
}
