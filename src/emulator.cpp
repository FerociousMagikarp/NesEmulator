#include "emulator.h"
#include "cpu.h"
#include "ppu.h"
#include <chrono>
#include <iostream>
#include <memory>

namespace nes
{
    constexpr int NTSC_CPU_FREQUENCY = 1789773; // 之后精细控制时钟周期的时候再用

    NesEmulator::NesEmulator()
        : m_RAM(std::make_unique<std::uint8_t[]>(0x0800))
    {
        m_CPU.SetReadFunction([this](std::uint16_t addr)->std::uint8_t{ return MainBusRead(addr); });
        m_CPU.SetWriteFunction([this](std::uint16_t addr, std::uint8_t val)->void{ MainBusWrite(addr, val); });
        m_PPU.SetReadMapperCHRCallback([this](std::uint16_t addr)->std::uint8_t { return m_cartridge->GetMapper()->ReadCHR(addr); });
        m_PPU.SetWriteMapperCHRCallback([this](std::uint16_t addr, std::uint8_t val)->void{ m_cartridge->GetMapper()->WriteCHR(addr, val); });
        m_PPU.SetNMICallback([this]()->void{ m_CPU.Interrupt(CPU6502InterruptType::NMI); });
    }

    NesEmulator::~NesEmulator()
    {

    }

    void NesEmulator::Run(const bool& running)
    {
        m_CPU.Reset();
        m_PPU.Reset();
        static auto last_time = std::chrono::high_resolution_clock::now();
        while (running)
        {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto delta_time = current_time - last_time;
            if (delta_time > std::chrono::nanoseconds(559))
            {
                last_time += std::chrono::nanoseconds(559);
                m_PPU.Step();
                m_PPU.Step();
                m_PPU.Step();
                m_CPU.Step();
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
            if (address == 0x4016)
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
            if (address == 0x4014) // OAMDMA
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

}
