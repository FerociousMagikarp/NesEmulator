#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

namespace nes
{
    constexpr int NES_WIDTH  = 256;
    constexpr int NES_HEIGHT = 240;

    class VirtualDevice
    {
        public:
            VirtualDevice() : m_screen(std::make_unique<std::uint8_t[]>(NES_WIDTH * NES_HEIGHT * 4)) {}
            ~VirtualDevice() = default;

            inline const int GetWidth() const { return NES_WIDTH * m_scale; }
            inline const int GetHeight() const { return NES_HEIGHT * m_scale; }

            inline void SetScale(int scale) { m_scale = scale; }
            inline const int GetScale() const { return m_scale; }

            // TODO : 记得加锁（在制作PPU的时候）
            inline std::uint8_t* GetScreenPointer() const { return m_screen.get(); }

        private:
            int m_scale = 3;

            std::uint8_t m_controller1 = 0;
            std::uint8_t m_controller2 = 0;
            std::uint8_t m_shift_controller1 = 0;
            std::uint8_t m_shift_controller2 = 0;

            std::unique_ptr<std::uint8_t[]> m_screen;

            // 读取和写入屏幕信息的时候都要加锁，控制器信息也是
            std::mutex m_mutex;
            std::condition_variable m_cond;
    };
}
