#include "emulator.h"
#include "cpu.h"
#include "ppu.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <memory>
#include <filesystem>
#include <fstream>

namespace nes
{
    NesEmulator::NesEmulator()
        : m_RAM(std::make_unique<std::uint8_t[]>(0x0800))
    {
        m_CPU.SetReadFunction([this](std::uint16_t addr)->std::uint8_t{ return MainBusRead(addr); });
        m_CPU.SetWriteFunction([this](std::uint16_t addr, std::uint8_t val)->void{ MainBusWrite(addr, val); });
        m_PPU.SetReadMapperCHRCallback([this](std::uint16_t addr)->std::uint8_t { return m_cartridge->GetMapper()->ReadCHR(addr); });
        m_PPU.SetWriteMapperCHRCallback([this](std::uint16_t addr, std::uint8_t val)->void{ m_cartridge->GetMapper()->WriteCHR(addr, val); });
        m_PPU.SetNMICallback([this]()->void{ m_CPU.Interrupt(CPU6502InterruptType::NMI); });
        m_APU.SetIRQCallback([this]()->void{ m_CPU.Interrupt(CPU6502InterruptType::IRQ); });
        m_APU.SetDMCReadCallback([this](std::uint16_t addr)->std::uint8_t{ return MainBusRead(addr); });
    }

    NesEmulator::~NesEmulator()
    {

    }

