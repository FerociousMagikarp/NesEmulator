#include "apu.h"
#include "virtual_device.h"

namespace nes
{
    namespace meta
    {
        consteval auto GeneratePulseTable()
        {
            std::array<float, 31> result{};
            result[0] = 0.0f;
            for (int i = 1; i < 31; i++)
                result[i] = 95.52f / (8128.0f / i + 100.0f);
            return result;
        }

        consteval auto GenerateTndTable()
        {
            std::array<float, 203> result{};
            result[0] = 0.0f;
            for (int i = 1; i < 203; i++)
                result[i] = 163.67f / (24329.0f / i + 100.0f);
            return result;
        }

        consteval auto GenerateTriangleDutyTable()
        {
            std::array<int, 32> result{};
            for (int i = 0; i < 16; i++)
                result[i] = 15 - i;
            for (int i = 16; i < 32; i++)
                result[i] = i - 16;
            return result;
        }

        constexpr auto PULSE_TABLE = GeneratePulseTable();
        constexpr auto TND_TABLE = GenerateTndTable();
        constexpr std::array<std::array<int, 8>, 4> PULSE_DUTY_TABLE = 
        {
            0, 1, 0, 0, 0, 0, 0, 0,
            0, 1, 1, 0, 0, 0, 0, 0,
            0, 1, 1, 1, 1, 0, 0, 0,
            1, 0, 0, 1, 1, 1, 1, 1
        };
        constexpr std::array<int, 32> TRIANGLE_DUTY_TABLE = GenerateTriangleDutyTable();
        constexpr std::array<int, 32> COUNTER_VALUE_TABLE = 
        {
            10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
            12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
        };
        constexpr std::array<int, 16> NTSC_NOISE_TABLE = 
        {
            4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
        };
    }

    void APU::Reset()
    {
        m_cycles = 0;
        m_frame_cycles = 0;
        m_output_record = 0.0f;
        m_frame_counter = 0.0f;
    }

    void APU::Step()
    {
        constexpr float CPU_AUDIO_RATIO = NTSC_CPU_FREQUENCY / static_cast<float>(AUDIO_FREQ);
        constexpr float CPU_FRAME_RATIO = NTSC_CPU_FREQUENCY / static_cast<float>(NTSC_FRAME_FREQUENCY);

        m_triangle.Step();

        if (m_cycles++ % 2 != 0)
            return;

        m_pulse1.Step();
        m_pulse2.Step();
        m_noise.Step();
        m_DMC.Step();

        m_frame_counter += 2.0f;
        if (m_frame_counter > CPU_FRAME_RATIO)
        {
            m_frame_counter -= CPU_FRAME_RATIO;
            m_frame_cycles++;
            if (!m_mode)
            {
                switch (m_frame_cycles % 4)
                {
                    case 3:
                        if (m_interrupt)
                            m_trigger_IRQ();
                        [[fallthrough]];
                    case 1:
                        m_pulse1.StepLength();
                        m_pulse2.StepLength();
                        m_triangle.StepLength();
                        m_noise.StepLength();
                        m_pulse1.StepSweep();
                        m_pulse2.StepSweep();
                        [[fallthrough]];
                    case 0:
                    case 2:
                        m_pulse1.StepEnvelope();
                        m_pulse2.StepEnvelope();
                        m_triangle.StepCounter();
                        break;
                }
            }
            else
            {
                switch (m_frame_cycles % 5)
                {
                    case 3:
                        break;
                    case 1:
                    case 4:
                        m_pulse1.StepLength();
                        m_pulse2.StepLength();
                        m_triangle.StepLength();
                        m_noise.StepLength();
                        m_pulse1.StepSweep();
                        m_pulse2.StepSweep();
                        [[fallthrough]];
                    case 0:
                    case 2:
                        m_pulse1.StepEnvelope();
                        m_pulse2.StepEnvelope();
                        m_triangle.StepCounter();
                        break;
                }
            }
        }

        m_output_record += 2.0f;
        if (m_output_record > CPU_AUDIO_RATIO)
        {
            m_output_record -= CPU_AUDIO_RATIO;
            auto output_pulse1 = m_pulse1.Output();
            auto output_pulse2 = m_pulse2.Output();
            auto output_triangle = m_triangle.Output();
            auto output_noise = m_noise.Output();
            auto output_dmc = m_DMC.Output();
            auto pulse = meta::PULSE_TABLE[output_pulse1 + output_pulse2];
            auto tnd = meta::TND_TABLE[3 * output_triangle + 2 * output_noise + output_dmc];
            m_device->PutAudioSample(static_cast<std::uint8_t>((pulse + tnd) * 256));
        }
    }

