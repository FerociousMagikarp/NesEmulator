#pragma once

#include "apu.h"
#include <cstdint>
#include <functional>
#include <memory>

namespace nes
{
    class VirtualDevice;

    namespace apu_channel
    {
        struct Channel
        {
            bool enable = false;
            bool length_counter_halt = false;
            std::uint8_t length_counter = 0;

            void StepLength();
        };

        struct Pulse : public Channel
        {
            void SetControl(std::uint8_t val);
            void SetSweep(std::uint8_t val);
            void SetTimerLow(std::uint8_t val);
            void SetTimerHigh(std::uint8_t val);

            void Step();
            void StepEnvelope();
            void StepSweep();
            std::uint8_t Output();

            void Sweep();
            
            std::uint8_t channel;
            std::uint8_t duty;
            
            bool constant_volume = false;
            std::uint8_t volume = 0;
            std::uint8_t cur_duty = 0;
            std::uint16_t timer = 0;
            std::uint16_t cur_time = 0;
            bool envelope_start_flag = false;
            std::uint8_t envelope_volume = 0;
            std::uint8_t envelope_value = 0;
            bool enable_sweep = false;
            std::uint8_t divider_period = 0;
            bool negate_flag = false;
            std::uint8_t shift_count = 0;
            bool sweep_reload_flag = false;
            std::uint8_t sweep_value = 0;
        };

        struct Triangle : public Channel
        {
            void SetControl(std::uint8_t val);
            void SetTimerLow(std::uint8_t val);
            void SetTimerHigh(std::uint8_t val);

            void Step();
            void StepCounter();
            std::uint8_t Output();

            std::uint16_t timer = 0;
            std::uint16_t cur_time = 0;
            std::uint8_t counter_reload_value = 0;
            std::uint8_t counter_value = 0;
            std::uint8_t cur_duty = 0;
            bool counter_reload_flag = false;
        };

        struct Noise : public Channel
        {
            void SetControl(std::uint8_t val);
            void SetNoisePeriod(std::uint8_t val);
            void SetLengthCounter(std::uint8_t val);

            void Step();
            std::uint8_t Output();
        };

        struct DMC : public Channel
        {
            void SetControl(std::uint8_t val);
            void SetLoadCounter(std::uint8_t val);
            void SetSampleAddress(std::uint8_t val);
            void SetSampleLength(std::uint8_t val);

            void Step();
            std::uint8_t Output();
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
            inline void SetDevice(std::shared_ptr<VirtualDevice> device) { m_device = std::move(device); }
            // 就能读$4015这一个。。。
            std::uint8_t ReadStatus();

            // 本来硬件上APU和CPU在一块的，但是写的时候分开了，所以中断只能回调了，无奈出此下策
            void SetIRQCallback(std::function<void()>&& callback) { m_trigger_IRQ = std::move(callback); }

        private:
            std::function<void()> m_trigger_IRQ;

            apu_channel::Pulse    m_pulse1 { .channel = 1 };
            apu_channel::Pulse    m_pulse2 { .channel = 2 };
            apu_channel::Triangle m_triangle;
            apu_channel::Noise    m_noise;
            apu_channel::DMC      m_DMC;

            std::shared_ptr<VirtualDevice> m_device;
            unsigned int m_cycles = 0;
            unsigned int m_frame_cycles = 0;

            bool m_mode;
            bool m_interrupt;

            float m_output_record = 0.0f;
            float m_frame_counter = 0.0f;
    };
}
