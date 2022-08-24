#pragma once

#include <memory>
#include <vector>

#include "def.h"
#include "mappers/mapper.h"

namespace nes
{
    class Cartridge
    {
    public:
        Cartridge() noexcept;
        ~Cartridge();

        bool LoadFromFile(const char *path);
        
        inline const std::vector<byte>& GetPRGRom() { return m_PRG_Rom; }
        inline const std::vector<byte>& GetCHRRom() { return m_CHR_Rom; }
        inline const std::unique_ptr<Mapper>& GetMapper() { return m_mapper; }
        inline byte ReadPRGRam(uint16 address) 
        {
            if(m_PRG_ram) return m_PRG_ram[address];
            else           return 0;
        }
        inline void WritePRGRam(uint16 address, byte value)
        {
            if (m_PRG_ram) m_PRG_ram[address] = value;
        }

    private:
        void CreateMapper();

    private:
        enum SpecialFlag
        {
            MirroringVertical              = (1 << 0),
            CartridgeContainsBatteryBacked = (1 << 1),
            Trainer                        = (1 << 2),
            IgnoreMirroringControl         = (1 << 3),
            VSUnisystem                    = (1 << 4),
            PlayChoicce10                  = (1 << 5),
            TVSystemNTSC                   = (1 << 6),
            TVSystemPal                    = (1 << 7),
            PRGRamPresent                  = (1 << 8),
            BoardHasBusConflict            = (1 << 9)
        };
        unsigned int m_special_flags = 0;
        unsigned int m_mapper_id = 0;

        std::unique_ptr<byte[]> m_trainer = nullptr;
        std::unique_ptr<Mapper> m_mapper = nullptr;
        std::unique_ptr<byte[]> m_PRG_ram = nullptr;
        std::vector<byte> m_PRG_Rom;
        std::vector<byte> m_CHR_Rom;
    };
}
