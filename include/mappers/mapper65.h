#pragma once

#include "mappers/mapper.h"
#include <array>

namespace nes
{
    class Mapper65 : public Mapper
    {
    public:
        Mapper65(Cartridge* cartridge);
        ~Mapper65() = default;

        std::uint8_t ReadPRG(std::uint16_t address) override;
        void WritePRG(std::uint16_t address, std::uint8_t value) override;
        std::uint8_t ReadCHR(std::uint16_t address) override;
        void WriteCHR(std::uint16_t address, std::uint8_t value) override;
    
        void SetTriggerIRQCallback(std::function<void(void)>&& callback) override { m_trigger_IRQ = std::move(callback); }
        void CPUCycleCounter() override;
    
        // 存档使用的函数
        std::vector<char> Save() const override;
        std::size_t GetSaveFileSize(int version) const noexcept override;
        void Load(const std::vector<char>& data, int version) override;

    private:
        std::uint16_t m_IRQ_counter = 0;
        std::uint16_t m_IRQ_tick = 0;
        bool m_IRQ_enable = false;
        bool m_PRG_layout = false;
        std::array<std::uint8_t, 4> m_PRG_bank{};
        std::array<std::uint8_t, 8> m_CHR_bank{};
        std::uint8_t m_Write_0x8000 = 0;

        std::function<void(void)> m_trigger_IRQ;
    };
}
