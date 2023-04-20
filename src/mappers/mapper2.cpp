#include "mappers/mapper2.h"
#include "cartridge.h"

namespace nes
{
    Mapper2::Mapper2(Cartridge* cartridge) : Mapper(cartridge)
    {
        m_CHR_ram = std::make_unique<std::uint8_t[]>(0x2000);
    }

    std::uint8_t Mapper2::ReadCHR(std::uint16_t address)
    {
        return m_CHR_ram[address];
    }

    void Mapper2::WriteCHR(std::uint16_t address, std::uint8_t value)
    {
        m_CHR_ram[address] = value;
    }

    std::uint8_t Mapper2::ReadPRG(std::uint16_t address)
    {
        if (address < 0xc000)
            return m_cartridge->GetPRGRom()[(address - 0x8000) | (m_select << 14)];
        else
            return m_cartridge->GetPRGRom()[m_cartridge->GetPRGRom().size() - 0x4000 + (address & 0x3fff)];
    }

    void Mapper2::WritePRG(std::uint16_t address, std::uint8_t value)
    {
        m_select = value & 0x0f;
    }
}
