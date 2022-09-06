#pragma once

#include "def.h"
#include <functional>
#include <memory>

namespace nes
{
    class Cartridge;

    class PPU
    {
    public:
        PPU();
        ~PPU();

        void Reset();
        void Step();

        inline void SetCartridge(Cartridge* cartridge) { m_cartridge = cartridge; }

        byte GetRegister(uint16 address);
        void SetRegister(uint16 address, byte value);
        // 调色板索引
        inline byte GetPalette(int index) 
        {
            // 调色板镜像
            if (index >= 0x10 && (index & 0x03) == 0)
                return m_palette[index & 0x0f];
            else
                return m_palette[index];
        }
        inline void SetPalette(int index, byte value)
        {
            if (index >= 0x10 && (index & 0x03) == 0)
                m_palette[index & 0x0f] = value;
            else
                m_palette[index] = value;
        }

    private:
        byte PPUBusRead(uint16 address);
        void PPUBusWrite(uint16 address, byte value);

        // PPUCTRL
        // TODO : 末两位还影响scroll？ 还有SpriteSize没写，Master/Slave Mode也没写
        inline int GetNametableAddress() { return (m_register[0] & 0x03); }
        inline uint16 GetAddressIncrement() { return (m_register[0] & 0x04) ? 1 : 32; }
        inline uint16 GetSpritePatternTableAddress() { return (m_register[0] & 0x08) ? 0x0000 : 0x1000; }
        inline uint16 GetBackgroundPatternTableAddress() { return (m_register[0] & 0x10) ? 0x0000 : 0x1000; }
        inline bool IsNMIEnabled() { return (m_register[0] & 0x80); }

        // OAMADDR
        inline void SetOAMADDR(byte value) { m_register[3] = value; } // 先这么写

        // OAMDATA
        inline void SetOAMData(byte value) { m_register[4] = value; }
        inline byte GetOAMData() { return m_register[4]; }

        // PPUSROLL
        void SetPPUSCROLL(byte value);

        // PPUADDR
        void SetPPUADDR(byte value);

        // PPUDATA
        void SetPPUDATA(byte value);
        byte GetPPUDATA();

    private:
        Cartridge* m_cartridge = nullptr;
        std::unique_ptr<byte[]> m_VRAM = nullptr;

        byte m_register[8];
        byte m_palette[0x20];

        uint16 m_tmp_address = 0;
        uint16 m_data_address = 0;
        bool m_first_write_address = true;
    };
}
