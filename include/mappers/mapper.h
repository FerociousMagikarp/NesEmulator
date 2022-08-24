#pragma once

#include "def.h"

namespace nes
{
    class Cartridge;

    class Mapper
    {
    public:
        Mapper(Cartridge* cartridge) : m_cartridge(cartridge) {}
        virtual ~Mapper() = default;

        virtual byte ReadPRG(uint16 address) = 0;
        virtual void WritePRG(uint16 address, byte value) = 0;
        virtual byte ReadCHR(uint16 address) = 0;
        virtual void WriteCHR(uint16 address, byte value) = 0;

    protected:
        Cartridge* m_cartridge;
    };
}
