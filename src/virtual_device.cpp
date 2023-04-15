#include "virtual_device.h"
#include "palette.h"
#include <assert.h>
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

    void VirtualDevice::SetPixel(int x, int y, int palette_index)
    {
        assert(x >= 0);
        assert(x < NES_HEIGHT);
        assert(y >= 0);
        assert(y < NES_WIDTH);
        assert(palette_index >= 0);
        assert(palette_index < PALETTE_COUNT);

        int index = (x * NES_WIDTH + y) * 4;
        PaletteColor color = DEFAULT_PALETTE[palette_index];

        m_screen[index] = color.b;
        m_screen[index + 1] = color.g;
        m_screen[index + 2] = color.r;
        m_screen[index + 3] = color.a;
    }
}
