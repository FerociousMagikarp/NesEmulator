#include "ppu.h"
#include <cstdint>
#include <wchar.h>

namespace nes
{
    constexpr int SCANLINE_PER_FRAME = 262;
    constexpr int CYCLE_PER_SCANLINE = 340;

    PPU::PPU() : m_VRAM(std::make_unique<std::uint8_t[]>(0x0800))
    {
        m_secondary_OAM.reserve(8);
    }

    PPU::~PPU()
    {

    }

    void PPU::Reset()
    {   
        m_scanline = 261;
        m_cycle = 0;
    }

    void PPU::Step()
    {
        if (m_scanline == 261) // PreRender
            StepPreRenderScanline();
        else if (m_scanline >= 0 && m_scanline <= 239) // Visible
            StepVisibleScanlines();
        else if (m_scanline == 240) // PostRender
            StepPostRenderScanline();
        else // m_scanline >= 241 && m_scanline <= 260, VerticalBlanking
            StepVerticalBlankingLines();
    }

    void PPU::StepPreRenderScanline()
    {
        if (IsRenderingEnabled())
        {
            if (m_cycle == 1)
            {
                m_PPUSTATUS &= ~0xC0; // 清除sprite 0 hit和vertical blank标记
            }
            else if (m_cycle == 257)
            {
                // v: ....A.. ...BCDEF <- t: ....A.. ...BCDEF
                m_PPUADDR &= ~0x041f;
                m_PPUADDR |= m_internal_register_wt & 0x041f;
            }
            else if (m_cycle >= 280 && m_cycle <= 304)
            {
                // v: GHIA.BC DEF..... <- t: GHIA.BC DEF.....
                m_PPUADDR &= ~0x7be0;
                m_PPUADDR |= m_internal_register_wt & 0x7be0;
            }
            else if (IsFetchingCycle())
            {
                FetchingTile();
                if (m_cycle % 8 == 0)
                {
                    ShiftTile();
                    IncHorizontal();
                }
            }
        }
        if (m_cycle >= 257 && m_cycle <= 320)
        {
            m_OAMADDR = 0;
        }
        // 奇数帧的时候会少一个cycle，直接跳到下一个渲染
        if (m_cycle++ >= CYCLE_PER_SCANLINE - ((m_frame & 1) && IsRenderingEnabled()))
        {
            m_cycle = 0;
            m_scanline = 0;
            m_device->StartPPURender(); // 趁这个时候把锁加上
        }
    }

    void PPU::StepVisibleScanlines()
    {
        if (m_cycle == 0)
        {
        }
        else if (IsRenderingCycle())
        {
            std::uint8_t background_color_index = 0;
            std::uint8_t sprite_color_index = 0;

            if (IsShowBackgroundEnabled() && (IsShowBackgroundLeftmost8() || m_cycle > 8))
            {
                int x = (m_fine_x_scroll + m_cycle - 1) & 0x07;
                background_color_index = ((m_pattern_high >> (7 - x) << 1) & 0x02) | (m_pattern_low >> (7 - x) & 0x01);
                if (background_color_index != 0)
                {
                    background_color_index |= (m_attribute_table & 0x0f);
                }

                FetchingTile();
                
                if (m_cycle % 8 == 0)
                {
                    ShiftTile();
                    IncHorizontal();
                }
                if (m_cycle == 256)
                {
                    IncVertical();
                }
            }
            if (IsShowSpriteEnabled() && (IsShowSpriteLeftmost8() || m_cycle > 8))
            {
                for (int i : m_secondary_OAM)
                {
                    int x = m_primary_OAM[i * 4 + 3];
                    int diff_x = m_cycle - x - 1;
                    if (diff_x >= 0 && diff_x < 8)
                    {
                        int y = m_primary_OAM[i * 4] + 1;
                        int index = m_primary_OAM[i * 4 + 1];
                        int attribute = m_primary_OAM[i * 4 + 2];

                        std::uint16_t pattern_addr = 0;
                        int diff_y = m_scanline - y;

                        if ((attribute & 0x40) == 0)
                            diff_x = 7 - diff_x;
                        if ((attribute & 0x80) != 0)
                            diff_y = (IsSpriteSize8x16() ? 15 : 7) - diff_y;

                        if (IsSpriteSize8x16())
                        {
                            diff_y = (diff_y & 0x07) | ((diff_y & 0x08) << 1);
                            pattern_addr = ((index >> 1) << 5) + diff_y;
                            pattern_addr |= (index & 0x01) << 12;
                        }
                        else
                        {
                            pattern_addr = (index * 16 + diff_y) | GetSpritePatternTableAddress();
                        }

                        std::uint8_t color = (PPUBusRead(pattern_addr) >> diff_x) & 0x01;
                        color |= ((PPUBusRead(pattern_addr + 8) >> diff_x) & 0x01) << 1;
                        if (color == 0)
                            continue;

                        color |= ((attribute & 0x03) << 2) | 0x10;

                        if (!IsSprite0Hit() && IsShowBackgroundEnabled() && i == 0)
                        {
                            m_PPUSTATUS |= 0x40;
                        }
                        if ((attribute & 0x20) == 0)
                            sprite_color_index = color;

                        break;
                    }
                }

                if (m_cycle == 256)
                {
                    // 本来是65~256cycle去做这个事，省了，就在最后一周期搞了，以后有问题以后再说
                    SpriteEvaluation(m_scanline + 1);
                }
            }

            std::uint8_t color_index = sprite_color_index == 0 ? background_color_index : sprite_color_index;
            m_device->SetPixel(m_cycle - 1, m_scanline, GetPalette(color_index & 0x1f) & 0x3f);
        }
        else if (m_cycle >= 257 && m_cycle <= 320)
        {
            if (m_cycle == 257 && IsRenderingEnabled())
            {
                // v: ....A.. ...BCDEF <- t: ....A.. ...BCDEF
                m_PPUADDR &= ~0x041f;
                m_PPUADDR |= m_internal_register_wt & 0x041f;
            }
            m_OAMADDR = 0;
        }
        else if (IsFetchingCycle())
        {
        }
        else // m_cycle >= 337 && m_cycle <= 340
        {
        }

        if (m_cycle++ >= CYCLE_PER_SCANLINE)
        {
            m_cycle = 0;
            ++m_scanline;
        }
    }

