#pragma once

#include "virtual_device.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace nes
{
    class Cartridge;
    class Device;

    enum class MirrorType
    {
        Horizontal,
        Vertical,
    };

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
        inline void SetMirrorType(MirrorType type) { m_mirror_type = type; }

        void OAMDMA(std::uint8_t* data);

    private:
        std::uint8_t PPUBusRead(std::uint16_t address);
        void PPUBusWrite(std::uint16_t address, std::uint8_t value);

        std::uint16_t GetVRAMAddress(std::uint16_t address);

        inline bool IsRenderingCycle() const { return m_cycle >= 1 && m_cycle <= 256; }
        inline bool IsFetchingCycle() const { return m_cycle >= 321 && m_cycle <= 336; }

        void StepPreRenderScanline();
        void StepVisibleScanlines();
        void StepPostRenderScanline();
        void StepVerticalBlankingLines();

        void IncHorizontal();
        void IncVertical();

        void FetchingNametable();
        void FetchingAttribute();
        void FetchingPatternLow();
        void FetchingPatternHigh();

        void FetchingTile();
        void ShiftTile();

        void SpriteEvaluation(int scanline);

        // PPUCTRL
        // TODO : Master/Slave Mode没写
        inline int GetNametableAddress() const { return (m_PPUCTRL & 0x03); }
        inline std::uint16_t GetAddressIncrement() const { return !(m_PPUCTRL & 0x04) ? 1 : 32; }
        inline std::uint16_t GetSpritePatternTableAddress() const { return !(m_PPUCTRL & 0x08) ? 0x0000 : 0x1000; }
        inline std::uint16_t GetBackgroundPatternTableAddress() const { return !(m_PPUCTRL & 0x10) ? 0x0000 : 0x1000; }
        inline bool IsNMIEnabled() const { return static_cast<bool>(m_PPUCTRL & 0x80); }
        inline bool IsSpriteSize8x16() const { return static_cast<bool>(m_PPUCTRL & 0x20); }
        void SetPPUCTRL(std::uint8_t value);

        // PPUMASK
        inline bool IsShowBackgroundEnabled() const { return static_cast<bool>(m_PPUMASK & 0x08); }
        inline bool IsShowSpriteEnabled() const { return static_cast<bool>(m_PPUMASK & 0x10); }
        inline bool IsRenderingEnabled() const { return (m_PPUMASK & 0x18) != 0; }
        inline bool IsBothBgAndSpEnabled() const { return (m_PPUMASK & 0x18) == 0x18; }
        inline bool IsShowBackgroundLeftmost8() const { return (m_PPUMASK & 0x02); }
        inline bool IsShowSpriteLeftmost8() const { return (m_PPUMASK & 0x04); }

        // PPUSTATUS
        std::uint8_t GetPPUSTATUS();
        inline bool IsSprite0Hit() const { return (m_PPUSTATUS & 0x40) != 0; }

        // OAMADDR
        void SetOAMADDR(std::uint8_t value);

        // OAMDATA
        void SetOAMData(std::uint8_t value);
        std::uint8_t GetOAMData() const;

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

        std::uint8_t m_palette[0x20] = { 0 };

        // w一位，v15位，合并一下，这个寄存器的t位在这里是m_PPUADDR
        std::uint16_t m_internal_register_wt = 0;
        // 这个只有末3位有用
        std::uint8_t m_fine_x_scroll = 0;
        std::uint8_t m_PPUDATA_buffer = 0;

        // 记录在始终周期内从显存读到的数据
        std::uint8_t m_nametable = 0;
        std::uint16_t m_attribute_table = 0;
        std::uint16_t m_pattern_low = 0;
        std::uint16_t m_pattern_high = 0;

        std::array<std::uint8_t, 64 * 4> m_primary_OAM;
        // 单纯存一下m_primary_OAM的坐标
        std::vector<int> m_secondary_OAM;

        int m_scanline = 0;
        int m_cycle = 0;
        unsigned int m_frame = 0;

        MirrorType m_mirror_type = MirrorType::Horizontal;

        std::function<std::uint8_t(std::uint16_t)> m_mapper_read_CHR;
        std::function<void(std::uint16_t, std::uint8_t)> m_mapper_write_CHR;
        std::function<void()> m_trigger_NMI;

        std::shared_ptr<VirtualDevice> m_device;
    };
}