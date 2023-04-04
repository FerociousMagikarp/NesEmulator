#pragma once


#include <cstdint>
namespace nes
{
    class Cartridge;

    class Mapper
    {
    public:
        Mapper(Cartridge* cartridge) : m_cartridge(cartridge) {}
        virtual ~Mapper() = default;

        virtual std::uint8_t ReadPRG(std::uint16_t address) = 0;
        virtual void WritePRG(std::uint16_t address, std::uint8_t value) = 0;
        virtual std::uint8_t ReadCHR(std::uint16_t address) = 0;
        virtual void WriteCHR(std::uint16_t address, std::uint8_t value) = 0;

    protected:
        Cartridge* m_cartridge;
    };
}
