#include "cpu.h"
#include "def.h"
#include <iostream>

namespace nes
{
    enum ProgramStatus
    {
        C = (1 << 0),
        Z = (1 << 1),
        I = (1 << 2),
        D = (1 << 3),
        B = (1 << 4),
        V = (1 << 6),
        N = (1 << 7)
    };

    CPU6502::CPU6502() noexcept
    {
        
    }

    CPU6502::~CPU6502()
    {

    }

    void CPU6502::Reset()
    {
        m_A = m_X = m_Y = 0;
        m_SP = 0xfd;
        m_P = 0x24;
        m_PC = ReadAddress(RESET_ADDRESS);
    }

    void CPU6502::Interrupt(CPU6502InterruptType type)
    {
        m_current_interrupt |= (1 << static_cast<int>(type));
    }

    uint16 CPU6502::ReadAddress(uint16 start_address)
    {
        return m_read_function(start_address) | (static_cast<uint16>(start_address + 1) << 8);
    }

    void CPU6502::InterruptExecute(CPU6502InterruptType type)
    {
        if (GetI() && type == CPU6502InterruptType::IRQ)
            return;
        if (type == CPU6502InterruptType::BRK)
            m_PC++;
        PushStack(m_PC >> 8);
        PushStack(m_PC);
        SetFlag(B, type == CPU6502InterruptType::BRK);
        PushStack(m_P);
        SetFlag(I, true);
        switch (type)
        {
        case CPU6502InterruptType::BRK:
        case CPU6502InterruptType::IRQ:
            m_PC = ReadAddress(BRK_VECTOR);
            break;
        case CPU6502InterruptType::NMI:
            m_PC = ReadAddress(NMI_VECTOR);
            break;
        }
    }

    bool CPU6502::ADC()
    {
        uint32 tmp = m_src + m_A + (GetC() ? 1: 0);
        SetFlag(Z, (tmp & 0xff) == 0);
        SetFlag(N, tmp & 0x80);
        SetFlag(C, tmp > 0xff);
        SetFlag(V, !((m_A ^ m_src) & 0x80) && ((m_A & tmp) & 0x80));
        m_A = tmp & 0xff;
        return false;
    }
    
    bool CPU6502::AND()
    {
        m_A = static_cast<byte>(m_src & m_A);
        SetFlag(N, m_A & 0x80);
        SetFlag(Z, m_A == 0x00);
        return false;
    }

    bool CPU6502::ASL()
    {
        SetFlag(C, m_src & 0x80);
        m_src = (m_src << 1) & 0xff;
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        return true;
    }

