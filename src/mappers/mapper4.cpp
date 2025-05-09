#include "mappers/mapper4.h"
#include "cartridge.h"

namespace nes
{
    constexpr std::size_t CHR_RAM_SIZE = 0x2000;

    Mapper4::Mapper4(Cartridge* cartridge) : Mapper(cartridge)
    {
        if (cartridge->GetCHRRom().empty())
            m_CHR_ram = std::make_unique<std::uint8_t[]>(CHR_RAM_SIZE);
        m_PRG_bank[3] = static_cast<std::uint32_t>(cartridge->GetPRGRom().size()) - 0x2000;
        m_CHR_bank[0] = 0;
        m_CHR_bank[1] = 0x400;
    }

    std::uint8_t Mapper4::ReadCHR(std::uint16_t address)
    {
        if (m_CHR_ram != nullptr)
            return m_CHR_ram[address];
        return m_cartridge->GetCHRRom()[m_CHR_bank[(address >> 10) & 0x07] + (address & 0x03ff)];
    }

    void Mapper4::WriteCHR(std::uint16_t address, std::uint8_t value)
    {
        if (m_CHR_ram != nullptr)
            m_CHR_ram[address] = value;
    }

    std::uint8_t Mapper4::ReadPRG(std::uint16_t address)
    {
        return m_cartridge->GetPRGRom()[m_PRG_bank[(address >> 13) & 0x03] + (address & 0x1fff)];
    }

    void Mapper4::WritePRG(std::uint16_t address, std::uint8_t value)
    {
        switch ((address >> 13) & 0x03)
        {
            case 0x00: // 偶数 bank select，奇数 bank data
                if ((address & 1) == 0)
                {
                    m_bank_select = value;
                }
                else
                {
                    BankSelect(value);
                }
                break;
            case 0x01: // 偶数镜像，奇数PRG RAM protect
                if ((address & 1) == 0)
                {
                    if ((value & 0x01) == 0x01)
                        m_on_morroring_changed(MirroringType::Horizontal);
                    else
                        m_on_morroring_changed(MirroringType::Vertical);
                }
                else
                {
                    // TODO
                }
                break;
            case 0x02: // 偶数IRQ latch，奇数IRQ reload
                if ((address & 1) == 0)
                    m_IRQ_latch = value;
                else
                    m_IRQ_counter = 0;
                break;
            case 0x03: // 偶数IRQ disable，奇数IRQ enable
                m_IRQ_enabled = (address & 1) != 0;
                break;
            default: // 没有这种情况
                break;
        }
    }

    void Mapper4::ReduceIRQCounter()
    {
        if (m_IRQ_counter == 0)
        {
            m_IRQ_counter = m_IRQ_latch;
        }
        else
        {
            if (--m_IRQ_counter == 0 && m_IRQ_enabled)
                m_trigger_IRQ();
        }
    }

