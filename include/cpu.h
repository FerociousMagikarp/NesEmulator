#pragma once

#include "def.h"
#include <functional>

namespace nes
{
    constexpr uint16 BRK_VECTOR = 0xfffe;
    constexpr uint16 NMI_VECTOR = 0xfffa;
    constexpr uint16 RESET_ADDRESS = 0xfffc;

    class CPU6502
    {
    public:
        CPU6502() noexcept;
        ~CPU6502();

        // 初始化
        void Reset();
        // 设置中断
        void Interrupt(CPU6502InterruptType type);
        
        inline void PushStack(byte value) { m_write_function(0x100 | m_SP--, value); }
        inline byte PullStack() { return m_read_function(0x100 | ++m_SP); }

        // 设置从总线读写ram或者mapper的回调
        inline void SetReadFunction(std::function<byte(uint16)>&& callback) { m_read_function = std::move(callback); }
        inline void SetWriteFunction(std::function<void(uint16, byte)>&& callback) { m_write_function = std::move(callback); }

        // 获取状态寄存器
        inline bool GetC() { return m_P & 0x01; }
        inline bool GetZ() { return m_P & 0x02; }
        inline bool GetI() { return m_P & 0x04; }
        inline bool GetB() { return m_P & 0x10; }
        inline bool GetV() { return m_P & 0x40; }
        inline bool GetN() { return m_P & 0x80; }
        inline void SetFlag(uint8 value, bool set)
        {
            if (set) m_SP |= value;
            else     m_SP &= ~value;
        }

    private:
        uint16 ReadAddress(uint16 start_address);
        void InterruptExecute(CPU6502InterruptType type);

        // 所有指令，返回值表示是否把结果存回去
        bool ADC();
        bool AND();
        bool ASL();
        bool BCC();
        bool BCS();
        bool BEQ();
        bool BIT();
        bool BMI();
        bool BNE();
        bool BPL();
        bool BRK();
        bool BVC();
        bool BVS();
        bool CLC();
        bool CLD();
        bool CLI();
        bool CLV();
        bool CMP();
        bool CPX();
        bool CPY();
        bool DEC();
        bool DEX();
        bool DEY();
        bool EOR();
        bool INC();
        bool INX();
        bool INY();
        bool JMP();
        bool JSR();
        bool LDA();
        bool LDX();
        bool LDY();
        bool LSR();
        bool NOP();
        bool ORA();
        bool PHA();
        bool PHP();
        bool PLA();
        bool PLP();
        bool ROL();
        bool ROR();
        bool RTI();
        bool RTS();
        bool SBC();
        bool SEC();
        bool SED();
        bool SEI();
        bool STA();
        bool STX();
        bool STY();
        bool TAX();
        bool TAY();
        bool TSX();
        bool TXA();
        bool TXS();
        bool TYA();


    private:
        // 程序计数器
        uint16 m_PC = 0;
        // 栈顶指针，地址为0x0100+S
        uint8 m_SP = 0xfd;
        // 处理器状态， N V 1 B D I Z C
        uint8 m_P = 0x24;
        // 累加器
        uint8 m_A = 0;
        // X变址寄存器
        uint8 m_X = 0;
        // Y变址寄存器
        uint8 m_Y = 0;

        // 正好差一字节对齐，弄个中断记录
        uint8 m_current_interrupt = 0;

        // 寻址寻出来的数放这了
        uint16 m_src = 0;
        // 需要跳过的周期数
        uint16 m_skip_cycles = 0;
        // 总的周期数
        uint32 m_cycles = 0;

        std::function<byte(uint16)> m_read_function;
        std::function<void(uint16, byte)> m_write_function;
    };
}
