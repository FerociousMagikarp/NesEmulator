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
        SetP(B, type == CPU6502InterruptType::BRK);
        PushStack(m_P);
        SetP(I, true);
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
}
