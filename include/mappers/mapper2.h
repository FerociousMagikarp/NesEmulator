#pragma once

#include "mappers/mapper.h"
#include <cstdint>
#include <memory>

namespace nes
{
    // UxROM
    class Mapper2 : public Mapper
    {
    public:
        Mapper2(Cartridge* cartridge);
        ~Mapper2() = default;

        std::uint8_t ReadPRG(std::uint16_t address) override;
        void WritePRG(std::uint16_t address, std::uint8_t value) override;
        std::uint8_t ReadCHR(std::uint16_t address) override;
        void WriteCHR(std::uint16_t address, std::uint8_t value) override;
    
    private:
        std::unique_ptr<std::uint8_t[]> m_CHR_ram = nullptr;
        std::uint8_t m_select = 0;
    };
}