    void APU::SetRegister(std::uint16_t addr, std::uint8_t val)
    {
        switch (addr & 0xff)
        {
            case 0x00:
                m_pulse1.SetControl(val);
                break;
            case 0x01:
                m_pulse1.SetSweep(val);
                break;
            case 0x02:
                m_pulse1.SetTimerLow(val);
                break;
            case 0x03:
                m_pulse1.SetTimerHigh(val);
                break;
            case 0x04:
                m_pulse2.SetControl(val);
                break;
            case 0x05:
                m_pulse2.SetSweep(val);
                break;
            case 0x06:
                m_pulse2.SetTimerLow(val);
                break;
            case 0x07:
                m_pulse2.SetTimerHigh(val);
                break;
            case 0x08:
                m_triangle.SetControl(val);
                break;
            case 0x09: // 没用
                break;
            case 0x0a:
                m_triangle.SetTimerLow(val);
                break;
            case 0x0b:
                m_triangle.SetTimerHigh(val);
                break;
            case 0x0c:
                m_noise.SetControl(val);
                break;
            case 0x0d: // 没用
                break;
            case 0x0e:
                m_noise.SetNoisePeriod(val);
                break;
            case 0x0f:
                m_noise.SetLengthCounter(val);
                break;
            case 0x10:
                m_DMC.SetControl(val);
                break;
            case 0x11:
                m_DMC.SetLoadCounter(val);
                break;
            case 0x12:
                m_DMC.SetSampleAddress(val);
                break;
            case 0x13:
                m_DMC.SetSampleLength(val);
                break;
            case 0x15:
                m_DMC.enable = val & 0x10;
                m_noise.enable = val & 0x08;
                m_triangle.enable = val & 0x04;
                m_pulse2.enable = val & 0x02;
                m_pulse1.enable = val & 0x01;
                if (!m_pulse1.enable)
                    m_pulse1.length_counter = 0;
                if (!m_pulse2.enable)
                    m_pulse2.length_counter = 0;
                if (!m_triangle.enable)
                    m_triangle.length_counter = 0;
                if (!m_noise.enable)
                    m_noise.length_counter = 0;
                if (!m_DMC.enable)
                    m_DMC.length_counter = 0;
                break;
            case 0x17:
                m_mode = val & 0x80;
                m_interrupt = val & 0x40;
                break;
            default:
                break;
        }
    }

    std::uint8_t APU::ReadStatus()
    {
        // 还有一些OpenBus、同一帧不清标记等没实现。
        std::uint8_t result = 0;
        if (m_interrupt)
        {
            result |= 0x40;
            m_interrupt = false;
        }
        result |= ((m_DMC.length_counter > 0) << 4);
        result |= ((m_noise.length_counter > 0) << 3);
        result |= ((m_triangle.length_counter > 0) << 2);
        result |= ((m_pulse2.length_counter > 0) << 1);
        result |= ((m_pulse1.length_counter > 0) << 0);
        return result;
    }

    namespace apu_channel
    {
        void Channel::StepLength()
        {
            if (enable && length_counter > 0 && !length_counter_halt)
                length_counter--;
        }

        void Pulse::SetControl(std::uint8_t val)
        {
            duty = (val >> 6) & 0x03;
            length_counter_halt = val & 0x20;
            constant_volume = val & 0x10;
            volume = val & 0x0f;
        }

        void Pulse::SetSweep(std::uint8_t val)
        {
            enable_sweep = val & 0x80;
            negate_flag = val & 0x08;
            divider_period = (val & 0x70) >> 4;
            shift_count = val & 0x07;
            sweep_reload_flag = true;
        }

        void Pulse::SetTimerLow(std::uint8_t val)
        {
            timer &= 0xff00;
            timer |= static_cast<std::uint16_t>(val);
        }

