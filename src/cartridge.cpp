#include "cartridge.h"
#include <fstream>
#include <iostream>
#include <memory>
#include "mappers/mapper_headers.h"


namespace nes
{
    struct NesFileHead
    {
        std::int8_t  identification[4];
        std::uint8_t PRG_ROM_size;
        std::uint8_t CHR_ROM_size;
        std::uint8_t flags6;
        std::uint8_t flags7;
        std::uint8_t flags8;
        std::uint8_t flags9;
        std::uint8_t flags10;
        std::uint8_t unused[5];
    };

    bool Cartridge::LoadFromFile(const std::string_view path)
    {
        m_file_name = path;

        std::ifstream ifstream;
        ifstream.open(path.data(), std::ios::in | std::ios::binary);

        if (!ifstream.is_open())
        {
            std::cout << "No such file : " << path << std::endl;
            return false;
        }

        bool load_result = false;
        NesFileHead file_head;

        do
        {
            if (!ifstream.read(reinterpret_cast<char*>(&file_head), sizeof(file_head)))
            {
                std::cout << "Read Head Failed." << std::endl;
                break;
            }

            // 检查NES标志位
            if (file_head.identification[0] != 'N' || file_head.identification[1] != 'E' || file_head.identification[2] != 'S' || file_head.identification[3] != '\x1a')
            {
                std::cout << "Identification wrong" << std::endl;
                break;
            }
            // 是否是nes2.0（还没写2.0，所以先return了）
            if ((file_head.flags7 & 0xc0) == 0x80)
            {
                std::cout << "NES2.0" << std::endl;
                break;
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
                m_trainer = std::make_unique<std::uint8_t[]>(trainer_size);
                if (!ifstream.read(reinterpret_cast<char*>(m_trainer.get()), trainer_size))
                {
                    std::cout << "Read Trainer Failed." << std::endl;
                    break;
                }
            }
            // 读取PRG_ROM
            m_PRG_Rom.resize(0x4000ull * file_head.PRG_ROM_size);
            if (!ifstream.read(reinterpret_cast<char*>(&m_PRG_Rom[0]), m_PRG_Rom.size()))
            {
                std::cout << "Read PRG ROM Failed." << std::endl;
                break;
            }
            // 读取CHR_ROM
            if (file_head.CHR_ROM_size > 0)
            {
                m_CHR_Rom.resize(0x2000ull * file_head.CHR_ROM_size);
                if (!ifstream.read(reinterpret_cast<char*>(&m_CHR_Rom[0]), m_CHR_Rom.size()))
                {
                    std::cout << "Read CHR ROM Failed." << std::endl;
                    break;
                }
            }

            // 输出rom大小
            std::cout << "PRG Rom size : " << static_cast<std::uint32_t>(file_head.PRG_ROM_size) * 16 << "KB"
                << ", CHR Rom size : " << static_cast<std::uint32_t>(file_head.CHR_ROM_size) * 8 << "KB" << "\n";
            // 输出mapper编号
            std::cout << "Mapper ID : " << m_mapper_id << "\n";

            constexpr int PRG_Ram_size = 0x2000;
            // 创建额外的RAM
            if (m_special_flags & CartridgeContainsBatteryBacked)
            {
                m_PRG_Ram = std::make_unique<std::uint8_t[]>(PRG_Ram_size);
            }

            // TODO : Play Choice
            
            // 创建Mapper
            if (!CreateMapper())
            {
                std::cout << "Unknown mapper : " << m_mapper_id << std::endl;
                break;
            }

            // 上面那个额外ram标记就跟闹着玩一样。如果mapper需要标记，但是上面没创建，就再创建一遍
            if (m_PRG_Ram == nullptr && m_mapper->HasExtendPRGRam())
            {
                m_PRG_Ram = std::make_unique<std::uint8_t[]>(PRG_Ram_size);
            }

            load_result = true;
        } while(false);

        ifstream.close();
        return load_result;
    }

    bool Cartridge::CreateMapper()
    {
        #define MAPPER_CASE(n) \
        case n: \
            m_mapper = std::make_unique<Mapper##n>(this); \
            break \

        switch (m_mapper_id)
        {
            MAPPER_CASE(0);
            MAPPER_CASE(1);
            MAPPER_CASE(2);
            MAPPER_CASE(3);
            MAPPER_CASE(4);
            MAPPER_CASE(65);
        default:
            return false;
        }

        #undef MAPPER_CASE

        return true;
    }
}
