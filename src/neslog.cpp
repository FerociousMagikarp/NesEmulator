#include "neslog.h"
#include <array>
#include <stdio.h>
#include "cpu_instructions.h"
#include "cpu.h"
#include "def.h"


namespace nes
{
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


    NesLog& NesLog::GetInstance()
    {
        static NesLog instance;
        return instance;
    }

    void NesLog::ShowCPULog(CPU6502 *cpu, uint16 address, byte op_code, byte op_num1, byte op_num2)
    {
        printf("$%04X    %s ", address, cpu_all_instruction_info[op_code].instruction_name);
        switch (cpu_all_instruction_info[op_code].addressing_type)
        {
        case CPU6502AddressingType::XXX:
        case CPU6502AddressingType::Implied:
            printf("                ");
            break;
        case CPU6502AddressingType::Accumulator:
            printf("A               ");
            break;
        case CPU6502AddressingType::Immediate:
            printf("#$%02X            ", op_num1);
            break;
        case CPU6502AddressingType::Absolute:
        case CPU6502AddressingType::AbsoluteAd:
            printf("#$%02X%02X          ", op_num1, op_num2);
            break;
        case CPU6502AddressingType::AbsoluteX:
            printf("#$%02X%02X, X       ", op_num1, op_num2);
            break;
        case CPU6502AddressingType::AbsoluteY:
            printf("#$%02X%02X, Y       ", op_num1, op_num2);
            break;
        case CPU6502AddressingType::ZeroPage:
            printf("#$%02X            ", op_num1);
            break;
        case CPU6502AddressingType::ZeroPageX:
            printf("#$%02X, X         ", op_num1);
            break;
        case CPU6502AddressingType::ZeroPageY:
            printf("#$%02X, Y         ", op_num1);
            break;
        case CPU6502AddressingType::Indirect:
            printf("($%02X%02X)         ", op_num1, op_num2);
            break;
        case CPU6502AddressingType::IndirectX:
            printf("(#$%02X, X)       ", op_num1);
            break;
        case CPU6502AddressingType::IndirectY:
            printf("($%02X), Y        ", op_num1);
            break;
        case CPU6502AddressingType::Relative:
            printf("%-4d            ", (int8)op_num1);
            break;
        }
        printf("A : %02X, X : %02X, Y : %02X, SP : %02X, Status : %02X\n", cpu->m_A, cpu->m_X, cpu->m_Y, cpu->m_SP, cpu->m_P);
    }
}
