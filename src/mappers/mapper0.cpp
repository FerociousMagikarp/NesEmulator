#include "mappers/mapper0.h"
#include "cartridge.h"
#include <memory>

namespace nes
{
    Mapper0::Mapper0(Cartridge* cartridge) : Mapper(cartridge)
    {
        if (cartridge->GetCHRRom().size() == 0)
        {
            m_CHR_ram = std::make_unique<std::uint8_t[]>(0x2000);
        }
    }

    std::uint8_t Mapper0::ReadCHR(std::uint16_t address)
    {
        if (m_cartridge->GetCHRRom().size() == 0)
            return m_CHR_ram[address];
        else
            return m_cartridge->GetCHRRom()[address];
    }

    void Mapper0::WriteCHR(std::uint16_t address, std::uint8_t value)
    {
         if (m_cartridge->GetCHRRom().size() == 0)
            m_CHR_ram[address] = value;
    }

    std::uint8_t Mapper0::ReadPRG(std::uint16_t address)
    {
        if (m_cartridge->GetPRGRom().size() == 0x4000)
            return m_cartridge->GetPRGRom()[(address - 0x8000) & 0x3fff];
        else
            return m_cartridge->GetPRGRom()[address - 0x8000];
    }

    void Mapper0::WritePRG(std::uint16_t address, std::uint8_t value)
    {

    }
}
