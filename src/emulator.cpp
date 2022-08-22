#include "emulator.h"

namespace nes
{
    Emulator::Emulator() noexcept
    {

    }

    Emulator::~Emulator()
    {

    }

    void Emulator::PutInCartridge(Cartridge *cartridge)
    {
        m_cartridge = cartridge;
    }
}
