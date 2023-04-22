#include "apu.h"

namespace nes
{
    void APU::Reset()
    {

    }

    void APU::Step()
    {

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
            case 0x15: // TODO
                break;
            case 0x17: // TODO
                break;
            default:
                break;
        }
    }

    std::uint8_t APU::ReadStatus()
    {
        return 0;
    }

    namespace apu_channel
    {
        void Pulse::SetControl(std::uint8_t val)
        {

        }

        void Pulse::SetSweep(std::uint8_t val)
        {

        }

        void Pulse::SetTimerLow(std::uint8_t val)
        {

        }

        void Pulse::SetTimerHigh(std::uint8_t val)
        {

        }

        void Triangle::SetControl(std::uint8_t val)
        {

        }

        void Triangle::SetTimerLow(std::uint8_t val)
        {

        }

        void Triangle::SetTimerHigh(std::uint8_t val)
        {

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
    }
}
