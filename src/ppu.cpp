#include "ppu.h"
#include "cartridge.h"
#include "def.h"

namespace nes
{
    PPU::PPU()
    {
        // 分配2K显存
        m_VRAM = std::make_unique<byte[]>(0x0800);
    }

    PPU::~PPU()
    {

    }

    void PPU::Reset()
    {
        
    }

    void PPU::Step()
    {
        
    }

    byte PPU::GetRegister(uint16 address)
    {
        switch (address - 0x2000)
        {
        case 0:
        case 1:
            // Can not read PPUCTRL PPUMASK
            break;
        case 2:
            // 在PPU rendering的时候设置这个标记，然后CPU去读
            return m_register[2];
        case 3:
            // Can not read OAMADDR
            break;
        case 4:
            return GetOAMData();
        case 5:
        case 6:
            // Can not read PPUSCROLL PPUADDR
            break;
        case 7:
            return GetPPUDATA();
        default:
            break;
        }
        return 0;
    }

    void PPU::SetRegister(uint16 address, byte value)
    {
        switch (address - 0x2000)
        {
        case 0:
            m_register[0] = value;
            break;
        case 1:
            m_register[1] = value;
            break;
        case 2:
            // Can not write PPUSTATUS
            break;
        case 3:
            SetOAMADDR(value);
            break;
        case 4:
            SetOAMData(value);
            break;
        case 5:
            SetPPUSCROLL(value);
            break;
        case 6:
            SetPPUADDR(value);
            break;
        case 7:
            SetPPUDATA(value);
            break;
        default:
            break;
        }
    }

    void PPU::SetPPUSCROLL(byte value)
    {
        m_register[5] = value;
    }

    void PPU::SetPPUADDR(byte value)
    {
        if (m_first_write_address)
        {
            m_tmp_address = value & 0x3f; // 地址最大就到 $3FFF, 范围外镜像
            m_tmp_address <<= 8;
            m_first_write_address = false;
        }
        else
        {
            m_tmp_address |= value;
            m_data_address = m_tmp_address;
            m_first_write_address = true;
        }
    }

    void PPU::SetPPUDATA(byte value)
    {
        PPUBusWrite(m_data_address, value);
        m_data_address += GetAddressIncrement();
    }

    byte PPU::GetPPUDATA()
    {
        // 还要交换buffer，麻烦，先不写了
        return 0;
    }

    byte PPU::PPUBusRead(uint16 address)
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
                return GetPalette(address & 0x1f);
        default:
            break;
        }
        return 0;
    }

    void PPU::PPUBusWrite(uint16 address, byte value)
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
                SetPalette(address & 0x1f, value);
            break;
        default:
            break;
        }
    }
}
