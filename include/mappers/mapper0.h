#pragma once

#include "def.h"
#include "mappers/mapper.h"

namespace nes
{
    class Mapper0 : public Mapper
    {
    public:
        Mapper0(Cartridge* cartridge) : Mapper(cartridge) {}

        byte ReadPRG(uint16 address) override;
        void WritePRG(uint16 address, byte value) override;
        byte ReadCHR(uint16 address) override;
        void WriteCHR(uint16 address, byte value) override;
    };
}
