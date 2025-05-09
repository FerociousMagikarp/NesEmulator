#include "mappers/mapper3.h"
#include "cartridge.h"

namespace nes
{
    constexpr std::size_t CHR_RAM_SIZE = 0x2000;

    Mapper3::Mapper3(Cartridge* cartridge) : Mapper(cartridge)
    {
        if (cartridge->GetCHRRom().size() == 0)
        {
            m_CHR_ram = std::make_unique<std::uint8_t[]>(CHR_RAM_SIZE);
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

    std::vector<char> Mapper3::Save() const
    {
        std::vector<char> res(GetSaveFileSize(SAVE_VERSION));

        auto pointer = res.data();

        pointer = UnsafeWrite(pointer, m_CHR_bank);

        if (m_CHR_ram != nullptr)
        {
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeWrite(pointer, m_CHR_ram[i]);
        }

        return res;
    }

    std::size_t Mapper3::GetSaveFileSize(int version) const noexcept
    {
        return 4 + (m_CHR_ram == nullptr ? 0 : CHR_RAM_SIZE);
    }
    
    void Mapper3::Load(const std::vector<char>& data, int version)
    {
        if (data.size() != GetSaveFileSize(version))
            return;
        
        auto pointer = data.data();

        pointer = UnsafeRead(pointer, m_CHR_bank);

        if (m_CHR_ram != nullptr)
        {
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeRead(pointer, m_CHR_ram[i]);
        }
    }
}