    void PPU::StepPostRenderScanline()
    {
        if (m_cycle == 0)
        {
            m_device->EndPPURender();
        }
        if (m_cycle++ >= CYCLE_PER_SCANLINE)
        {
            m_cycle = 0;
            ++m_scanline;
        }
    }

    void PPU::StepVerticalBlankingLines()
    {
        if (m_scanline == 241 && m_cycle == 1)
        {
            m_PPUSTATUS |= 0x80; // 设置vertical blank标记
            if (IsNMIEnabled())
                m_trigger_NMI();
        }
        if (m_cycle++ >= CYCLE_PER_SCANLINE)
        {
            m_cycle = 0;
            if (++m_scanline > 260)
                ++m_frame;
        }
    }

    void PPU::IncHorizontal()
    {
        // 宰予抄Wiki代码。子曰：朽木不可雕也，粪土之墙不可杇也。于予与何诛？
        if ((m_PPUADDR & 0x001F) == 31)    // if coarse X == 31
        {
            m_PPUADDR &= ~0x001F;          // coarse X = 0
            m_PPUADDR ^= 0x0400;           // switch horizontal nametable
        }
        else
            m_PPUADDR += 1;                // increment coarse X
    }

    void PPU::IncVertical()
    {
        // 子贡问曰：https://www.nesdev.org/wiki/PPU_scrolling#Tile_and_attribute_fetching 可抄吗？
        // 子曰：伪代码都给你了，为啥不抄！     你怎么连注释都抄了?
        if ((m_PPUADDR & 0x7000) != 0x7000)        // if fine Y < 7
            m_PPUADDR += 0x1000;                   // increment fine Y
        else
        {
            m_PPUADDR &= ~0x7000;                  // fine Y = 0
            int y = (m_PPUADDR & 0x03E0) >> 5;     // let y = coarse Y
            if (y == 29)
            {
                y = 0;                             // coarse Y = 0
                m_PPUADDR ^= 0x0800;               // switch vertical nametable
            }
            else if (y == 31)
                y = 0;                             // coarse Y = 0, nametable not switched
            else
                y += 1;                            // increment coarse Y
            m_PPUADDR = (m_PPUADDR & ~0x03E0) | (y << 5); // put coarse Y back into v
        }
    }

    void PPU::FetchingTile()
    {
        switch (m_cycle % 8)
        {
            case 1:
                FetchingNametable();
                break;
            case 3:
                FetchingAttribute();
                break;
            case 5:
                FetchingPatternLow();
                break;
            case 7:
                FetchingPatternHigh();
                break;
            default:
                break;
        }
    }