    bool CPU6502::BCC()
    {
        if (!GetC())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::BCS()
    {
        if (GetC())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::BEQ()
    {
        if (GetZ())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::BIT()
    {
        SetFlag(N, m_src & 0x80);
        SetFlag(V, m_src & 0x40);
        SetFlag(Z, (m_A & m_src & 0xff) == 0);
        return false;
    }

    bool CPU6502::BMI()
    {
        if (GetN())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::BNE()
    {
        if (!GetZ())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::BPL()
    {
        if (!GetN())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::BRK()
    {
        InterruptExecute(CPU6502InterruptType::BRK);
        return false;
    }

    bool CPU6502::BVC()
    {
        if (!GetV())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::BVS()
    {
        if (GetV())
        {
            m_skip_cycles += (m_PC & 0xff00) != ((m_PC + m_src) & 0xff00) ? 2 : 1;
            m_PC += m_src;
        }
        return false;
    }

    bool CPU6502::CLC()
    {
        SetFlag(C, false);
        return false;
    }

    bool CPU6502::CLD()
    {
        SetFlag(D, false);
        return false;
    }

    bool CPU6502::CLI()
    {
        SetFlag(I, false);
        return false;
    }

    bool CPU6502::CLV()
    {
        SetFlag(V, false);
        return false;
    }

    bool CPU6502::CMP()
    {
        m_src = m_A - m_src;
        SetFlag(C, m_src < 0x100);
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, (m_src & 0xff) == 0);
        return false;
    }

    bool CPU6502::CPX()
    {
        m_src = m_X - m_src;
        SetFlag(C, m_src < 0x100);
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, (m_src & 0xff) == 0);
        return false;
    }

    bool CPU6502::CPY()
    {
        m_src = m_Y - m_src;
        SetFlag(C, m_src < 0x100);
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, (m_src & 0xff) == 0);
        return false;
    }

    bool CPU6502::DEC()
    {
        m_src = (m_src - 1) & 0xff;
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        return true;
    }

    bool CPU6502::DEX()
    {
        m_X--;
        SetFlag(N, m_X & 0x80);
        SetFlag(Z, m_X == 0);
        return false;
    }

    bool CPU6502::DEY()
    {
        m_Y--;
        SetFlag(N, m_Y & 0x80);
        SetFlag(Z, m_Y == 0);
        return false;
    }

    bool CPU6502::EOR()
    {
        m_A ^= m_src;
        SetFlag(N, m_A & 0x80);
        SetFlag(Z, m_A == 0);
        return false;
    }

    bool CPU6502::INC()
    {
        m_src = (m_src + 1) & 0xff;
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        return true;
    }

    bool CPU6502::INX()
    {
        m_X++;
        SetFlag(N, m_X & 0x80);
        SetFlag(Z, m_X == 0);
        return false;
    }

    bool CPU6502::INY()
    {
        m_Y++;
        SetFlag(N, m_Y & 0x80);
        SetFlag(Z, m_Y == 0);
        return false;
    }

    bool CPU6502::JMP()
    {
        m_PC = m_src;
        return false;
    }

    bool CPU6502::JSR()
    {
        m_PC--;
        PushStack(m_PC >> 8);
        PushStack(m_PC);
        m_PC = m_src;
        return false;
    }

    bool CPU6502::LDA()
    {
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        m_A = m_src;
        return false;
    }

    bool CPU6502::LDX()
    {
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        m_X = m_src;
        return false;
    }

    bool CPU6502::LDY()
    {
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        m_Y = m_src;
        return false;
    }

    bool CPU6502::LSR()
    {
        SetFlag(C, m_src & 0x01);
        m_src >>= 1;
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        return true;
    }

    bool CPU6502::NOP()
    {
        return false;
    }

    bool CPU6502::ORA()
    {
        m_A |= m_src;
        SetFlag(N, m_A & 0x80);
        SetFlag(Z, m_A == 0);
        return false;
    }

    bool CPU6502::PHA()
    {
        PushStack(m_A);
        return false;
    }

    bool CPU6502::PHP()
    {
        PushStack(m_SP);
        return false;
    }

    bool CPU6502::PLA()
    {
        auto value = PullStack();
        SetFlag(N, value & 0x80);
        SetFlag(Z, value == 0);
        return false;
    }

    bool CPU6502::PLP()
    {
        m_SP = PullStack();
        return false;
    }

    bool CPU6502::ROL()
    {
        m_src = (m_src << 1) | GetC();
        SetFlag(C, m_src > 0xff);
        m_src &= 0xff;
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        return true;
    }

    bool CPU6502::ROR()
    {
        if (GetC())
            m_src |= 0x100;
        SetFlag(C, m_src & 0x01);
        m_src >>= 1;
        SetFlag(N, m_src & 0x80);
        SetFlag(Z, m_src == 0);
        return true;
    }

    bool CPU6502::RTI()
    {
        m_SP = PullStack();
        uint16 value = PullStack();
        value |= static_cast<uint16>(PullStack()) << 8;
        m_PC = value;
        return false;
    }

    bool CPU6502::RTS()
    {
        uint16 value = PullStack();
        value |= (static_cast<uint16>(PullStack()) << 8) + 1;
        m_PC = value;
        return false;
    }

    bool CPU6502::SBC()
    {
        uint32 tmp = m_src - m_A - (GetC() ? 1: 0);
        SetFlag(Z, (tmp & 0xff) == 0);
        SetFlag(N, tmp & 0x80);
        SetFlag(C, tmp < 0x100);
        SetFlag(V, ((m_A ^ m_src) & 0x80) && ((m_A & tmp) & 0x80));
        m_A = tmp & 0xff;
        return false;
    }

    bool CPU6502::SEC()
    {
        SetFlag(C, true);
        return false;
    }

    bool CPU6502::SED()
    {
        SetFlag(D, true);
        return false;
    }

    bool CPU6502::SEI()
    {
        SetFlag(I, true);
        return false;
    }

    bool CPU6502::STA()
    {
        m_src = m_A;
        return true;
    }

    bool CPU6502::STX()
    {
        m_src = m_X;
        return true;
    }

    bool CPU6502::STY()
    {
        m_src = m_Y;
        return true;
    }

    bool CPU6502::TAX()
    {
        SetFlag(N, m_A & 0x80);
        SetFlag(Z, m_A == 0);
        m_X = m_A;
        return false;
    }

    bool CPU6502::TAY()
    {
        SetFlag(N, m_A & 0x80);
        SetFlag(Z, m_A == 0);
        m_Y = m_A;
        return false;
    }

    bool CPU6502::TSX()
    {
        SetFlag(N, m_SP & 0x80);
        SetFlag(Z, m_SP == 0);
        m_X = m_SP;
        return false;
    }

    bool CPU6502::TXA()
    {
        SetFlag(N, m_X & 0x80);
        SetFlag(Z, m_X == 0);
        m_A = m_X;
        return false;
    }

    bool CPU6502::TXS()
    {
        m_SP = m_X;
        return false;
    }

    bool CPU6502::TYA()
    {
        SetFlag(N, m_Y & 0x80);
        SetFlag(Z, m_Y == 0);
        m_A = m_Y;
        return false;
    }
}
