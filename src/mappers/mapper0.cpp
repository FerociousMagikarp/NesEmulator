#include "mappers/mapper0.h"
#include "cartridge.h"

namespace nes
{
    byte Mapper0::ReadCHR(uint16 address)
    {
        return 0;
    }

    void Mapper0::WriteCHR(uint16 address, byte value)
    {

    }

    byte Mapper0::ReadPRG(uint16 address)
    {
        if (m_cartridge->GetPRGRom().size() == 0x4000)
            return m_cartridge->GetPRGRom()[(address - 0x8000) & 0x3fff];
        else
            return m_cartridge->GetPRGRom()[address - 0x8000];
    }

    void Mapper0::WritePRG(uint16 address, byte value)
    {

    }
}

