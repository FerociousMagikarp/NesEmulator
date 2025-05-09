#include "cpu_disassembly.h"
#include <array>
#include <stdio.h>
#include "cpu.h"
#include "cpu_instructions.h"

namespace nes
{
    constexpr const char* GetImmediateDataFormat()    { return "#$%02X"; }
    constexpr const char* GetAbsoluteDataFormat()     { return "$%04X"; }
    constexpr const char* GetZeroPageDataFormat()     { return "$%02X"; }
    constexpr const char* GetImpliedDataFormat()      { return ""; }
    constexpr const char* GetAccumulatorDataFormat()  { return "A"; }
    constexpr const char* GetAbsoluteXDataFormat()    { return "$%04X, X"; }
    constexpr const char* GetAbsoluteYDataFormat()    { return "$%04X, Y"; }
    constexpr const char* GetZeroPageXDataFormat()    { return "$%02X, X"; }
    constexpr const char* GetZeroPageYDataFormat()    { return "$%02X, Y"; }
    constexpr const char* GetIndirectDataFormat()     { return "($%04X)"; }
    constexpr const char* GetIndirectXDataFormat()    { return "($%02X, X)"; }
    constexpr const char* GetIndirectYDataFormat()    { return "($%02X), Y"; }
    constexpr const char* GetRelativeDataFormat()     { return "$%02X"; }
    constexpr int GetImmediateLength()   { return 2; }
    constexpr int GetAbsoluteLength()    { return 3; }
    constexpr int GetZeroPageLength()    { return 2; }
    constexpr int GetImpliedLength()     { return 1; }
    constexpr int GetAccumulatorLength() { return 1; }
    constexpr int GetAbsoluteXLength()   { return 3; }
    constexpr int GetAbsoluteYLength()   { return 3; }
    constexpr int GetZeroPageXLength()   { return 2; }
    constexpr int GetZeroPageYLength()   { return 2; }
    constexpr int GetIndirectLength()    { return 3; }
    constexpr int GetIndirectXLength()   { return 2; }
    constexpr int GetIndirectYLength()   { return 2; }
    constexpr int GetRelativeLength()    { return 2; }

    constexpr auto GetAllInstructionNames()
    {
        std::array<const char*, 256> result{};

        #define OPERATION(op_code_, instruction_, addressing_, ...) \
            result[op_code_] = #instruction_; \

        CPU6502_ALL_INSTRUCTIONS

        #undef OPERATION

        return result;
    }

    constexpr auto GetAllInstructionDataFormat()
    {
        std::array<const char*, 256> result{};

        #define OPERATION(op_code_, instruction_, addressing_, ...) \
            result[op_code_] = Get##addressing_##DataFormat(); \

        CPU6502_ALL_INSTRUCTIONS

        #undef OPERATION

        return result;
    }

    constexpr auto GetAllInstructionLength()
    {
        std::array<int, 256> result{};

        #define OPERATION(op_code_, instruction_, addressing_, ...) \
            result[op_code_] = Get##addressing_##Length(); \

        CPU6502_ALL_INSTRUCTIONS

        #undef OPERATION

        return result;
    }

    constexpr std::array<const char*, 256> ALL_INSTRUCTION_NAMES       = GetAllInstructionNames();
    constexpr std::array<const char*, 256> ALL_INSTRUCTION_DATA_FORMAT = GetAllInstructionDataFormat();
    constexpr std::array<int, 256>         ALL_INSTRUCTION_LENGTH      = GetAllInstructionLength();

    void CPU6502Disassembly::ShowCPUInfo(std::uint8_t op_code)
    {
        int length = ALL_INSTRUCTION_LENGTH[op_code];
        char data[10];
        switch (length)
        {
            case 1:
                sprintf_s(data, ALL_INSTRUCTION_DATA_FORMAT[op_code]);
                break;
            case 2:
                sprintf_s(data, ALL_INSTRUCTION_DATA_FORMAT[op_code], m_CPU->m_main_bus_read(m_CPU->m_PC));
                break;
            case 3:
                sprintf_s(data, ALL_INSTRUCTION_DATA_FORMAT[op_code],
                    m_CPU->m_main_bus_read(m_CPU->m_PC) | (static_cast<std::uint16_t>(m_CPU->m_main_bus_read(m_CPU->m_PC + 1)) << 8));
                break;
            default:
                break; // 没这种情况
        }

        printf("$%04X  %s %-10s A : %02X, X : %02X, Y : %02X, P : %02X SP : %02X\n", m_CPU->m_PC - 1, 
            ALL_INSTRUCTION_NAMES[op_code], data, m_CPU->m_A, m_CPU->m_X, m_CPU->m_Y, m_CPU->m_P, m_CPU->m_SP);
    }
}
