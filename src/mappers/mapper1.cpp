#include "mappers/mapper1.h"
#include "cartridge.h"
#include "def.h"

namespace nes
{
    constexpr std::size_t CHR_RAM_SIZE = 0x2000;

    Mapper1::Mapper1(Cartridge* cartridge) : Mapper(cartridge)
    {
        m_first_bank_PRG = 0x0000;
        m_last_bank_PRG = static_cast<std::uint32_t>(cartridge->GetPRGRom().size()) - 0x4000;
        m_CHR_bank_low = 0x0000;
        m_CHR_bank_high = 0x1000;
        if (cartridge->GetCHRRom().empty())
            m_CHR_Ram = std::make_unique<std::uint8_t[]>(CHR_RAM_SIZE);
    }

    std::uint8_t Mapper1::ReadCHR(std::uint16_t address)
    {
        if (m_CHR_Ram)
            return m_CHR_Ram[address & 0x1fff];
        if (address < 0x1000)
            return m_cartridge->GetCHRRom()[m_CHR_bank_low + address];
        else
            return m_cartridge->GetCHRRom()[m_CHR_bank_high + (address & 0x0fff)];
    }

    void Mapper1::WriteCHR(std::uint16_t address, std::uint8_t value)
    {
        if (m_CHR_Ram)
            m_CHR_Ram[address & 0x1fff] = value;
    }

    std::uint8_t Mapper1::ReadPRG(std::uint16_t address)
    {
        if (address < 0xc000)
            return m_cartridge->GetPRGRom()[m_first_bank_PRG + (address & 0x3fff)];
        else
            return m_cartridge->GetPRGRom()[m_last_bank_PRG + (address & 0x3fff)];
    }

    void Mapper1::WritePRG(std::uint16_t address, std::uint8_t value)
    {
        if ((value & 0x80) != 0)
            Reset();
        else
            PushValueToSR(address, value & 0x01);
    }

    void Mapper1::Reset()
    {
        m_control |= 0x0c;
        m_shift_register = 0x10;
        SwitchPRGBank();
    }

    void Mapper1::PushValueToSR(std::uint16_t addr, std::uint8_t val)
    {
        if (m_shift_register & 0x01) // 已经移4位了，这是最后一位了
        {
            m_shift_register = (val << 4) | (m_shift_register >> 1);
            switch ((addr >> 13) & 0x03)
            {
                case 0x00: // Control
                    m_control = m_shift_register; // TODO：这里末两位是mirror类型，先不管了
                    m_on_morroring_changed(GetMirroringType(m_control & 0x03));
                    SwitchPRGBank();
                    SwitchCHRBank();
                    break;
                case 0x01: // CHR bank 0
                    m_CHR_bank0 = m_shift_register;
                    SwitchCHRBank();
                    break;
                case 0x02: // CHR bank 1
                    m_CHR_bank1 = m_shift_register;
                    SwitchCHRBank();
                    break;
                case 0x03: // PRG bank
                    m_PRG_bank = m_shift_register;
                    SwitchPRGBank();
                    break;
                default: // 没有这种情况
                    break;
            }

            m_shift_register = 0x10;
        }
        else
        {
            m_shift_register >>= 1;
            m_shift_register |= val << 4;
        }
    }

    void Mapper1::SwitchPRGBank()
    {
        switch((m_control >> 2) & 0x03)
        {
            case 0:  // 32KB模式，忽略最后一位
            case 1:
                m_first_bank_PRG = static_cast<std::uint32_t>(m_PRG_bank & 0x0e) << 14;
                m_last_bank_PRG = m_first_bank_PRG + 0x4000;
                break;
            case 2:  // fix first bank at $8000 and switch 16 KB bank at $C000
                m_first_bank_PRG = 0;
                m_last_bank_PRG = static_cast<std::uint32_t>(m_PRG_bank & 0x0f) << 14;
                break;
            case 3: // fix last bank at $C000 and switch 16 KB bank at $8000
                m_first_bank_PRG = static_cast<std::uint32_t>(m_PRG_bank & 0x0f) << 14;
                m_last_bank_PRG = static_cast<std::uint32_t>(m_cartridge->GetPRGRom().size()) - 0x4000;
                break;
            default: // 没有这种情况
                break;
        }
    }

    void Mapper1::SwitchCHRBank()
    {
        if (m_control & 0x10) // switch two separate 4 KB banks
        {
            m_CHR_bank_low = static_cast<std::uint32_t>(m_CHR_bank0) << 12;
            m_CHR_bank_high = static_cast<std::uint32_t>(m_CHR_bank1) << 12;
        }
        else  // switch 8 KB at a time
        {
            m_CHR_bank_low = static_cast<std::uint32_t>(m_CHR_bank0 & ~0x01) << 12;
            m_CHR_bank_high = m_CHR_bank_low + 0x1000;
        }
    }

    MirroringType Mapper1::GetMirroringType(std::uint8_t mirroring)
    {
        // 根据 Control 寄存器的末两位决定镜像类型
        switch (mirroring & 0x03)
        {
            case 0:
                return MirroringType::OneScreenLowerBank;
            case 1:
                return MirroringType::OneScreenUpperBank;
            case 2:
                return MirroringType::Vertical;
            case 3:
                return MirroringType::Horizontal;
            default: // 不会有这种情况
                break;
        }
        return MirroringType::Horizontal; // 不会有这种情况
    }

    std::vector<char> Mapper1::Save() const
    {
        std::vector<char> res(GetSaveFileSize(SAVE_VERSION));
        auto pointer = res.data();

        pointer = UnsafeWrite(pointer, m_shift_register);
        pointer = UnsafeWrite(pointer, m_control);
        pointer = UnsafeWrite(pointer, m_CHR_bank0);
        pointer = UnsafeWrite(pointer, m_CHR_bank1);
        pointer = UnsafeWrite(pointer, m_PRG_bank);
        pointer = UnsafeWrite(pointer, m_first_bank_PRG);
        pointer = UnsafeWrite(pointer, m_last_bank_PRG);
        pointer = UnsafeWrite(pointer, m_CHR_bank_low);
        pointer = UnsafeWrite(pointer, m_CHR_bank_high);

        if (m_CHR_Ram != nullptr)
        {
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeWrite(pointer, m_CHR_Ram[i]);
        }

        return res;
    }

    std::size_t Mapper1::GetSaveFileSize(int version) const noexcept
    {
        return 68 + (m_CHR_Ram != nullptr ? CHR_RAM_SIZE : 0);
    }

    void Mapper1::Load(const std::vector<char>& data, int version)
    {
        if (data.size() != GetSaveFileSize(version))
            return;
        
        auto pointer = data.data();

        pointer = UnsafeRead(pointer, m_shift_register);
        pointer = UnsafeRead(pointer, m_control);
        pointer = UnsafeRead(pointer, m_CHR_bank0);
        pointer = UnsafeRead(pointer, m_CHR_bank1);
        pointer = UnsafeRead(pointer, m_PRG_bank);
        pointer = UnsafeRead(pointer, m_first_bank_PRG);
        pointer = UnsafeRead(pointer, m_last_bank_PRG);
        pointer = UnsafeRead(pointer, m_CHR_bank_low);
        pointer = UnsafeRead(pointer, m_CHR_bank_high);

        if (m_CHR_Ram != nullptr)
        {
            for (std::size_t i = 0; i < CHR_RAM_SIZE; i++)
                pointer = UnsafeRead(pointer, m_CHR_Ram[i]);
        }
    }
}
