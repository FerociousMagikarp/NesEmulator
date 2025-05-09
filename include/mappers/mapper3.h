#pragma once

#include "mappers/mapper.h"
#include <memory>

namespace nes
{
    class Mapper3 : public Mapper
    {
    public:
        Mapper3(Cartridge* cartridge);
        ~Mapper3() = default;

        std::uint8_t ReadPRG(std::uint16_t address) override;
        void WritePRG(std::uint16_t address, std::uint8_t value) override;
        std::uint8_t ReadCHR(std::uint16_t address) override;
        void WriteCHR(std::uint16_t address, std::uint8_t value) override;
    
        // 存档使用的函数
        std::vector<char> Save() const override;
        std::size_t GetSaveFileSize(int version) const noexcept override;
        void Load(const std::vector<char>& data, int version) override;

    private:
        std::unique_ptr<std::uint8_t[]> m_CHR_ram = nullptr;
        std::uint32_t m_CHR_bank = 0;
    };
}
