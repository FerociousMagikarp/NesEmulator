#include "mappers/mapper0.h"
#include "cartridge.h"
#include <memory>

namespace nes
{
    constexpr std::size_t CHR_RAM_SIZE = 0x2000;

    Mapper0::Mapper0(Cartridge* cartridge) : Mapper(cartridge)
    {
        if (cartridge->GetCHRRom().size() == 0)
        {
            m_CHR_ram = std::make_unique<std::uint8_t[]>(CHR_RAM_SIZE);
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

    std::vector<char> Mapper0::Save() const
    {
        std::vector<char> res(GetSaveFileSize(SAVE_VERSION));

        if (m_CHR_ram != nullptr)
        {
            auto pointer = res.data();
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeWrite(pointer, m_CHR_ram[i]);
        }

        return res;
    }

    std::size_t Mapper0::GetSaveFileSize(int version) const noexcept
    {
        return m_CHR_ram == nullptr ? 1 : CHR_RAM_SIZE;
    }

    void Mapper0::Load(const std::vector<char>& data, int version)
    {
        if (m_CHR_ram != nullptr)
        {
            if (data.size() != GetSaveFileSize(version))
                return;

            auto pointer = data.data();
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeRead(pointer, m_CHR_ram[i]);
        }
    }
}
