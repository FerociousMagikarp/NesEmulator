#pragma once

#include <cstdint>
#include <functional>

namespace nes
{
    enum class CPU6502InterruptType
    {
        IRQ,
        BRK,
        NMI
    };

    constexpr std::uint16_t BRK_VECTOR = 0xfffe;
    constexpr std::uint16_t NMI_VECTOR = 0xfffa;
    constexpr std::uint16_t RESET_ADDRESS = 0xfffc;

    class CPU6502
    {
    public:
        CPU6502();
        ~CPU6502() = default;

        // 初始化
        void Reset();
        // 单步执行
        void Step();
        // 设置中断
        void Interrupt(CPU6502InterruptType type);
        
        inline void PushStack(std::uint8_t value) { m_main_bus_write(0x100 | m_SP--, value); }
        inline std::uint8_t PullStack() { return m_main_bus_read(0x100 | ++m_SP); }

        // 设置从总线读写ram或者mapper的回调
        inline void SetReadFunction(std::function<std::uint8_t(std::uint16_t)>&& callback) { m_main_bus_read = std::move(callback); }
        inline void SetWriteFunction(std::function<void(std::uint16_t, std::uint8_t)>&& callback) { m_main_bus_write = std::move(callback); }

        void SkipOAMDMACycle();

        // 获取状态寄存器
        inline bool GetC() const { return m_P & 0x01; }
        inline bool GetZ() const { return m_P & 0x02; }
        inline bool GetI() const { return m_P & 0x04; }
        inline bool GetB() const { return m_P & 0x10; }
        inline bool GetV() const { return m_P & 0x40; }
        inline bool GetN() const { return m_P & 0x80; }
        inline void SetFlag(std::uint8_t value, bool set)
        {
            if (set) m_P |= value;
            else     m_P &= ~value;
        }

    private:
        std::uint16_t ReadAddress(std::uint16_t start_address);
        void InterruptExecute(CPU6502InterruptType type);
        void ExecuteCode(std::uint8_t op_code);

        template<typename Addr, typename R, typename... Args>
        void CombineAddressingAndInstruction(R(CPU6502::*)(Args...), Addr);

        template<typename Addr, typename R, typename Arg>
        void CombineAddressingAndInstruction(R(CPU6502::* instruction)(Arg), Addr addr);

        template<typename Addr, typename R>
        void CombineAddressingAndInstruction(R(CPU6502::* instruction)(), Addr);

        // 寻址方式，寻到的值放到src里，返回值是地址。
        int           Implied();
        std::uint8_t  Immediate();
        std::uint16_t Absolute();
        std::uint16_t ZeroPage();
        std::uint8_t  Accumulator();
        std::uint16_t AbsoluteX();
        std::uint16_t AbsoluteY();
        std::uint16_t ZeroPageX();
        std::uint16_t ZeroPageY();
        std::uint16_t Indirect();
        std::uint16_t IndirectX();
        std::uint16_t IndirectY();
        std::uint16_t Relative();

        // 所有指令
        // 根据寻址模式通过编译期选择对应的函数，所以参数和返回值会有所不同
        void         ADC(std::uint8_t  src);
        void         AND(std::uint8_t  src);
        std::uint8_t ASL(std::uint8_t  src);
        void         BCC(std::uint16_t addr);
        void         BCS(std::uint16_t addr);
        void         BEQ(std::uint16_t addr);
        void         BIT(std::uint8_t  src);
        void         BMI(std::uint16_t addr);
        void         BNE(std::uint16_t addr);
        void         BPL(std::uint16_t addr);
        void         BRK();
        void         BVC(std::uint16_t addr);
        void         BVS(std::uint16_t addr);
        void         CLC();
        void         CLD();
        void         CLI();
        void         CLV();
        void         CMP(std::uint8_t  src);
        void         CPX(std::uint8_t  src);
        void         CPY(std::uint8_t  src);
        std::uint8_t DEC(std::uint8_t  src);
        void         DEX();
        void         DEY();
        void         EOR(std::uint8_t  src);
        std::uint8_t INC(std::uint8_t  src);
        void         INX();
        void         INY();
        void         JMP(std::uint16_t addr);
        void         JSR(std::uint16_t addr);
        void         LDA(std::uint8_t  src);
        void         LDX(std::uint8_t  src);
        void         LDY(std::uint8_t  src);
        std::uint8_t LSR(std::uint8_t  src);
        void         NOP();
        void         ORA(std::uint8_t  src);
        void         PHA();
        void         PHP();
        void         PLA();
        void         PLP();
        std::uint8_t ROL(std::uint8_t  src);
        std::uint8_t ROR(std::uint8_t  src);
        void         RTI();
        void         RTS();
        void         SBC(std::uint8_t  src);
        void         SEC();
        void         SED();
        void         SEI();
        std::uint8_t STA();
        std::uint8_t STX();
        std::uint8_t STY();
        void         TAX();
        void         TAY();
        void         TSX();
        void         TXA();
        void         TXS();
        void         TYA();

        // 扩展用的指令及寻址方式
        inline void  XXX() {}

        // 未记录指令
        std::uint8_t SLO(std::uint8_t src);
        std::uint8_t RLA(std::uint8_t src);
        std::uint8_t SRE(std::uint8_t src);
        std::uint8_t RRA(std::uint8_t src);
        std::uint8_t SAX();
        void         LAX(std::uint8_t src);
        std::uint8_t DCP(std::uint8_t src);
        std::uint8_t ISC(std::uint8_t src);

        // 其余指令不做实现
        inline void  ANC() {}
        inline void  ALR() {}
        inline void  ARR() {}
        inline void  XAA() {}
        inline void  AXS() {}
        inline void  AHX() {}
        inline void  SHY() {}
        inline void  SHX() {}
        inline void  TAS() {}
        inline void  LAS() {}

    private:
        // 程序计数器
        std::uint16_t m_PC = 0;
        // 栈顶指针，地址为0x0100+S
        std::uint8_t m_SP = 0xfd;
        // 处理器状态， N V 1 B D I Z C
        std::uint8_t m_P = 0x24;
        // 累加器
        std::uint8_t m_A = 0;
        // X变址寄存器
        std::uint8_t m_X = 0;
        // Y变址寄存器
        std::uint8_t m_Y = 0;

        std::uint8_t m_current_interrupt = 0;

        // 需要跳过的周期数
        std::uint16_t m_skip_cycles = 0;
        // 总的周期数
        std::uint32_t m_cycles = 0;

        std::function<std::uint8_t(std::uint16_t)> m_main_bus_read;
        std::function<void(std::uint16_t, std::uint8_t)> m_main_bus_write;

        friend class CPU6502Disassembly;
    };
}
