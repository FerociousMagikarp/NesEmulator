#include "mappers/mapper0.h"
#include "cartridge.h"
#include <memory>

namespace nes
{
    Mapper0::Mapper0(Cartridge* cartridge) : Mapper(cartridge)
    {
        if (cartridge->GetCHRRom().size() == 0)
        {
            m_CHR_ram = std::make_unique<byte[]>(0x2000);
        }
    }

    byte Mapper0::ReadCHR(uint16 address)
    {
        if (m_cartridge->GetCHRRom().size() == 0)
            return m_CHR_ram[address];
        else
            return m_cartridge->GetCHRRom()[address];
    }

    void Mapper0::WriteCHR(uint16 address, byte value)
    {
         if (m_cartridge->GetCHRRom().size() == 0)
            m_CHR_ram[address] = value;
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

