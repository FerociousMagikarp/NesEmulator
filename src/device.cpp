#include "device.h"
#include "def.h"
#include <memory>

namespace nes
{
    Device::Device()
    {
        m_screen = std::make_unique<byte[]>(WIDTH * HEIGHT * 4);
    }
}