#include "cartridge.h"
#include <array>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <stdio.h>

namespace nes
{
    struct NesFileHead
    {
        char identification[4];
        uint8 PRG_ROM_size;
        uint8 CHR_ROM_size;
        uint8 flags6;
        uint8 flags7;
        uint8 flags8;
        uint8 flags9;
        uint8 flags10;
        uint8 unused[5];
    };

    Cartridge::Cartridge()
    {

    }

    Cartridge::~Cartridge()
    {

    }

    bool Cartridge::LoadFromFile(const char* path)
    {
        std::ifstream ifstream;
        ifstream.open(path, std::ios::in | std::ios::binary);

        if (!ifstream.is_open())
        {
            std::cout << "No such file : " << path << std::endl;
            return false;
        }

        NesFileHead file_head;
        if (!ifstream.read(reinterpret_cast<char*>(&file_head), sizeof(file_head)))
        {
            std::cout << "Read Head Failed." << std::endl;
            goto analyze_error;
        }

        // 检查NES标志位
        if (file_head.identification[0] != 'N' || file_head.identification[1] != 'E' || file_head.identification[2] != 'S' || file_head.identification[3] != '\x1a')
        {
            std::cout << "Identification wrong" << std::endl;
            goto analyze_error;
        }
        // 是否是nes2.0（还没写2.0，所以先return了）
        if ((file_head.flags7 & 0xc0) == 0x80)
        {
            std::cout << "NES2.0" << std::endl;
            goto analyze_error;
        }
        // 设置一下标记位
        m_special_flags |= (file_head.flags6 & 0x0f);
        m_special_flags |= (file_head.flags7 & 0x03) << 4;
        // TODO : flags9 flags10

        // 设置mapper
        m_mapper_id = ((file_head.flags6 & 0xf0) >> 4) | (file_head.flags7 & 0xf0);

        // 读取trainer
        if ((m_special_flags & Trainer) != 0)
        {
            constexpr int trainer_size = 512;
            m_trainer = std::make_unique<std::array<byte, trainer_size>>();
            // 这写法十分诡异
            if (!ifstream.read(reinterpret_cast<char*>(&m_trainer->at(0)), trainer_size))
            {
                std::cout << "Read Trainer Failed." << std::endl;
                goto analyze_error;
            }
        }
        // 读取PRG_ROM
        m_PRG_Rom.resize(0x4000 * file_head.PRG_ROM_size);
        if (!ifstream.read(reinterpret_cast<char*>(&m_PRG_Rom[0]), m_PRG_Rom.size()))
        {
            std::cout << "Read PRG ROM Failed." << std::endl;
            goto analyze_error;
        }
        // 读取CHR_ROm
        if (file_head.CHR_ROM_size > 0)
        {
            m_CHR_Rom.resize(0x2000 * file_head.CHR_ROM_size);
            if (!ifstream.read(reinterpret_cast<char*>(&m_CHR_Rom[0]), m_CHR_Rom.size()))
            {
                std::cout << "Read CHR ROM Failed." << std::endl;
                goto analyze_error;
            }
        }

        // TODO : Play Choice
        
        ifstream.close();
        return true;
    analyze_error:
        ifstream.close();
        return false;
    }
}
