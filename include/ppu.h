#pragma once

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

    private:
        std::uint8_t PPUBusRead(std::uint16_t address);
        void PPUBusWrite(std::uint16_t address, std::uint8_t value);

        // PPUCTRL
        // TODO : 末两位还影响scroll？ 还有SpriteSize没写，Master/Slave Mode也没写
        inline int GetNametableAddress() { return (m_register[0] & 0x03); }
        inline std::uint16_t GetAddressIncrement() { return (m_register[0] & 0x04) ? 1 : 32; }
        inline std::uint16_t GetSpritePatternTableAddress() { return (m_register[0] & 0x08) ? 0x0000 : 0x1000; }
        inline std::uint16_t GetBackgroundPatternTableAddress() { return (m_register[0] & 0x10) ? 0x0000 : 0x1000; }
        inline bool IsNMIEnabled() { return (m_register[0] & 0x80); }

        // OAMADDR
        inline void SetOAMADDR(std::uint8_t value) { m_register[3] = value; } // 先这么写

        // OAMDATA
        inline void SetOAMData(std::uint8_t value) { m_register[4] = value; }
        inline std::uint8_t GetOAMData() { return m_register[4]; }

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

        std::uint8_t m_register[8];
        std::uint8_t m_palette[0x20];

        std::uint16_t m_tmp_address = 0;
        std::uint16_t m_data_address = 0;
        bool m_first_write_address = true;
    };
}
