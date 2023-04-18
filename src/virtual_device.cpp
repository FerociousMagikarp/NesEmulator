#include "virtual_device.h"
#include "palette.h"
#include <assert.h>
#include <cstdint>
#include <iostream>

namespace nes
{
    void VirtualDevice::ApplicationUpdate()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_can_app_update)
            m_cond.wait(lock);
        m_app_update_callback();
    }

    void VirtualDevice::ApplicationSetControllers(std::uint8_t controller1, std::uint8_t controller2)
    {
        m_controllers = static_cast<std::uint16_t>(controller1) << 8 | controller2;
    }

    void VirtualDevice::StartPPURender()
    {
        while (m_can_app_update)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_can_app_update = false;
        }
    }

    void VirtualDevice::EndPPURender()
    {
        while (!m_can_app_update)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_can_app_update = true;
        }
        m_cond.notify_one();
    }

    void VirtualDevice::Write4016(std::uint8_t val)
    {
        m_strobe = val & 0x7; // 就后三位有用，虽然目前只用最后一位
        if ((m_strobe & 0x01) == 0) // 最后一位空了就开始读
        {
            std::uint16_t input = m_controllers;
            m_shift_controller1 = static_cast<std::uint8_t>(input >> 8);
            m_shift_controller2 = static_cast<std::uint8_t>(input & 0xff);
        }
    }

    std::uint8_t VirtualDevice::Read4016()
    {
        std::uint8_t res = 0;
        if ((m_strobe & 0x01) != 0)
        {
            std::uint16_t current_input = m_controllers;
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
            std::uint16_t current_input = m_controllers;
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
