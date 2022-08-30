#pragma once

#include "def.h"
#include "mappers/mapper.h"
#include <memory>

namespace nes
{
    class Mapper0 : public Mapper
    {
    public:
        Mapper0(Cartridge* cartridge);
        ~Mapper0() = default;

        byte ReadPRG(uint16 address) override;
        void WritePRG(uint16 address, byte value) override;
        byte ReadCHR(uint16 address) override;
        void WriteCHR(uint16 address, byte value) override;
    
    private:
        std::unique_ptr<byte[]> m_CHR_ram = nullptr;
    };
}