        void Pulse::SetTimerHigh(std::uint8_t val)
        {
            timer &= 0x00ff;
            timer |= static_cast<std::uint16_t>(val & 0x07) << 8;
            if (enable)
                length_counter = meta::COUNTER_VALUE_TABLE[(val >> 3) & 0x1f];
            envelope_start_flag = true;
        }

        void Pulse::Step()
        {
            if (cur_time == 0)
            {
                cur_time = timer;
                cur_duty = (cur_duty + 1) % 8;
            }
            else
            {
                cur_time--;
            }
        }

        void Pulse::StepEnvelope()
        {
            if (envelope_start_flag)
            {
                envelope_volume = 15;
                envelope_value = constant_volume;
                envelope_start_flag = false;
            }
            else if (envelope_value > 0)
            {
                envelope_value--;
            }
            else
            {
                if (envelope_volume > 0)
                    envelope_volume--;
                else if (length_counter_halt)
                    envelope_volume = 15;
                envelope_value = constant_volume;
            }
        }

        void Pulse::StepSweep()
        {
            if (sweep_reload_flag)
            {
                if (enable_sweep && sweep_value == 0)
                    Sweep();
                sweep_value = divider_period;
                sweep_reload_flag = false;
            }
            else if (sweep_value > 0)
            {
                    sweep_value--;
            }
            else
            {
                if (enable_sweep)
                    Sweep();
                sweep_value = divider_period;
            }
        }

        void Pulse::Sweep()
        {
            std::uint8_t delta = divider_period >> shift_count;
            if (negate_flag)
            {
                cur_time -= delta;
                if (channel == 1)
                    cur_time--;
            }
            else
            {
                cur_time += delta;
            }
        }

        std::uint8_t Pulse::Output()
        {
            if (!enable || length_counter == 0 || meta::PULSE_DUTY_TABLE[duty][cur_duty] == 0 || cur_time < 8 || cur_time > 0x7ff)
                return 0;
            if (constant_volume)
                return volume;
            else
                return envelope_volume;
        }

        void Triangle::SetControl(std::uint8_t val)
        {
            length_counter_halt = val & 0x80;
            counter_reload_value = val & 0x7f;
        }

        void Triangle::SetTimerLow(std::uint8_t val)
        {
            timer &= 0xff00;
            timer |= static_cast<std::uint16_t>(val);
        }

        void Triangle::SetTimerHigh(std::uint8_t val)
        {
            timer &= 0x00ff;
            timer |= static_cast<std::uint16_t>(val & 0x07) << 8;
            if (enable)
                length_counter = meta::COUNTER_VALUE_TABLE[(val >> 3) & 0x1f];
            counter_reload_flag = true;
        }

        void Triangle::Step()
        {
            if (cur_time == 0)
            {
                cur_time = timer;
                if (length_counter > 0 && counter_value > 0)
                    cur_duty = (cur_duty + 1) % 32;
            }
            else
            {
                cur_time--;
            }
        }

        void Triangle::StepCounter()
        {
            if (counter_reload_flag)
                counter_value = counter_reload_value;
            else if (counter_value > 0)
                counter_value--;

            if (!length_counter_halt)
                counter_reload_flag = false;
        }

        std::uint8_t Triangle::Output()
        {
            if (!enable || counter_value == 0 || length_counter == 0)
                return 0;
            return meta::TRIANGLE_DUTY_TABLE[cur_duty];
        }

        void Noise::SetControl(std::uint8_t val)
        {

        }

        void Noise::SetNoisePeriod(std::uint8_t val)
        {

        }

        void Noise::SetLengthCounter(std::uint8_t val)
        {

        }

        void Noise::Step()
        {

        }

        std::uint8_t Noise::Output()
        {
            return 0;
        }

        void DMC::SetControl(std::uint8_t val)
        {

        }

        void DMC::SetLoadCounter(std::uint8_t val)
        {

        }

        void DMC::SetSampleAddress(std::uint8_t val)
        {

        }

        void DMC::SetSampleLength(std::uint8_t val)
        {

        }

        void DMC::Step()
        {

        }

        std::uint8_t DMC::Output()
        {
            return 0;
        }
    }
}
