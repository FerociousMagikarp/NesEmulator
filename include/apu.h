#pragma once

#include "apu.h"
#include <cstdint>
#include <functional>
namespace nes
{
    namespace apu_channel
    {
        struct Pulse
        {
            void SetControl(std::uint8_t val);
            void SetSweep(std::uint8_t val);
            void SetTimerLow(std::uint8_t val);
            void SetTimerHigh(std::uint8_t val);
        };

        struct Triangle
        {
            void SetControl(std::uint8_t val);
            void SetTimerLow(std::uint8_t val);
            void SetTimerHigh(std::uint8_t val);
        };

        struct Noise
        {
            void SetControl(std::uint8_t val);
            void SetNoisePeriod(std::uint8_t val);
            void SetLengthCounter(std::uint8_t val);
        };

        struct DMC
        {
            void SetControl(std::uint8_t val);
            void SetLoadCounter(std::uint8_t val);
            void SetSampleAddress(std::uint8_t val);
            void SetSampleLength(std::uint8_t val);
        };
    }

    class APU
    {
        public:
            APU() = default;
            ~APU() = default;

            void Reset();
            void Step();

            void SetRegister(std::uint16_t addr, std::uint8_t val);
            // 就能读$4015这一个。。。
            std::uint8_t ReadStatus();

            // 本来硬件上APU和CPU在一块的，但是写的时候分开了，所以中断只能回调了，无奈出此下策
            void SetIRQCallback(std::function<void()>&& callback) { m_trigger_IRQ = std::move(callback); }

        private:
            std::function<void()> m_trigger_IRQ;

            apu_channel::Pulse    m_pulse1;
            apu_channel::Pulse    m_pulse2;
            apu_channel::Triangle m_triangle;
            apu_channel::Noise    m_noise;
            apu_channel::DMC      m_DMC;
    };
}
