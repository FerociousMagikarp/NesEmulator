#pragma once

#include "mappers/mapper.h"
#include <memory>
#include <array>

namespace nes
{
    // MMC3
    class Mapper4 : public Mapper
    {
    public:
        Mapper4(Cartridge* cartridge);
        ~Mapper4() = default;

        std::uint8_t ReadPRG(std::uint16_t address) override;
        void WritePRG(std::uint16_t address, std::uint8_t value) override;
        std::uint8_t ReadCHR(std::uint16_t address) override;
        void WriteCHR(std::uint16_t address, std::uint8_t value) override;
    
        bool HasExtendPRGRam() const override { return true; }
        void ReduceIRQCounter() override;
        void SetTriggerIRQCallback(std::function<void(void)>&& callback) override { m_trigger_IRQ = std::move(callback); }
        void BankSelect(std::uint8_t val);

        // 存档使用的函数
        std::vector<char> Save() const override;
        std::size_t GetSaveFileSize(int version) const noexcept override;
        void Load(const std::vector<char>& data, int version) override;

    private:
        std::unique_ptr<std::uint8_t[]> m_CHR_ram = nullptr;
        std::uint8_t m_bank_select = 0;
        bool m_IRQ_enabled = false;
        int m_IRQ_latch = 0;
        int m_IRQ_counter = 0;

        std::array<std::uint32_t, 4> m_PRG_bank{};
        std::array<std::uint32_t, 8> m_CHR_bank{};

        std::function<void(void)> m_trigger_IRQ;
    };
}
