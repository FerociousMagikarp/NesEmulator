#pragma once

#include "def.h"
#include <functional>

namespace nes
{
    class PPU
    {
    public:
        PPU() noexcept;
        ~PPU();

        void Reset();
        void Step();

        // 这边减 0x2000 是因为CPU内存的0x2000-0x2007的镜像
        inline byte GetRegister(int index) { return m_register[index - 0x2000]; }
        inline void SetRegister(int index, byte value) { m_register[index - 0x2000] = value; }
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

        // 设置从PPU总线读写的回调
        inline void SetReadFunction(std::function<byte(uint16)>&& callback) { m_read_function = std::move(callback); }
        inline void SetWriteFunction(std::function<void(uint16, byte)>&& callback) { m_write_function = std::move(callback); }

    private:
        byte m_register[8];
        byte m_palette[0x20];

        std::function<byte(uint16)> m_read_function;
        std::function<void(uint16, byte)> m_write_function;
    };
}