    void PPU::ShiftTile()
    {
        m_attribute_table >>= 8;
        m_pattern_low >>= 8;
        m_pattern_high >>= 8;
    }

    void PPU::FetchingNametable()
    {
        std::uint16_t name_addr = 0x2000 | (m_PPUADDR & 0x0fff);
        m_nametable = PPUBusRead(name_addr);
    }

    void PPU::FetchingAttribute()
    {
        std::uint16_t attribute_addr = 0x23c0 | (m_PPUADDR & 0x0c00) | ((m_PPUADDR >> 4) & 0x38) | ((m_PPUADDR >> 2) & 0x07);
        std::uint16_t attribute = PPUBusRead(attribute_addr);
        m_attribute_table |= (attribute >> ((m_PPUADDR >> 4 & 0x04) | (m_PPUADDR & 0x02)) & 0x03) << 10;
    }

    void PPU::FetchingPatternLow()
    {
        std::uint16_t pattern_address = (static_cast<std::uint16_t>(m_nametable) << 4) | ((m_PPUADDR >> 12) & 0x07) | GetBackgroundPatternTableAddress();
        m_pattern_low |= static_cast<std::uint16_t>(PPUBusRead(pattern_address)) << 8;
    }

    void PPU::FetchingPatternHigh()
    {
        std::uint16_t pattern_address = (static_cast<std::uint16_t>(m_nametable) << 4) | ((m_PPUADDR >> 12) & 0x07) | GetBackgroundPatternTableAddress() | 0x08;
        m_pattern_high |= static_cast<std::uint16_t>(PPUBusRead(pattern_address)) << 8;
    }

    void PPU::SpriteEvaluation(int scanline)
    {
        int limit = IsSpriteSize8x16() ? 16 : 8;
        m_secondary_OAM.clear();

        for (int i = 0; i < 64; i++)
        {
            int diff = scanline - m_primary_OAM[i * 4] - 1;
            if (diff >= 0 && diff < limit)
            {
                if (m_secondary_OAM.size() >= 8)
                {
                    m_PPUSTATUS |= 0x20;
                    break;
                }
                m_secondary_OAM.push_back(i);
            }
        }
    }

