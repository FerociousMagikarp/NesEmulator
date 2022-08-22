#pragma once

#include "def.h"

namespace nes
{
    class CPU6502
    {
    public:
        CPU6502() noexcept;
        ~CPU6502();

    private:
        // 程序计数器
        uint16 m_PC = 0;
        // 栈顶指针，地址为0x0100+S
        uint8 m_S = 0;
        // 处理器状态， N V 1 B D I Z C
        uint8 m_P = 0;
        // 累加器
        uint8 m_A = 0;
        // X变址寄存器
        uint8 m_X = 0;
        // Y变址寄存器
        uint8 m_Y = 0;
    };
}
