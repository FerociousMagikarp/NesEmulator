#pragma once

#include "cpu.h"
#include "ppu.h"
#include <cstdint>
#include <memory>

namespace nes
{
    class Cartridge;

    class NesEmulator
    {
    public:
        NesEmulator();
        ~NesEmulator();

        void PutInCartridge(std::unique_ptr<Cartridge> cartridge);
        void Run();

    private:
        std::uint8_t MainBusRead(std::uint16_t address);
        void MainBusWrite(std::uint16_t address, std::uint8_t value);

    private:
        std::unique_ptr<Cartridge> m_cartridge = nullptr;
        std::unique_ptr<std::uint8_t[]> m_RAM  = nullptr;

        CPU6502 m_CPU;
        PPU     m_PPU;
    };
}
