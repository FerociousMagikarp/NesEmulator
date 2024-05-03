#include "virtual_device.h"
#include "palette.h"
#include <assert.h>
#include <cstdint>

namespace nes
{
    void VirtualDevice::ApplicationUpdate()
    {
        m_write_screen_finish.wait(false);
        m_app_update_callback();
        m_write_screen_finish.store(false);
    }

    void VirtualDevice::ApplicationKeyDown(Player player, InputKey key)
    {
        m_keyboard[static_cast<int>(player)][static_cast<int>(key)] = true;
    }

    void VirtualDevice::ApplicationKeyUp(Player player, InputKey key)
    {
        m_keyboard[static_cast<int>(player)][static_cast<int>(key)] = false;
    }

    bool VirtualDevice::IsKeyDown(Player player, InputKey key) const
    {
        return m_keyboard[static_cast<int>(player)][static_cast<int>(key)];
    }

    void VirtualDevice::ApplicationTurboTick()
    {
        using enum Player;
        using enum InputKey;

        // 如果没有按下连发键就不需要更新时间了
        if (IsKeyDown(Player1, TurboA) || IsKeyDown(Player1, TurboB) || IsKeyDown(Player2, TurboA) || IsKeyDown(Player2, TurboB))
        {
            auto now = std::chrono::steady_clock::now();
            auto delta_time = now - m_turbo_time;
            if (delta_time.count() / 1000000 >= m_turbo_time_interval_ms)
            {
                m_is_turbo = !m_is_turbo;
                m_turbo_time = now;
            }
        }

        // 这个活放到 application 线程来干
        auto c1 = GetNesKey(Player::Player1);
        auto c2 = GetNesKey(Player::Player2);
        m_controllers.store(static_cast<std::uint16_t>(c1) << 8 | c2);
    }

    std::uint8_t VirtualDevice::GetNesKey(Player player) const
    {
        std::uint8_t res = 0;
        for (int i = 0; i < 8; i++)
        {
            res |= (static_cast<std::uint8_t>(m_keyboard[static_cast<int>(player)][i]) << i);
        }
        if (m_is_turbo)
        {
            if (IsKeyDown(player, InputKey::TurboA))
                res |= 1;
            if (IsKeyDown(player, InputKey::TurboB))
                res |= 2;
        }
        return res;
    }

    void VirtualDevice::StartPPURender()
    {
    }

    void VirtualDevice::EndPPURender()
    {
        m_write_screen_finish.store(true);
        m_write_screen_finish.notify_one();
    }

    void VirtualDevice::FillAudioSamples(unsigned char* stream, int len)
    {
        if (m_audio_samples.size() > 1)
        {
            std::lock_guard<std::mutex> lock(m_audio_mutex);
            for (int i = 0; i < len; i++)
            {
                stream[i] = m_audio_samples.front().data[i];
            }
            m_garbage_audio_samples.splice(m_garbage_audio_samples.cend(), m_audio_samples, m_audio_samples.cbegin());
        }
    }

    void VirtualDevice::PutAudioSample(std::uint8_t sample)
    {
        if (m_audio_samples.empty() || m_audio_samples.back().index >= AUDIO_BUFFER_SAMPLES)
        {
            // 添加新的buffer放在这
            std::lock_guard<std::mutex> lock(m_audio_mutex);
            if (!m_garbage_audio_samples.empty())
            {
                m_audio_samples.splice(m_audio_samples.cend(), m_garbage_audio_samples, m_garbage_audio_samples.cbegin());
                m_audio_samples.back().index = 0;
            }
            else
            {
                m_audio_samples.push_back(AudioSamples{});
            }
        }
        auto& container = m_audio_samples.back();
        container.data[container.index++] = sample;
    }

    void VirtualDevice::Write4016(std::uint8_t val)
    {
        m_strobe = val & 0x7; // 就后三位有用，虽然目前只用最后一位
        if ((m_strobe & 0x01) == 0) // 最后一位空了就开始读
        {
            std::uint16_t input = m_controllers.load();
            m_shift_controller1 = static_cast<std::uint8_t>(input >> 8);
            m_shift_controller2 = static_cast<std::uint8_t>(input & 0xff);
        }
    }

    std::uint8_t VirtualDevice::Read4016()
    {
        std::uint8_t res = 0;
        if ((m_strobe & 0x01) != 0)
        {
            std::uint16_t current_input = m_controllers.load();
            res = static_cast<std::uint8_t>(current_input >> 8) & 0x01;
        }
        else
        {
            res = m_shift_controller1 & 0x01;
            m_shift_controller1 >>= 1;
        }
        return res | 0x40;
    }

    std::uint8_t VirtualDevice::Read4017()
    {
        std::uint8_t res = 0;
        if ((m_strobe & 0x01) != 0)
        {
            std::uint16_t current_input = m_controllers.load();
            res = static_cast<std::uint8_t>(current_input & 0xff) & 0x01;
        }
        else
        {
            res = m_shift_controller2 & 0x01;
            m_shift_controller2 >>= 1;
        }
        return res | 0x40;
    }

    void VirtualDevice::SetPixel(int x, int y, int palette_index)
    {
        assert(x >= 0);
        assert(x < NES_WIDTH);
        assert(y >= 0);
        assert(y < NES_HEIGHT);
        assert(palette_index >= 0);
        assert(palette_index < PALETTE_COUNT);

        int index = (y * NES_WIDTH + x) * 4;
        PaletteColor color = DEFAULT_PALETTE[palette_index];

        m_screen[index] = color.b;
        m_screen[index + 1ull] = color.g;
        m_screen[index + 2ull] = color.r;
        m_screen[index + 3ull] = color.a;
    }
}
