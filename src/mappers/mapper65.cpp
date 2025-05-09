#include "mappers/mapper65.h"
#include "cartridge.h"

namespace nes
{
    Mapper65::Mapper65(Cartridge* cartridge) : Mapper(cartridge)
    {
        m_PRG_bank[0] = 0x00;
        m_PRG_bank[1] = 0x01;
        m_PRG_bank[2] = 0x3e;
        m_PRG_bank[3] = 0x3f;
    }

    std::uint8_t Mapper65::ReadCHR(std::uint16_t address)
    {
        auto add = (static_cast<std::size_t>(m_CHR_bank[(address >> 10) & 0x07]) << 10) | (address & 0x03ff);
        add = add % m_cartridge->GetCHRRom().size(); // 这样更安全
        return m_cartridge->GetCHRRom()[add];
    }

    void Mapper65::WriteCHR(std::uint16_t address, std::uint8_t value)
    {
    }

    std::uint8_t Mapper65::ReadPRG(std::uint16_t address)
    {
        auto add = (static_cast<std::size_t>(m_PRG_bank[(address >> 13) & 0x03]) << 13) | (address & 0x1fff);
        add = add % m_cartridge->GetPRGRom().size(); // 这样更安全
        return m_cartridge->GetPRGRom()[add];
    }

    void Mapper65::WritePRG(std::uint16_t address, std::uint8_t value)
    {
        // PRG
        if (address == 0x8000)
        {
            m_Write_0x8000 = value;
            if (!m_PRG_layout)
            {
                m_PRG_bank[0] = value;
                m_PRG_bank[2] = 0x3e;
            }
            else
            {
                m_PRG_bank[0] = 0x3e;
                m_PRG_bank[2] = value;
            }
        }
        else if (address == 0xa000)
            m_PRG_bank[1] = value;
        else if (address == 0x9000)
        {
            if ((value & 0x80) == 0)
            {
                m_PRG_layout = false;
                m_PRG_bank[0] = m_Write_0x8000;
                m_PRG_bank[2] = 0x3e;
            }
            else
            {
                m_PRG_layout = true;
                m_PRG_bank[0] = 0x3e;
                m_PRG_bank[2] = m_Write_0x8000;
            }
        }

        // CHR
        else if (address >= 0xb000 && address <= 0xb007)
            m_CHR_bank[static_cast<std::size_t>(address & 0x07)] = value;

        else if (address == 0x9001)
        {
            switch (value >> 6)
            {
                case 0:
                    m_on_morroring_changed(MirroringType::Vertical);
                    break;
                case 2:
                    m_on_morroring_changed(MirroringType::Horizontal);
                    break;
                default: // 那个 1scA 是什么意思。。。没看懂
                    break;
            }
        }

        // IRQ
        else if (address == 0x9003)
            m_IRQ_enable = (value & 0x80) != 0;
        else if (address == 0x9004)
            m_IRQ_tick = m_IRQ_counter;
        else if (address == 0x9005)
            m_IRQ_counter = static_cast<std::uint16_t>(value) << 8;
        else if (address == 0x9006)
            m_IRQ_counter |= value;
    }

    void Mapper65::CPUCycleCounter()
    {
        if (m_IRQ_counter > 0 && m_IRQ_enable)
        {
            m_IRQ_counter--;
            if (m_IRQ_counter == 0)
            {
                m_trigger_IRQ();
                m_IRQ_enable = false;
            }
        }
    }

    std::vector<char> Mapper65::Save() const
    {
        std::vector<char> res(GetSaveFileSize(SAVE_VERSION));

        auto pointer = res.data();

        pointer = UnsafeWrite(pointer, m_IRQ_counter);
        pointer = UnsafeWrite(pointer, m_IRQ_tick);
        pointer = UnsafeWrite(pointer, m_IRQ_enable);
        pointer = UnsafeWrite(pointer, m_PRG_layout);
        pointer = UnsafeWrite(pointer, m_PRG_bank);
        pointer = UnsafeWrite(pointer, m_CHR_bank);
        pointer = UnsafeWrite(pointer, m_Write_0x8000);

        return res;
    }

    std::size_t Mapper65::GetSaveFileSize(int version) const noexcept
    {
        return 20;
    }
    
    void Mapper65::Load(const std::vector<char>& data, int version)
    {
        if (data.size() != GetSaveFileSize(version))
            return;
        
        auto pointer = data.data();

        pointer = UnsafeRead(pointer, m_IRQ_counter);
        pointer = UnsafeRead(pointer, m_IRQ_tick);
        pointer = UnsafeRead(pointer, m_IRQ_enable);
        pointer = UnsafeRead(pointer, m_PRG_layout);
        pointer = UnsafeRead(pointer, m_PRG_bank);
        pointer = UnsafeRead(pointer, m_CHR_bank);
        pointer = UnsafeRead(pointer, m_Write_0x8000);
    }
}
