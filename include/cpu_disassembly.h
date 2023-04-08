#pragma once

#include <cstdint>


namespace nes
{
    class CPU6502;

    class CPU6502Disassembly
    {
        public:
            static CPU6502Disassembly& GetInstance()
            {
                static CPU6502Disassembly instance;
                return instance;
            }

            ~CPU6502Disassembly() = default;

            inline void Init(CPU6502* cpu) { m_CPU = cpu; }
            void ShowCPUInfo(std::uint8_t op_code);

        private:
            CPU6502Disassembly() = default;
            CPU6502* m_CPU;
    };
}
