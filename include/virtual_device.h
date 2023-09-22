#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <functional>
#include <list>
#include "def.h"

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

            inline std::uint8_t* GetScreenPointer() const { return m_screen.get(); }
            inline void SetApplicationUpdateCallback(std::function<void(void)>&& callback) { m_app_update_callback = std::move(callback); }

            void ApplicationUpdate();
            void ApplicationSetControllers(std::uint8_t controller1, std::uint8_t controller2);

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
            int m_scale = 3;

            // 两个手柄按键放一块了，为了不加锁，先1再2
            // 顺序 ： → ← ↓ ↑ Start Select B A
            std::uint16_t m_controllers = 0;
            std::uint8_t m_shift_controller1 = 0;
            std::uint8_t m_shift_controller2 = 0;

            std::unique_ptr<std::uint8_t[]> m_screen;

            // 读取和写入屏幕信息的时候都要加锁，控制器信息也是
            std::mutex m_mutex;
            std::condition_variable m_cond;

            // 操作音频buffer的时候加的锁
            std::mutex m_audio_mutex;

            std::function<void(void)> m_app_update_callback;

            bool m_can_app_update = false;
            std::uint8_t m_strobe = 0;

            struct AudioSamples
            {
                std::array<std::uint8_t, AUDIO_BUFFER_SAMPLES> data;
                unsigned int index = 0;
            };

            std::list<AudioSamples> m_audio_samples;
            std::list<AudioSamples> m_garbage_audio_samples;
    };
}