    void NesEmulator::Run(const bool& running)
    {
        m_CPU.Reset();
        m_PPU.Reset();
        m_APU.Reset();
        auto last_time = std::chrono::steady_clock::now();
        while (running)
        {
            auto current_time = std::chrono::steady_clock::now();
            auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time);
            double time_s = delta_time.count() / 1000.0;
            int step_n = static_cast<int>(time_s * NTSC_CPU_FREQUENCY);
            bool frame_changed = false;
            int step_tick = 0;
            while (step_tick++ < step_n)
            {
                m_PPU.Step();
                m_PPU.Step();
                m_PPU.Step();
                m_CPU.Step();
                m_APU.Step(); // APU自己在里面降频吧，因为三角波是CPU周期刷新的。

                auto PPU_frame = m_PPU.GetFrame();
                if (PPU_frame != m_frame)
                {
                    m_frame = PPU_frame;
                    frame_changed = true;
                    break;
                }
            }
            constexpr auto remainder = 1000000000ll % NTSC_CPU_FREQUENCY;
            constexpr auto quotient = 1000000000ll / NTSC_CPU_FREQUENCY;
            last_time += std::chrono::nanoseconds(step_tick * quotient + step_tick * remainder / NTSC_CPU_FREQUENCY);

            // 只有在一帧结束之后才会读取对应的快捷操作
            if (frame_changed)
            {
                auto op = m_operation.exchange(EmulatorOperation::None);
                switch (op)
                {
                case EmulatorOperation::None:
                    break;
                case EmulatorOperation::Save:
                    Save();
                    break;
                case EmulatorOperation::Load:
                    Load();
                    m_frame = m_PPU.GetFrame();
                    break;
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    }

    void NesEmulator::PutInCartridge(std::unique_ptr<Cartridge> cartridge)
    {
        m_cartridge = std::move(cartridge);

        // 先这么写，四屏模式以后再搞
        if (m_cartridge->IsMirroringVertical())
            m_PPU.SetMirrorType(MirroringType::Vertical);
        else
            m_PPU.SetMirrorType(MirroringType::Horizontal);

        m_cartridge->GetMapper()->OnMirroringChanged([this](MirroringType type)->void
        {
            m_PPU.SetMirrorType(type);
        });
        m_cartridge->GetMapper()->SetTriggerIRQCallback([this]()->void
        {
            m_CPU.Interrupt(CPU6502InterruptType::IRQ);
        });
        m_PPU.SetMapperReduceIRQCounterCallback([this]()->void
        {
            m_cartridge->GetMapper()->ReduceIRQCounter();
        });
    }

    std::uint8_t NesEmulator::MainBusRead(std::uint16_t address)
    {
        // 知乎上看到的高手使用位运算减少了if的分支判断
        switch (address >> 13)
        {
        case 0x00:  // 地址范围 : [0, 0x2000)
            return m_RAM[address & 0x07ff]; // 只有2K内存，剩下的全是镜像
        case 0x01:  // 地址范围 : [0x2000, 0x4000)
            return m_PPU.GetRegister(address & 0x2007);
        case 0x02:  // 地址范围 : [0x4000, 0x6000)
            if (address == 0x4015)
                return m_APU.ReadStatus();
            else if (address == 0x4016)
                return m_device->Read4016();
            else if (address == 0x4017)
                return m_device->Read4017();
            break;
        case 0x03:  // 地址范围 : [0x6000, 0x8000)
            return m_cartridge->ReadPRGRam(address & 0x1fff);
        case 0x04:  // 地址范围 : [0x8000, 0xA000)
        case 0x05:  // 地址范围 : [0xA000, 0xC000)
        case 0x06:  // 地址范围 : [0xC000, 0xE000)
        case 0x07:  // 地址范围 : [0xE000, 0x10000)
            return m_cartridge->GetMapper()->ReadPRG(address);
        default:
            break;
        }
        return 0;
    }

    void NesEmulator::MainBusWrite(std::uint16_t address, std::uint8_t value)
    {
        switch (address >> 13)
        {
        case 0x00:  // 地址范围 : [0, 0x2000)
            m_RAM[address & 0x07ff] = value; // 只有2K内存，剩下的全是镜像
            break;
        case 0x01:  // 地址范围 : [0x2000, 0x4000)
            m_PPU.SetRegister(address & 0x2007, value);
            break;
        case 0x02:  // 地址范围 : [0x4000, 0x6000)
            if (address <= 0x4013 || address == 0x4015 || address == 0x4017)  // 所有写APU的都算进去了
                m_APU.SetRegister(address, value);
            else if (address == 0x4014) // OAMDMA
            {
                m_CPU.SkipOAMDMACycle();
                m_PPU.OAMDMA(m_RAM.get() + ((value << 8) & 0x700));
            }
            if (address == 0x4016)
                m_device->Write4016(value);
            break;
        case 0x03:  // 地址范围 : [0x6000, 0x8000)
            m_cartridge->WritePRGRam(address & 0x1fff, value);
            break;
        case 0x04:  // 地址范围 : [0x8000, 0xA000)
        case 0x05:  // 地址范围 : [0xA000, 0xC000)
        case 0x06:  // 地址范围 : [0xC000, 0xE000)
        case 0x07:  // 地址范围 : [0xE000, 0x10000)
            m_cartridge->GetMapper()->WritePRG(address, value);
            break;
        default:
            break;
        }
    }

    void NesEmulator::SetOperation(EmulatorOperation operation)
    {
        m_operation.store(operation);
    }

    std::string NesEmulator::GetSavePath() const
    {
        if (!m_cartridge)
            return "";
        const std::string& file_name = m_cartridge->GetFileName();
        std::filesystem::path p(file_name);
        auto dir = p.parent_path();
        auto name = p.filename();
        name.replace_extension(".sav");
        auto total = dir / name;
        return total.string();
    }

    void NesEmulator::Save()
    {
        auto path = GetSavePath();
        if (path.empty())
            return;

        std::ofstream ofs(path, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        if (!ofs.is_open())
            return;

        int magic_number = SAVE_MAGIC_NUMBER;
        ofs.write(reinterpret_cast<const char*>(&magic_number), sizeof(magic_number));
        std::uint32_t save_version = SAVE_VERSION;
        ofs.write(reinterpret_cast<const char*>(&save_version), sizeof(save_version));

        // 保存RAM
        ofs.write(reinterpret_cast<const char*>(m_RAM.get()), 0x0800);

        // 保存CPU
        auto CPU_data = m_CPU.Save();
        ofs.write(CPU_data.data(), CPU_data.size());

        // 保存PPU
        auto PPU_data = m_PPU.Save();
        ofs.write(PPU_data.data(), PPU_data.size());

        // 保存Mapper
        if (m_cartridge->GetMapper() != nullptr)
        {
            auto Mapper_data = m_cartridge->GetMapper()->Save();
            ofs.write(Mapper_data.data(), Mapper_data.size());
        }

        // 保存APU
        auto APU_data = m_APU.Save();
        ofs.write(APU_data.data(), APU_data.size());

        ofs.close();
        std::cout << "Save success in : " << path << "\n";
    }

    void NesEmulator::Load()
    {
        auto path = GetSavePath();
        if (path.empty())
            return;

        std::ifstream ifs(path, std::ios_base::in | std::ios_base::binary);
        if (!ifs.is_open())
            return;

        do
        {
            int magic_number;
            ifs.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
            if (magic_number != SAVE_MAGIC_NUMBER)
            {
                std::cout << "Load save file error\n";
                break;
            }
            std::uint32_t save_version;
            ifs.read(reinterpret_cast<char*>(&save_version), sizeof(save_version));

            // 读取RAM
            ifs.read(reinterpret_cast<char*>(m_RAM.get()), 0x0800);
            // 读取CPU
            std::vector<char> data(m_CPU.GetSaveFileSize(save_version));
            ifs.read(data.data(), data.size());
            m_CPU.Load(data, save_version);

            // 读取PPU
            data.resize(m_PPU.GetSaveFileSize(save_version));
            ifs.read(data.data(), data.size());
            m_PPU.Load(data, save_version);

            // 读取Mapper
            if (m_cartridge->GetMapper() != nullptr)
            {
                data.resize(m_cartridge->GetMapper()->GetSaveFileSize(save_version));
                ifs.read(data.data(), data.size());
                m_cartridge->GetMapper()->Load(data, save_version);
            }

            // 读取APU
            data.resize(m_APU.GetSaveFileSize(save_version));
            ifs.read(data.data(), data.size());
            m_APU.Load(data, save_version);

            std::cout << "Load success from : " << path << "\n";
        } while(false);

        ifs.close();
    }

}
