#pragma once

#include "mappers/mapper.h"
#include <cstdint>
#include <memory>

namespace nes
{
    class Mapper0 : public Mapper
    {
    public:
        Mapper0(Cartridge* cartridge);
        ~Mapper0() = default;

        std::uint8_t ReadPRG(std::uint16_t address) override;
        void WritePRG(std::uint16_t address, std::uint8_t value) override;
        std::uint8_t ReadCHR(std::uint16_t address) override;
        void WriteCHR(std::uint16_t address, std::uint8_t value) override;
    
    private:
        std::unique_ptr<std::uint8_t[]> m_CHR_ram = nullptr;
    };
}
