#pragma once

#include "def.h"

namespace nes
{
    class CPU6502;

    class NesLog
    {
    public:
        static NesLog& GetInstance();
        // 无脑传两个数
        void ShowCPULog(CPU6502* cpu, uint16 address, byte op_code, byte op_num1, byte op_num2);

    private:
        NesLog() {}
    };
}
