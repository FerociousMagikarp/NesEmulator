#pragma once

#include "virtual_device.h"
#include <cstdint>
#include <functional>
#include <memory>

namespace nes
{
    class Cartridge;
    class Device;

    class PPU
    {
    public:
        PPU();
        ~PPU();

        void Reset();
        void Step();

        std::uint8_t GetRegister(std::uint16_t address);
        void SetRegister(std::uint16_t address, std::uint8_t value);

        inline void SetReadMapperCHRCallback(std::function<std::uint8_t(std::uint16_t)>&& callback) { m_mapper_read_CHR = std::move(callback); }
        inline void SetWriteMapperCHRCallback(std::function<void(std::uint16_t, std::uint8_t)>&& callback) { m_mapper_write_CHR = std::move(callback); }
        inline void SetNMICallback(std::function<void()>&& callback) { m_trigger_NMI = std::move(callback); }

        inline void SetDevice(std::shared_ptr<VirtualDevice> device) { m_device = std::move(device); }

    private:
        std::uint8_t PPUBusRead(std::uint16_t address);
        void PPUBusWrite(std::uint16_t address, std::uint8_t value);

        void StepPreRenderScanline();
        void StepVisibleScanlines();
        void StepPostRenderScanline();
        void StepVerticalBlankingLines();

        // PPUCTRL
        // TODO : 末两位还影响scroll？ 还有SpriteSize没写，Master/Slave Mode也没写
        inline int GetNametableAddress() const { return (m_PPUCTRL & 0x03); }
        inline std::uint16_t GetAddressIncrement() const { return !(m_PPUCTRL & 0x04) ? 1 : 32; }
        inline std::uint16_t GetSpritePatternTableAddress() const { return !(m_PPUCTRL & 0x08) ? 0x0000 : 0x1000; }
        inline std::uint16_t GetBackgroundPatternTableAddress() const { return !(m_PPUCTRL & 0x10) ? 0x0000 : 0x1000; }
        inline bool IsNMIEnabled() const { return static_cast<bool>(m_PPUCTRL & 0x80); }
        void SetPPUCTRL(std::uint8_t value);

        // PPUMASK
        inline bool IsShowBackgroundEnabled() const { return static_cast<bool>(m_PPUMASK & 0x08); }
        inline bool IsShowSpriteEnabled() const { return static_cast<bool>(m_PPUMASK & 0x10); }
        inline bool IsRenderingEnabled() const { return (m_PPUMASK & 0x18) != 0; }

        // PPUSTATUS
        std::uint8_t GetPPUSTATUS();

        // OAMADDR
        inline void SetOAMADDR(std::uint8_t value) { m_OAMADDR = value; } // 先这么写

        // OAMDATA
        inline void SetOAMData(std::uint8_t value) { m_OAMDATA = value; }
        inline std::uint8_t GetOAMData() const { return m_OAMDATA; }

        // PPUSROLL
        void SetPPUSCROLL(std::uint8_t value);

        // PPUADDR
        void SetPPUADDR(std::uint8_t value);

        // PPUDATA
        void SetPPUDATA(std::uint8_t value);
        std::uint8_t GetPPUDATA();

        // 调色板索引
        std::uint8_t GetPalette(int index);
        void SetPalette(int index, std::uint8_t value);

    private:
        std::unique_ptr<std::uint8_t[]> m_VRAM = nullptr;

        std::uint8_t m_PPUCTRL = 0;
        std::uint8_t m_PPUMASK = 0;
        std::uint8_t m_PPUSTATUS = 0;
        std::uint8_t m_OAMADDR = 0;
        std::uint8_t m_OAMDATA = 0;
        std::uint16_t m_PPUADDR = 0; // 写两次

        std::uint8_t m_palette[0x20];

        // w一位，v15位，合并一下，这个寄存器的t位在这里是m_PPUADDR
        std::uint16_t m_internal_register_wt = 0;
        // 这个只有末3位有用
        std::uint8_t m_fine_x_scroll = 0;
        std::uint8_t m_PPUDATA_buffer = 0;

        int m_scanline = 0;
        int m_cycle = 0;
        unsigned int m_frame = 0;

        std::function<std::uint8_t(std::uint16_t)> m_mapper_read_CHR;
        std::function<void(std::uint16_t, std::uint8_t)> m_mapper_write_CHR;
        std::function<void()> m_trigger_NMI;

        std::shared_ptr<VirtualDevice> m_device;
    };
}
