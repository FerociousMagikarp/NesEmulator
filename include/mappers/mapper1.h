#pragma once

#include "mappers/mapper.h"
#include <cstdint>
#include <memory>

namespace nes
{
    class Mapper1 : public Mapper
    {
    public:
        Mapper1(Cartridge* cartridge);
        ~Mapper1() = default;

        std::uint8_t ReadPRG(std::uint16_t address) override;
        void WritePRG(std::uint16_t address, std::uint8_t value) override;
        std::uint8_t ReadCHR(std::uint16_t address) override;
        void WriteCHR(std::uint16_t address, std::uint8_t value) override;

        bool HasExtendPRGRam() const override { return true; }

        void Reset();
        void PushValueToSR(std::uint16_t addr, std::uint8_t val);
        void SwitchPRGBank();
        void SwitchCHRBank();
        MirroringType GetMirroringType(std::uint8_t mirroring);

        // 存档使用的函数
        std::vector<char> Save() const override;
        std::size_t GetSaveFileSize(int version) const noexcept override;
        void Load(const std::vector<char>& data, int version) override;
    
    private:
        std::uint8_t m_shift_register = 0x10;
        std::uint8_t m_control = 0x0c;
        std::uint8_t m_CHR_bank0 = 0;
        std::uint8_t m_CHR_bank1 = 0;
        std::uint8_t m_PRG_bank = 0;

        std::uint32_t m_first_bank_PRG = 0;
        std::uint32_t m_last_bank_PRG = 0;
        std::uint32_t m_CHR_bank_low = 0;
        std::uint32_t m_CHR_bank_high = 0;

        std::unique_ptr<std::uint8_t[]> m_CHR_Ram = nullptr;
    };
}
