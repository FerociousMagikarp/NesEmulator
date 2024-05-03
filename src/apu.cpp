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
        constexpr std::array<int, 32> LENGTH_COUNTER_TABLE = 
        {
            10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
            12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
        };
        constexpr std::array<int, 16> NTSC_NOISE_TABLE = 
        {
            4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
        };
        // APU周期已经对半了，所以这个数比起Wiki的(https://www.nesdev.org/wiki/APU_DMC)也要除以2
        constexpr std::array<int, 16> NTSC_DMC_TABLE = 
        {
            214, 190, 170, 160, 143, 127, 113, 107, 95, 80, 71, 64, 53, 42, 36, 27,
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

        if (m_cycles++ % 2 == 0)
        {
            m_pulse1.Step();
            m_pulse2.Step();
            m_noise.Step();
            m_DMC.Step();
        }

        m_frame_counter += 1.0f;
        if (m_frame_counter > CPU_FRAME_RATIO)
        {
            m_frame_counter -= CPU_FRAME_RATIO;
            if (!m_mode)
            {
                m_frame_cycles = m_frame_cycles % 4;
                switch (m_frame_cycles)
                {
                    case 3:
                        if (m_interrupt)
                        {
                            m_trigger_IRQ();
                            m_frame_interrupt = true;
                        }
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
                        m_noise.StepEnvelope();
                        break;
                }
            }
            else
            {
                m_frame_cycles = m_frame_cycles % 5;
                switch (m_frame_cycles)
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
                        m_noise.StepEnvelope();
                        break;
                }
            }
            m_frame_cycles++;
        }

        m_output_record += 1.0f;
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
                m_DMC.data.SetControl(val);
                break;
            case 0x11:
                m_DMC.data.SetLoadCounter(val);
                break;
            case 0x12:
                m_DMC.data.SetSampleAddress(val);
                break;
            case 0x13:
                m_DMC.data.SetSampleLength(val);
                break;
            case 0x15:
                m_DMC.data.enable = val & 0x10;
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
                if (!m_DMC.data.enable)
                    m_DMC.data.cur_length = 0;
                else if (m_DMC.data.cur_length == 0)
                {
                    m_DMC.data.cur_address = m_DMC.data.sample_address;
                    m_DMC.data.cur_length = m_DMC.data.sample_length;
                }
                break;
            case 0x17:
                m_mode = val & 0x80;
                if ((val & 0x40) != 0)
                {
                    m_interrupt = false;
                    m_frame_interrupt = false;
                }
                else
                    m_interrupt = true;
                m_frame_cycles = 0;
                if (m_mode)
                {
                    m_pulse1.StepLength();
                    m_pulse2.StepLength();
                    m_triangle.StepLength();
                    m_noise.StepLength();
                    m_pulse1.StepSweep();
                    m_pulse2.StepSweep();
                    m_pulse1.StepEnvelope();
                    m_pulse2.StepEnvelope();
                    m_triangle.StepCounter();
                    m_noise.StepEnvelope();
                }
                break;
            default:
                break;
        }
    }

    std::uint8_t APU::ReadStatus()
    {
        std::uint8_t result = 0;
        if (m_frame_interrupt)
        {
            result |= 0x40;
            m_frame_interrupt = false;
        }
        result |= ((m_DMC.data.length_counter > 0) << 4);
        result |= (((m_noise.length_counter > 0) << 3));
        result |= (((m_triangle.length_counter > 0) << 2));
        result |= (((m_pulse2.length_counter > 0) << 1));
        result |= (((m_pulse1.length_counter > 0) << 0));
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
            divider_period = ((val & 0x70) >> 4) + 1;
            shift_count = val & 0x07;
            sweep_reload_flag = true;
        }

        void Pulse::SetTimerLow(std::uint8_t val)
        {
            timer &= 0xff00;
            timer |= static_cast<std::uint16_t>(val);
            sweep_muting = false;
        }

        void Pulse::SetTimerHigh(std::uint8_t val)
        {
            timer &= 0x00ff;
            timer |= static_cast<std::uint16_t>(val & 0x07) << 8;
            if (enable)
                length_counter = meta::LENGTH_COUNTER_TABLE[(val >> 3) & 0x1f];
            envelope_start_flag = true;
            cur_duty = 0;
            sweep_muting = false;
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
                envelope_value = volume;
                envelope_start_flag = false;
                return;
            }

            if (envelope_value > 0)
            {
                envelope_value--;
            }
            else
            {
                if (envelope_volume > 0)
                    envelope_volume--;
                else if (length_counter_halt)
                    envelope_volume = 15;
                envelope_value = volume;
            }
        }

        void Pulse::StepSweep()
        {
            if (sweep_reload_flag)
            {
                if (enable_sweep && sweep_value == 0 && !sweep_muting)
                    Sweep();
                sweep_value = divider_period;
                sweep_reload_flag = false;
                return;
            }

            if (sweep_value > 0)
            {
                sweep_value--;
            }
            else
            {
                if (enable_sweep && !sweep_muting)
                    Sweep();
                sweep_value = divider_period;
            }
        }

        void Pulse::Sweep()
        {
            std::uint16_t delta = (timer & 0x07ff) >> shift_count;
            std::uint16_t target_period = timer;
            if (negate_flag)
            {
                if (channel == 1)
                    delta += 1;
                if (delta >= target_period)
                    target_period = 0;
                else
                    target_period -= delta;
            }
            else
            {
                target_period += delta;
            }

            if (target_period < 8 || target_period > 0x7ff)
            {
                sweep_muting = true;
                return;
            }
            if (shift_count > 0)
                timer = target_period;
        }

        std::uint8_t Pulse::Output()
        {
            if (!enable || length_counter == 0 || meta::PULSE_DUTY_TABLE[duty][cur_duty] == 0 || timer < 8 || sweep_muting)
            {
                return 0;
            }

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
                length_counter = meta::LENGTH_COUNTER_TABLE[(val >> 3) & 0x1f];
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
            if (!enable || counter_value == 0 || length_counter == 0 || cur_time < 2)
                return 0;
            return meta::TRIANGLE_DUTY_TABLE[cur_duty];
        }

        void Noise::SetControl(std::uint8_t val)
        {
            length_counter_halt = val & 0x20;
            constant_volume = val & 0x10;
            volume = val & 0x0f;
        }

        void Noise::SetNoisePeriod(std::uint8_t val)
        {
            mode_flag = val & 0x80;
            timer_period = meta::NTSC_NOISE_TABLE[val & 0x0f];
        }

        void Noise::SetLengthCounter(std::uint8_t val)
        {
            if (enable)
                length_counter = meta::LENGTH_COUNTER_TABLE[(val >> 3) & 0x1f];
            envelope_start_flag = true;
        }

        void Noise::Step()
        {
            if (cur_time > 0)
            {
                cur_time--;
            }
            else
            {
                std::uint16_t val = 0;
                if (mode_flag)
                    val = ((shift_register >> 6) ^ shift_register) & 0x0001;
                else
                    val = ((shift_register >> 1) ^ shift_register) & 0x0001;
                shift_register >>= 1;
                shift_register |= (val << 14);

                cur_time = timer_period;
            }
        }

        void Noise::StepEnvelope()
        {
            if (envelope_start_flag)
            {
                envelope_volume = 15;
                envelope_value = volume;
                envelope_start_flag = false;
                return;
            }
            
            if (envelope_value > 0)
            {
                envelope_value--;
            }
            else
            {
                if (envelope_volume > 0)
                    envelope_volume--;
                else if (length_counter_halt)
                    envelope_volume = 15;
                envelope_value = volume;
            }
        }

        std::uint8_t Noise::Output()
        {
            if (!enable || length_counter == 0 || (shift_register & 0x01) == 1)
                return 0;
            if (constant_volume)
                return volume;
            else
                return envelope_volume;
        }

        void DMCData::SetControl(std::uint8_t val)
        {
            IRQ_enable = val & 0x80;
            loop = val & 0x40;
            frequency = meta::NTSC_DMC_TABLE[val & 0x0f];
        }

        void DMCData::SetLoadCounter(std::uint8_t val)
        {
            load_counter = val & 0x7f;
        }

        void DMCData::SetSampleAddress(std::uint8_t val)
        {
            sample_address = 0xc000 | (static_cast<std::uint16_t>(val) << 6);
        }

        void DMCData::SetSampleLength(std::uint8_t val)
        {
            sample_length = (static_cast<std::uint16_t>(val) << 4) + 1;
        }

        void DMC::Step()
        {
            if (!data.enable)
                return;

            if (data.cur_length > 0 && data.shift_count == 0) // 需要重新读一下数据
            {
                data.shift_reg = read_callback(data.cur_address++);
                data.shift_count = 8;
                data.cur_address |= 0x8000;
                if (--data.cur_length == 0 && data.loop)
                {
                    data.cur_address = data.sample_address;
                    data.cur_length = data.sample_length;
                }
            }

            if (data.cur_freq > 0)
            {
                data.cur_freq--;
            }
            else
            {
                data.cur_freq = data.frequency;
                if (data.shift_count > 0)
                {
                    if (data.shift_reg & 1)
                    {
                        if (data.output <= 125)
                            data.output += 2;
                    }
                    else
                    {
                        if (data.output >= 2)
                            data.output -= 2;
                    }
                    data.shift_reg >>= 1;
                    data.shift_count--;
                }
            }
        }

        std::uint8_t DMC::Output()
        {
            if (!data.enable)
                return 0;
            return data.output;
        }
    }

    std::vector<char> APU::Save() const
    {
        std::vector<char> res(GetSaveFileSize(SAVE_VERSION));

        auto pointer = res.data();

        pointer = UnsafeWrite(pointer, m_pulse1);
        pointer = UnsafeWrite(pointer, m_pulse2);
        pointer = UnsafeWrite(pointer, m_triangle);
        pointer = UnsafeWrite(pointer, m_noise);
        pointer = UnsafeWrite(pointer, m_DMC.data);
        pointer = UnsafeWrite(pointer, m_cycles);
        pointer = UnsafeWrite(pointer, m_frame_cycles);
        pointer = UnsafeWrite(pointer, m_mode);
        pointer = UnsafeWrite(pointer, m_interrupt);
        pointer = UnsafeWrite(pointer, m_frame_interrupt);
        pointer = UnsafeWrite(pointer, m_output_record);
        pointer = UnsafeWrite(pointer, m_frame_counter);

        return res;
    }

    std::size_t APU::GetSaveFileSize(int version) const noexcept
    {
        return 116;
    }
    
    void APU::Load(const std::vector<char>& data, int version)
    {
        if (data.size() != GetSaveFileSize(version))
            return;
        
        auto pointer = data.data();

        pointer = UnsafeRead(pointer, m_pulse1);
        pointer = UnsafeRead(pointer, m_pulse2);
        pointer = UnsafeRead(pointer, m_triangle);
        pointer = UnsafeRead(pointer, m_noise);
        pointer = UnsafeRead(pointer, m_DMC.data);
        pointer = UnsafeRead(pointer, m_cycles);
        pointer = UnsafeRead(pointer, m_frame_cycles);
        pointer = UnsafeRead(pointer, m_mode);
        pointer = UnsafeRead(pointer, m_interrupt);
        pointer = UnsafeRead(pointer, m_frame_interrupt);
        pointer = UnsafeRead(pointer, m_output_record);
        pointer = UnsafeRead(pointer, m_frame_counter);
    }
}
