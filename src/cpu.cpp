#include "cpu.h"
#include "cpu_instructions.h"
#include "def.h"
#include <iostream>
#include <type_traits>

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

    struct InstructionInfo
    {
        char                  instruction_name[4];
        CPU6502AddressingType addressing_type;
        int                   cycles;
    };

    constexpr auto GetAllInstructionInfo()
    {
        std::array<InstructionInfo, 256> result{};

        #define OPERATION(op_code_, instruction_, addressing_, cycles_, ...) \
        { \
            InstructionInfo info{#instruction_, CPU6502AddressingType::addressing_, cycles_}; \
            result[op_code_] = info; \
        } \

        _CPU6502_ALL_INSTRUCTIONS_
        #undef OPERATION

        return result;
    }

    constexpr auto cpu_all_instruction_info = GetAllInstructionInfo();

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

    void CPU6502::Step()
    {
        if (m_skip_cycles > 0)
        {
            m_skip_cycles--;
            return;
        }
        // 执行中断
        if (m_current_interrupt != 0)
        {
            if (m_current_interrupt & (1 << static_cast<int>(CPU6502InterruptType::NMI)))
            {
                InterruptExecute(CPU6502InterruptType::NMI);
                m_current_interrupt = 0;
                m_skip_cycles--; // 本周期已经执行过了，所以-1
                return;
            }
            else if (m_current_interrupt & (1 << static_cast<int>(CPU6502InterruptType::IRQ)))
            {
                InterruptExecute(CPU6502InterruptType::IRQ);
                m_current_interrupt = 0;
                m_skip_cycles--; // 本周期已经执行过了，所以-1
                return;
            }
        }
        // 读取指令
        byte op_code = m_read_function(m_PC++);
        // std::cout << m_PC-1 << " : " << (int)op_code << "  " << cpu_all_instruction_info[op_code].instruction_name << std::endl;
        ExecuteCode(op_code);
        m_skip_cycles--; // 本周期已经执行过了，所以-1
    }

    void CPU6502::Interrupt(CPU6502InterruptType type)
    {
        m_current_interrupt |= (1 << static_cast<int>(type));
    }

    uint16 CPU6502::ReadAddress(uint16 start_address)
    {
        return m_read_function(start_address) | (static_cast<uint16>(m_read_function(start_address + 1)) << 8);
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
        m_skip_cycles += 7;
    }

    void CPU6502::ExecuteCode(uint8 op_code)
    {
        // 为可能使用到的地址
        uint16 address;

        #define OPERATION(op_code_, instruction_, addressing_, cycles_, ...) \
        case op_code_: \
            if constexpr (CPU6502AddressingType::addressing_ == CPU6502AddressingType::XXX) \
            { \
                std::cout << "Unrecognized OP Code : " << op_code_ << std::endl; \
            } \
            else if constexpr (CPU6502AddressingType::addressing_ == CPU6502AddressingType::Implied || \
                               CPU6502AddressingType::addressing_ == CPU6502AddressingType::Immediate || \
                               CPU6502AddressingType::addressing_ == CPU6502AddressingType::Accumulator || \
                               CPU6502AddressingType::addressing_ == CPU6502AddressingType::Indirect || \
                               CPU6502AddressingType::addressing_ == CPU6502AddressingType::Relative) \
            { \
                this->addressing_(); \
                this->instruction_(); \
                this->m_skip_cycles += cycles_; \
            } \
            else \
            { \
                address = this->addressing_(); \
                if (this->instruction_()) /*结果为true说明需要写回地址*/ \
                    this->m_write_function(address, (this->m_src) & 0xff); \
                this->m_skip_cycles += cycles_; \
                /*只有在这种情况下才会出现跨页加指令周期*/ \
                if constexpr(0##__VA_ARGS__) \
                { \
                    if ((this->m_PC & 0xff00) != (address & 0xff00))\
                        this->m_skip_cycles++; \
                } \
            } \
            break; \

        switch (op_code)
        {
            _CPU6502_ALL_INSTRUCTIONS_
        }

        #undef OPERATION
    }

    // =========================================
    // 寻址模式
    // =========================================

    // 立即寻址
    uint16 CPU6502::Immediate()
    {
        m_src = m_read_function(m_PC++);
        return 0;
    }

    uint16 CPU6502::Absolute()
    {
        uint16 address = m_read_function(m_PC++);
        address |= static_cast<uint16>(m_PC++) << 8;
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::AbsoluteAd()
    {
        uint16 address = m_read_function(m_PC++);
        address |= static_cast<uint16>(m_PC++) << 8;
        m_src = ReadAddress(address);
        return 0;
    }

    uint16 CPU6502::ZeroPage()
    {
        uint16 address = m_read_function(m_PC++);
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::Accumulator()
    {
        m_src = m_A;
        return 0;
    }

    uint16 CPU6502::AbsoluteX()
    {
        uint16 address = m_read_function(m_PC++);
        address |= static_cast<uint16>(m_PC++) << 8;
        address += m_X;
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::AbsoluteY()
    {
        uint16 address = m_read_function(m_PC++);
        address |= static_cast<uint16>(m_PC++) << 8;
        address += m_Y;
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::ZeroPageX()
    {
        uint16 address = m_read_function(m_PC++);
        address += m_X;
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::ZeroPageY()
    {
        uint16 address = m_read_function(m_PC++);
        address += m_Y;
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::Indirect()
    {
        // 这个仅用于JMP，而且还有bug
        uint16 address_tmp = m_read_function(m_PC++);
        address_tmp |= static_cast<uint16>(m_read_function(m_PC++)) << 8;
        uint16 addresss_first = (address_tmp & 0xff00) | ((address_tmp + 1) & 0x00ff);
        uint16 address = m_read_function(address_tmp);
        address |= static_cast<uint16>(m_read_function(addresss_first)) << 8;
        m_src = address;
        return address;
    }

    uint16 CPU6502::IndirectX()
    {
        byte op = m_read_function(m_PC++);
        uint16 address_first = m_read_function(op + m_X);
        uint16 address = m_read_function(address_first);
        address |= static_cast<uint16>(m_read_function(address_first + 1)) << 8;
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::IndirectY()
    {
        uint16 address = m_read_function(m_PC++);
        address |= static_cast<uint16>(m_read_function(m_PC++)) << 8;
        address += m_Y;
        m_src = m_read_function(address);
        return address;
    }

    uint16 CPU6502::Relative()
    {
        m_src = m_read_function(m_PC++);
        m_src = m_PC + static_cast<int8>(m_src);
        return 0;
    }

    // =========================================
    // 指令
    // =========================================

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
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
        }
        return false;
    }

    bool CPU6502::BCS()
    {
        if (GetC())
        {     
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
        }
        return false;
    }

    bool CPU6502::BEQ()
    {
        if (GetZ())
        {
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
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
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
        }
        return false;
    }

    bool CPU6502::BNE()
    {
        if (!GetZ())
        {
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
        }
        return false;
    }

    bool CPU6502::BPL()
    {
        if (!GetN())
        {
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
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
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
        }
        return false;
    }

    bool CPU6502::BVS()
    {
        if (GetV())
        {
            m_skip_cycles += (m_PC & 0xff00) != (m_src & 0xff00) ? 2 : 1;
            m_PC = m_src;
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
