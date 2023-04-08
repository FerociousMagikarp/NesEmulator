#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "mappers/mapper.h"

namespace nes
{
    class Cartridge
    {
    public:
        Cartridge() = default;
        ~Cartridge() = default;

        bool LoadFromFile(const char *path);
        
        inline const std::vector<std::uint8_t>& GetPRGRom() const { return m_PRG_Rom; }
        inline const std::vector<std::uint8_t>& GetCHRRom() const { return m_CHR_Rom; }
        inline const std::unique_ptr<Mapper>& GetMapper() { return m_mapper; }
        inline std::uint8_t ReadPRGRam(std::uint16_t address)
        {
            if(m_PRG_Ram)  return m_PRG_Ram[address];
            else           return 0;
        }
        inline void WritePRGRam(std::uint16_t address, std::uint8_t value)
        {
            if (m_PRG_Ram) m_PRG_Ram[address] = value;
        }

    private:
        bool CreateMapper();

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

        std::unique_ptr<std::uint8_t[]> m_trainer = nullptr;
        std::unique_ptr<Mapper> m_mapper = nullptr;
        std::unique_ptr<std::uint8_t[]> m_PRG_Ram = nullptr;
        std::vector<std::uint8_t> m_PRG_Rom;
        std::vector<std::uint8_t> m_CHR_Rom;
    };
}
