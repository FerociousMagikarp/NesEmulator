#pragma once

#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include <cstdint>
#include <memory>
#include <chrono>
#include "cartridge.h"
#include "virtual_device.h"

namespace nes
{
    class Cartridge;

    class NesEmulator
    {
    public:
        NesEmulator();
        ~NesEmulator();

        void PutInCartridge(std::unique_ptr<Cartridge> cartridge);
        void Run(const bool& running);

        inline void SetVirtualDevice(std::shared_ptr<VirtualDevice> device)
        { 
            m_device = device;
            m_PPU.SetDevice(device);
            m_APU.SetDevice(device);
        }

    private:
        std::uint8_t MainBusRead(std::uint16_t address);
        void MainBusWrite(std::uint16_t address, std::uint8_t value);

    private:
        std::unique_ptr<Cartridge> m_cartridge = nullptr;
        std::unique_ptr<std::uint8_t[]> m_RAM  = nullptr;

        std::shared_ptr<VirtualDevice> m_device = nullptr;

        CPU6502 m_CPU;
        PPU     m_PPU;
        APU     m_APU;
    };
}
