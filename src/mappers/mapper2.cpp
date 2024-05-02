#include "mappers/mapper2.h"
#include "cartridge.h"

namespace nes
{
    constexpr std::size_t CHR_RAM_SIZE = 0x2000;

    Mapper2::Mapper2(Cartridge* cartridge) : Mapper(cartridge)
    {
        m_CHR_ram = std::make_unique<std::uint8_t[]>(CHR_RAM_SIZE);
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

    std::vector<char> Mapper2::Save() const
    {
        std::vector<char> res(GetSaveFileSize(SAVE_VERSION));

        auto pointer = res.data();

        pointer = UnsafeWrite(pointer, m_select);
        
        for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
        {
            pointer = UnsafeWrite(pointer, m_CHR_ram[i]);
        }

        return res;
    }

    std::size_t Mapper2::GetSaveFileSize(int version) const noexcept
    {
        return 0x2000 + 1;
    }
    
    void Mapper2::Load(const std::vector<char>& data, int version)
    {
        if (data.size() != GetSaveFileSize(version))
            return;
        
        auto pointer = data.data();

        pointer = UnsafeRead(pointer, m_select);
        
        for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
        {
            pointer = UnsafeRead(pointer, m_CHR_ram[i]);
        }
    }
}
