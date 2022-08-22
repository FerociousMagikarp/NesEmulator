#pragma once

#include <memory>
#include <array>
#include <vector>

namespace nes
{
    class Cartridge
    {
    public:
        Cartridge();
        ~Cartridge();

        bool LoadFromFile(const char *path);
        
        inline const std::vector<unsigned char>& GetPRGRom() { return m_PRG_Rom; }
        inline const std::vector<unsigned char>& GetCHRRom() { return m_CHR_Rom; }

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

        std::unique_ptr<std::array<unsigned char, 512>> m_trainer = nullptr;
        std::vector<unsigned char> m_PRG_Rom;
        std::vector<unsigned char> m_CHR_Rom;
    };
}