    void Mapper4::BankSelect(std::uint8_t val)
    {
        switch (m_bank_select & 0x07)
        {
            case 0: // R0: Select 2 KB CHR bank at PPU $0000-$07FF (or $1000-$17FF)
                if ((m_bank_select & 0x80) == 0)
                {
                    m_CHR_bank[0] = static_cast<std::uint32_t>(val & ~0x01) << 10;
                    m_CHR_bank[1] = m_CHR_bank[0] + 0x400;
                }
                else
                {
                    m_CHR_bank[4] = static_cast<std::uint32_t>(val & ~0x01) << 10;
                    m_CHR_bank[5] = m_CHR_bank[4] + 0x400;
                }
                break;
            case 1: // R1: Select 2 KB CHR bank at PPU $0800-$0FFF (or $1800-$1FFF)
                if ((m_bank_select & 0x80) == 0)
                {
                    m_CHR_bank[2] = static_cast<std::uint32_t>(val & ~0x01) << 10;
                    m_CHR_bank[3] = m_CHR_bank[2] + 0x400;
                }
                else
                {
                    m_CHR_bank[6] = static_cast<std::uint32_t>(val & ~0x01) << 10;
                    m_CHR_bank[7] = m_CHR_bank[6] + 0x400;
                }
                break;
            case 2: // R2: Select 1 KB CHR bank at PPU $1000-$13FF (or $0000-$03FF)
                if ((m_bank_select & 0x80) == 0)
                    m_CHR_bank[4] = static_cast<std::uint32_t>(val) << 10;
                else
                    m_CHR_bank[0] = static_cast<std::uint32_t>(val) << 10;
                break;
            case 3: // R3: Select 1 KB CHR bank at PPU $1400-$17FF (or $0400-$07FF)
                if ((m_bank_select & 0x80) == 0)
                    m_CHR_bank[5] = static_cast<std::uint32_t>(val) << 10;
                else
                    m_CHR_bank[1] = static_cast<std::uint32_t>(val) << 10;
                break;
            case 4: // R4: Select 1 KB CHR bank at PPU $1800-$1BFF (or $0800-$0BFF)
                if ((m_bank_select & 0x80) == 0)
                    m_CHR_bank[6] = static_cast<std::uint32_t>(val) << 10;
                else
                    m_CHR_bank[2] = static_cast<std::uint32_t>(val) << 10;
                break;
            case 5: // R5: Select 1 KB CHR bank at PPU $1C00-$1FFF (or $0C00-$0FFF)
                if ((m_bank_select & 0x80) == 0)
                    m_CHR_bank[7] = static_cast<std::uint32_t>(val) << 10;
                else
                    m_CHR_bank[3] = static_cast<std::uint32_t>(val) << 10;
                break;
            case 6: // R6: Select 8 KB PRG ROM bank at $8000-$9FFF (or $C000-$DFFF)
                if (m_bank_select & 0x40)
                {
                    auto size = static_cast<std::uint32_t>(m_cartridge->GetPRGRom().size());
                    m_PRG_bank[0] = size - 0x4000;
                    m_PRG_bank[2] = static_cast<std::uint32_t>(val & 0x3f) % (size >> 13) << 13;
                }
                else
                {
                    auto size = static_cast<std::uint32_t>(m_cartridge->GetPRGRom().size());
                    m_PRG_bank[0] = static_cast<std::uint32_t>(val & 0x3f) % (size >> 13) << 13;
                    m_PRG_bank[2] = size - 0x4000;
                }
                break;
            case 7: // R7: Select 8 KB PRG ROM bank at $A000-$BFFF
                m_PRG_bank[1] = static_cast<std::uint32_t>(val & 0x3f) % (static_cast<std::uint32_t>(m_cartridge->GetPRGRom().size()) >> 13) << 13;
                break;
        }
    }

    std::vector<char> Mapper4::Save() const
    {
        std::vector<char> res(GetSaveFileSize(SAVE_VERSION));

        auto pointer = res.data();

        pointer = UnsafeWrite(pointer, m_bank_select);
        pointer = UnsafeWrite(pointer, m_IRQ_enabled);
        pointer = UnsafeWrite(pointer, m_IRQ_latch);
        pointer = UnsafeWrite(pointer, m_IRQ_counter);
        pointer = UnsafeWrite(pointer, m_PRG_bank);
        pointer = UnsafeWrite(pointer, m_CHR_bank);

        if (m_CHR_ram != nullptr)
        {
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeWrite(pointer, m_CHR_ram[i]);
        }

        return res;
    }

    std::size_t Mapper4::GetSaveFileSize(int version) const noexcept
    {
        return 60 + (m_CHR_ram == nullptr ? 0 : CHR_RAM_SIZE);
    }
    
    void Mapper4::Load(const std::vector<char>& data, int version)
    {
        if (data.size() != GetSaveFileSize(version))
            return;
        
        auto pointer = data.data();

        pointer = UnsafeRead(pointer, m_bank_select);
        pointer = UnsafeRead(pointer, m_IRQ_enabled);
        pointer = UnsafeRead(pointer, m_IRQ_latch);
        pointer = UnsafeRead(pointer, m_IRQ_counter);
        pointer = UnsafeRead(pointer, m_PRG_bank);
        pointer = UnsafeRead(pointer, m_CHR_bank);

        if (m_CHR_ram != nullptr)
        {
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeRead(pointer, m_CHR_ram[i]);
        }
    }
}
