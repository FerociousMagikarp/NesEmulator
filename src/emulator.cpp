#include "emulator.h"

namespace nes
{
    Emulator::Emulator()
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
