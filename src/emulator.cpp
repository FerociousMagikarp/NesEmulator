#include "emulator.h"
#include "def.h"
#include "cartridge.h"
#include <iostream>
#include <memory>

namespace nes
{
    Emulator::Emulator()
    {
        m_CPU.SetReadFunction([this](uint16 address)->byte { return MainBusRead(address); });
        m_CPU.SetWriteFunction([this](uint16 address, byte value) { return MainBusWrite(address, value); });
        m_PPU.SetReadFunction([this](uint16 address)->byte { return PPUBusRead(address); });
        m_PPU.SetWriteFunction([this](uint16 address, byte value) { return PPUBusWrite(address, value); });
        // 分配2K内存
        m_RAM  = std::make_unique<byte[]>(0x0800);
        // 分配2K显存
        m_VRAM = std::make_unique<byte[]>(0x0800);
    }

    Emulator::~Emulator()
    {

    }

    void Emulator::PutInCartridge(Cartridge *cartridge)
    {
        m_cartridge = cartridge;
    }

    void Emulator::Run(Device* device, bool& is_running)
    {
        m_CPU.Reset();
        m_PPU.Reset();
        /*while (is_running)
        {
            m_PPU.Step();
            m_PPU.Step();
            m_PPU.Step();
            m_CPU.Step();
        }*/
    }

    byte Emulator::MainBusRead(uint16 address)
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

    void Emulator::MainBusWrite(uint16 address, byte value)
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

    byte Emulator::PPUBusRead(uint16 address)
    {
        switch (address >> 12)
        {
        case 0x00:  // 地址范围 : [0, 0x1000)
            return m_cartridge->GetMapper()->ReadCHR(address); // 图样表0
        case 0x01:  // 地址范围 : [0x1000, 0x2000)
            return m_cartridge->GetMapper()->ReadCHR(address); // 图样表1
        case 0x02:  // 地址范围 : [0x2000, 0x3000)
            // 名称表0 ：[0x2000, 0x2400)
            // 名称表1 ：[0x2400, 0x2800)
            // 名称表2 ：[0x2800, 0x2C00)
            // 名称表3 ：[0x2C00, 0x3000)
            // 先按照名称表0,2对应显存0x0000, 1,3对应0x0400来写
            return m_VRAM[((address >> 10) & 0x01) * 0x400 + (address & 0x3ff)];
        case 0x03:  // 地址范围 : [0x3000, 0x4000)
            if (address < 0x3eff) // [0x2000, 0x2eff)镜像
                return PPUBusRead(address & 0x2fff);
            else
                return m_PPU.GetPalette(address & 0x1f);
        default:
            break;
        }
        return 0;
    }

    void Emulator::PPUBusWrite(uint16 address, byte value)
    {
        switch (address >> 12)
        {
        case 0x00:  // 地址范围 : [0, 0x1000)
            m_cartridge->GetMapper()->WriteCHR(address, value); // 图样表0
            break;
        case 0x01:  // 地址范围 : [0x1000, 0x2000)
            m_cartridge->GetMapper()->WriteCHR(address, value); // 图样表1
            break;
        case 0x02:  // 地址范围 : [0x2000, 0x3000)
            // 名称表0 ：[0x2000, 0x2400)
            // 名称表1 ：[0x2400, 0x2800)
            // 名称表2 ：[0x2800, 0x2C00)
            // 名称表3 ：[0x2C00, 0x3000)
            m_VRAM[((address >> 10) & 0x01) * 0x400 + (address & 0x3ff)] = value;
            break;
        case 0x03:  // 地址范围 : [0x3000, 0x4000)
            if (address < 0x3eff) // [0x2000, 0x2eff)镜像
                PPUBusWrite(address & 0x2fff, value);
            else
                m_PPU.SetPalette(address & 0x1f, value);
            break;
        default:
            break;
        }
    }
}
