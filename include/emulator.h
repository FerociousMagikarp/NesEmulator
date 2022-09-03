#pragma once

#include "cpu.h"
#include "device.h"
#include "ppu.h"
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
        void Run(Device* device, bool& is_running);

    private:
        byte MainBusRead(uint16 address);
        void MainBusWrite(uint16 address, byte value);
        byte PPUBusRead(uint16 address);
        void PPUBusWrite(uint16 address, byte value);

    private:
        Cartridge* m_cartridge = nullptr;
        std::unique_ptr<byte[]> m_RAM  = nullptr;
        std::unique_ptr<byte[]> m_VRAM = nullptr;

        CPU6502 m_CPU;
        PPU     m_PPU;
    };
}
