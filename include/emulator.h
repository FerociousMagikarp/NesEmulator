#pragma once

#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include <memory>
#include <atomic>
#include <concepts>
#include <functional>
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

        template <typename F>
            requires requires(F f) { {f()} -> std::same_as<void>; }
        void SetScreenshotCallback(F&& f) { m_screenshot_callback = std::forward<F>(f); }

        void SetOperation(EmulatorOperation operation);
        const std::string& GetCartridgeFilename() const noexcept { return m_cartridge->GetFileName(); }

    private:
        std::uint8_t MainBusRead(std::uint16_t address);
        void MainBusWrite(std::uint16_t address, std::uint8_t value);

        std::string GetSavePath() const;

        void Save();
        void Load();

    private:
        std::unique_ptr<Cartridge> m_cartridge = nullptr;
        std::unique_ptr<std::uint8_t[]> m_RAM  = nullptr;

        std::shared_ptr<VirtualDevice> m_device = nullptr;

        std::atomic<EmulatorOperation> m_operation = EmulatorOperation::None;
        std::uint64_t m_frame = 0;
        std::function<void(void)> m_screenshot_callback;

        CPU6502 m_CPU;
        PPU     m_PPU;
        APU     m_APU;
    };
}