    std::uint8_t PPU::GetRegister(std::uint16_t address)
    {
        switch (address & 0x7)
        {
        case 0:
        case 1:
            // Can not read PPUCTRL PPUMASK
            break;
        case 2:
            // 在PPU rendering的时候设置这个标记，然后CPU去读
            return GetPPUSTATUS();
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

    void PPU::SetRegister(std::uint16_t address, std::uint8_t value)
    {
        switch (address & 0x7)
        {
        case 0:
            SetPPUCTRL(value);
            break;
        case 1:
            m_PPUMASK = value;
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

    void PPU::SetPPUCTRL(std::uint8_t value)
    {
        m_PPUCTRL = value;
        m_internal_register_wt &= ~0x0c00;
        m_internal_register_wt |= (value & 0x3) << 10;
    }

    std::uint8_t PPU::GetPPUSTATUS()
    {
        m_internal_register_wt &= ~0x8000;
        std::uint8_t res = m_PPUSTATUS;
        m_PPUSTATUS &= ~0x80;
        return res;
    }

    void PPU::SetOAMADDR(std::uint8_t value)
    {
        m_OAMADDR = value;
    }

    void PPU::SetOAMData(std::uint8_t value)
    {
        if ((m_scanline >= 240 && m_scanline <= 260) || !IsRenderingEnabled())
            m_primary_OAM[m_OAMADDR] = value;
        ++m_OAMADDR;
    }

    std::uint8_t PPU::GetOAMData() const
    {
        if (m_scanline >= 241 && m_scanline <= 260)
            return m_primary_OAM[m_OAMADDR];
        return 0xff;
    }

    void PPU::SetPPUSCROLL(std::uint8_t value)
    {
        if (!(m_internal_register_wt & 0x8000))
        {
            // t: ....... ...ABCDE <- d: ABCDE...
            // x:              FGH <- d: .....FGH
            m_internal_register_wt &= ~0x1f;
            m_internal_register_wt |= (value >> 3) & 0x1f;
            m_fine_x_scroll = value & 0x07;
            m_internal_register_wt |= 0x8000;
        }
        else
        {
            // t: FGH..AB CDE..... <- d: ABCDEFGH
            m_internal_register_wt &= ~0x73e0;
            m_internal_register_wt |= static_cast<std::uint16_t>(value & 0xf8) << 2;
            m_internal_register_wt |= static_cast<std::uint16_t>(value & 0x07) << 12;
            m_internal_register_wt &= ~0x8000;
        }
    }

    void PPU::SetPPUADDR(std::uint8_t value)
    {
        if (!(m_internal_register_wt & 0x8000))
        {
            m_internal_register_wt &= 0x00ff;
            m_internal_register_wt |= (value & 0x3f) << 8;
            m_internal_register_wt |= 0x8000;  // 把最高位的w置上
        }
        else
        {
            m_internal_register_wt &= 0xff00;
            m_internal_register_wt |= value;
            m_internal_register_wt &= ~0x8000; // w标记置回去
            m_PPUADDR = m_internal_register_wt & 0x3fff;
        }
    }

    void PPU::SetPPUDATA(std::uint8_t value)
    {
        PPUBusWrite(m_PPUADDR, value);
        m_PPUADDR = (m_PPUADDR + GetAddressIncrement()) & 0x3fff;
    }

    std::uint8_t PPU::GetPPUDATA()
    {
        std::uint8_t res = m_PPUDATA_buffer;
        m_PPUDATA_buffer = PPUBusRead(m_PPUADDR);
        if (m_PPUADDR >= 0x3f00)
            res = m_PPUDATA_buffer;
        m_PPUADDR = (m_PPUADDR + GetAddressIncrement()) & 0x3fff;
        return res;
    }

    std::uint16_t PPU::GetVRAMAddress(std::uint16_t address)
    {
        switch (m_mirror_type)
        {
            case MirrorType::Horizontal:
                if (address >= 0x2400 && address < 0x2c00)
                    address -= 0x0400;
                else if (address >= 0x2c00 && address < 0x3000)
                    address -= 0x0800;
                break;
            case MirrorType::Vertical:
                if (address >= 0x2800 && address < 0x3000)
                    address -= 0x0800;
                break;
        }

        return address & 0x07ff;
    }

    void PPU::OAMDMA(std::uint8_t *data)
    {
        // 从OAMADDR为起始下标，拷贝256字节
        if (m_OAMADDR == 0)
            memcpy(m_primary_OAM.data(), data, 256);
        else
        {
            memcpy(m_primary_OAM.data() + m_OAMADDR, data, 256 - m_OAMADDR);
            memcpy(m_primary_OAM.data(), data + 256 - m_OAMADDR, m_OAMADDR);
        }
    }

    std::uint8_t PPU::PPUBusRead(std::uint16_t address)
    {
        switch (address >> 12)
        {
        case 0x00:  // 地址范围 : [0, 0x1000)
        case 0x01:  // 地址范围 : [0x1000, 0x2000)
            return m_mapper_read_CHR(address);
        case 0x02:  // 地址范围 : [0x2000, 0x3000)
            // 名称表0 ：[0x2000, 0x2400)
            // 名称表1 ：[0x2400, 0x2800)
            // 名称表2 ：[0x2800, 0x2C00)
            // 名称表3 ：[0x2C00, 0x3000)
            return m_VRAM[GetVRAMAddress(address)];
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

    void PPU::PPUBusWrite(std::uint16_t address, std::uint8_t value)
    {
        switch (address >> 12)
        {
        case 0x00:  // 地址范围 : [0, 0x1000)
        case 0x01:  // 地址范围 : [0x1000, 0x2000)
            m_mapper_write_CHR(address, value);
            break;
        case 0x02:  // 地址范围 : [0x2000, 0x3000)
            // 名称表0 ：[0x2000, 0x2400)
            // 名称表1 ：[0x2400, 0x2800)
            // 名称表2 ：[0x2800, 0x2C00)
            // 名称表3 ：[0x2C00, 0x3000)
            m_VRAM[GetVRAMAddress(address)] = value;
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

    std::uint8_t PPU::GetPalette(int index)
    {
        // 调色板镜像
        if (index >= 0x10 && (index & 0x03) == 0)
            return m_palette[index & 0x0f];
        else
            return m_palette[index];
    }

    void PPU::SetPalette(int index, std::uint8_t value)
    {
        if (index >= 0x10 && (index & 0x03) == 0)
            m_palette[index & 0x0f] = value;
        else
            m_palette[index] = value;
    }
}
