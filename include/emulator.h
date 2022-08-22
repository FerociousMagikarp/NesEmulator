#pragma once

#include "cpu.h"

namespace nes
{
    class Cartridge;

    class Emulator
    {
    public:
        Emulator() noexcept;
        ~Emulator();

        void PutInCartridge(Cartridge* cartridge);

    private:
        Cartridge* m_cartridge = nullptr;

        CPU6502 m_CPU;
    };
}
