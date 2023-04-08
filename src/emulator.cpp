#include "emulator.h"
#include "cartridge.h"
#include <iostream>
#include <memory>

namespace nes
{
    NesEmulator::NesEmulator()
    {
        // 分配2K内存
        m_RAM  = std::make_unique<std::uint8_t[]>(0x0800);
        m_CPU.SetReadFunction([this](std::uint16_t addr)->std::uint8_t{ return MainBusRead(addr); });
        m_CPU.SetWriteFunction([this](std::uint16_t addr, std::uint8_t val)->void{ MainBusWrite(addr, val); });
    }

    NesEmulator::~NesEmulator()
    {

    }

    void NesEmulator::Run()
    {
        m_CPU.Reset();
        for (int i = 0; i < 100; i++)
            m_CPU.Step();
    }

    void NesEmulator::PutInCartridge(std::unique_ptr<Cartridge> cartridge)
    {
        m_cartridge = std::move(cartridge);
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
