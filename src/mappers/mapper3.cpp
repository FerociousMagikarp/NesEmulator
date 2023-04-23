#include "mappers/mapper3.h"
#include "cartridge.h"
#include <memory>

namespace nes
{
    Mapper3::Mapper3(Cartridge* cartridge) : Mapper(cartridge)
    {
        if (cartridge->GetCHRRom().size() == 0)
        {
            m_CHR_ram = std::make_unique<std::uint8_t[]>(0x2000);
        }
    }

    std::uint8_t Mapper3::ReadCHR(std::uint16_t address)
    {
        if (m_CHR_ram != nullptr)
            return m_CHR_ram[address];
        return m_cartridge->GetCHRRom()[(m_CHR_bank << 13) | address];
    }

    void Mapper3::WriteCHR(std::uint16_t address, std::uint8_t value)
    {
         if (m_CHR_ram != nullptr)
            m_CHR_ram[address] = value;
    }

    std::uint8_t Mapper3::ReadPRG(std::uint16_t address)
    {
        if (m_cartridge->GetPRGRom().size() == 0x4000)
            return m_cartridge->GetPRGRom()[(address - 0x8000) & 0x3fff];
        else
            return m_cartridge->GetPRGRom()[address - 0x8000];
    }

    void Mapper3::WritePRG(std::uint16_t address, std::uint8_t value)
    {
        m_CHR_bank = value & 0x03;
    }
}
