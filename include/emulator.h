#pragma once

#include "cpu.h"
#include <memory>

namespace nes
{
    class Cartridge;

    class Emulator
    {
    public:
        Emulator();
        ~Emulator();

        void PutInCartridge(Cartridge* cartridge);
        void Run();

    private:
        byte MainBusRead(uint16 address);
        void MainBusWrite(uint16 address, byte value);

    private:
        Cartridge* m_cartridge = nullptr;
        std::unique_ptr<byte[]> m_RAM = nullptr;

        CPU6502 m_CPU;
    };
}
