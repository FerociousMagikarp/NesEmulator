#pragma once


#include "def.h"
#include <cstdint>
#include <functional>
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

        virtual bool HasExtendPRGRam() const { return false; }
        virtual void OnMirroringChanged(std::function<void(MirroringType)>&& callback) { m_on_morroring_changed = std::move(callback); }

    protected:
        Cartridge* m_cartridge;
        std::function<void(MirroringType)> m_on_morroring_changed;
    };
}
