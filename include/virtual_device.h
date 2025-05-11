#pragma once

#include <atomic>
#include <mutex>
#include <functional>
#include <list>
#include <array>
#include "def.h"

namespace nes
{
    class VirtualDevice
    {
        public:
            VirtualDevice() = default;
            ~VirtualDevice() = default;

            const int GetWidth() const noexcept { return NES_WIDTH * m_scale; }
            const int GetHeight() const noexcept { return NES_HEIGHT * m_scale; }

            void SetScale(int scale) noexcept { m_scale = scale; }
            const int GetScale() const noexcept { return m_scale; }
            void SetTurboTimeIntervalMs(std::uint64_t val) noexcept { m_turbo_time_interval_ms = val; }
            std::uint64_t GetTurboTimeIntervalMs() const noexcept { return m_turbo_time_interval_ms; }

            std::uint8_t* GetScreenPtr() noexcept { return m_screen.data(); }
            void SetApplicationUpdateCallback(std::function<void(const std::uint8_t*)>&& callback) { m_app_update_callback = std::move(callback); }

            void ApplicationUpdate();
            void ApplicationKeyDown(Player player, InputKey key);
            void ApplicationKeyUp(Player player, InputKey key);
            void ApplicationTurboTick();

            void StartPPURender();
            void EndPPURender();

            void FillAudioSamples(unsigned char* stream, int len);
            void PutAudioSample(std::uint8_t sample);

            void Write4016(std::uint8_t val);
            std::uint8_t Read4016();
            std::uint8_t Read4017();

            // PPU用来设置像素的，传位置和调色板索引
            //  x :      0       1
            //  y : 0 (0, 0)  (1, 0), ...
            //      1 (0, 1)  (1, 1), ...
            void SetPixel(int x, int y, int palette_index);
            
        private:
            std::uint8_t GetNesKey(Player player) const;
            bool IsKeyDown(Player player, InputKey key) const;

            int m_scale = 3;

            // 两个手柄按键放一块了，先1再2
            // 顺序 ： → ← ↓ ↑ Start Select B A
            std::atomic<std::uint16_t> m_controllers = 0;
            std::uint8_t m_shift_controller1 = 0;
            std::uint8_t m_shift_controller2 = 0;

            std::array<std::uint8_t, NES_WIDTH * NES_HEIGHT * 4> m_screen;

            // 读取和写入时的锁
            std::atomic<bool> m_write_screen_finish = false;

            // 操作音频buffer的时候加的锁
            std::mutex m_audio_mutex;

            std::function<void(const std::uint8_t*)> m_app_update_callback;

            std::uint8_t m_strobe = 0;

            // 两个player，10种按键（包括连发），先这么写
            std::array<std::array<bool, 10>, 2> m_keyboard{};
            bool m_is_turbo = false;

            std::int64_t m_turbo_time_interval_ms = 20;
            std::chrono::steady_clock::time_point m_turbo_time{};

            struct AudioSamples
            {
                std::array<std::uint8_t, AUDIO_BUFFER_SAMPLES> data;
                unsigned int index = 0;
            };

            std::list<AudioSamples> m_audio_samples;
            std::list<AudioSamples> m_garbage_audio_samples;
    };
}
