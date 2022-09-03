#pragma once

#include "def.h"
#include <memory>

namespace nes
{
    class Device
    {
    public:
        Device();
        ~Device() = default;

        inline byte* GetScreenPointer() { if (m_screen) return m_screen.get(); return nullptr; }

    private:
        std::unique_ptr<byte[]> m_screen = nullptr;
    };
}
