#pragma once

#ifdef _CPU6502_ALL_INSTRUCTIONS_
#undef _CPU6502_ALL_INSTRUCTIONS_
#endif

#ifdef OPERATION
#undef OPERATION
#endif

// 各种指令（有一些额外的就没实现，标记为XXX）
// 参数 ： 指令码，指令名，寻址类型，指令周期，跳页周期+1（不填为否）
#define CPU6502_ALL_INSTRUCTIONS \
OPERATION(0x00, BRK, Implied,     0)  /*这里其实是把周期加在中断里了*/ \
OPERATION(0x01, ORA, IndirectX,   6) \
OPERATION(0x02, XXX, Implied,     0) \
OPERATION(0x03, SLO, IndirectX,   8) \
OPERATION(0x04, NOP, ZeroPage,    3) \
OPERATION(0x05, ORA, ZeroPage,    3) \
OPERATION(0x06, ASL, ZeroPage,    5) \
OPERATION(0x07, SLO, ZeroPage,    5) \
OPERATION(0x08, PHP, Implied,     3) \
OPERATION(0x09, ORA, Immediate,   2) \
OPERATION(0x0a, ASL, Accumulator, 2) \
OPERATION(0x0b, ANC, Immediate,   2) \
OPERATION(0x0c, NOP, Absolute,    4) \
OPERATION(0x0d, ORA, Absolute,    4) \
OPERATION(0x0e, ASL, Absolute,    6) \
OPERATION(0x0f, SLO, Absolute,    6) \
OPERATION(0x10, BPL, Relative,    2) \
OPERATION(0x11, ORA, IndirectY,   5, 1) \
OPERATION(0x12, XXX, Implied,     0) \
OPERATION(0x13, SLO, IndirectY,   8) \
OPERATION(0x14, NOP, ZeroPageX,   4) \
OPERATION(0x15, ORA, ZeroPageX,   4) \
OPERATION(0x16, ASL, ZeroPageX,   6) \
OPERATION(0x17, SLO, ZeroPageX,   6) \
OPERATION(0x18, CLC, Implied,     2) \
OPERATION(0x19, ORA, AbsoluteY,   4, 1) \
OPERATION(0x1a, NOP, Implied,     2) \
OPERATION(0x1b, SLO, AbsoluteY,   7) \
OPERATION(0x1c, NOP, AbsoluteX,   4, 1) \
OPERATION(0x1d, ORA, AbsoluteX,   4, 1) \
OPERATION(0x1e, ASL, AbsoluteX,   7) \
OPERATION(0x1f, SLO, AbsoluteX,   7) \
OPERATION(0x20, JSR, Absolute,    6) \
OPERATION(0x21, AND, IndirectX,   6) \
OPERATION(0x22, XXX, Implied,     0) \
OPERATION(0x23, RLA, IndirectX,   8) \
OPERATION(0x24, BIT, ZeroPage,    3) \
OPERATION(0x25, AND, ZeroPage,    3) \
OPERATION(0x26, ROL, ZeroPage,    5) \
OPERATION(0x27, RLA, ZeroPage,    5) \
OPERATION(0x28, PLP, Implied,     4) \
OPERATION(0x29, AND, Immediate,   2) \
OPERATION(0x2a, ROL, Accumulator, 2) \
OPERATION(0x2b, ANC, Immediate,   2) \
OPERATION(0x2c, BIT, Absolute,    4) \
OPERATION(0x2d, AND, Absolute,    4) \
OPERATION(0x2e, ROL, Absolute,    6) \
OPERATION(0x2f, RLA, Absolute,    6) \
OPERATION(0x30, BMI, Relative,    2) \
OPERATION(0x31, AND, IndirectY,   5, 1) \
OPERATION(0x32, XXX, Implied,     0) \
OPERATION(0x33, RLA, IndirectY,   8) \
OPERATION(0x34, NOP, ZeroPageX,   4) \
OPERATION(0x35, AND, ZeroPageX,   4) \
OPERATION(0x36, ROL, ZeroPageX,   6) \
OPERATION(0x37, RLA, ZeroPageX,   6) \
OPERATION(0x38, SEC, Implied,     2) \
OPERATION(0x39, AND, AbsoluteY,   4, 1) \
OPERATION(0x3a, NOP, Implied,     2) \
OPERATION(0x3b, RLA, AbsoluteY,   7) \
OPERATION(0x3c, NOP, AbsoluteX,   4, 1) \
OPERATION(0x3d, AND, AbsoluteX,   4, 1) \
OPERATION(0x3e, ROL, AbsoluteX,   7) \
OPERATION(0x3f, RLA, AbsoluteX,   7) \
OPERATION(0x40, RTI, Implied,     6) \
OPERATION(0x41, EOR, IndirectX,   6) \
OPERATION(0x42, XXX, Implied,     0) \
OPERATION(0x43, SRE, IndirectX,   8) \
OPERATION(0x44, NOP, ZeroPage,    4) \
OPERATION(0x45, EOR, ZeroPage,    3) \
OPERATION(0x46, LSR, ZeroPage,    5) \
OPERATION(0x47, SRE, ZeroPage,    5) \
OPERATION(0x48, PHA, Implied,     3) \
OPERATION(0x49, EOR, Immediate,   2) \
OPERATION(0x4a, LSR, Accumulator, 2) \
OPERATION(0x4b, ALR, Immediate,   2) \
OPERATION(0x4c, JMP, Absolute,    3) \
OPERATION(0x4d, EOR, Absolute,    4) \
OPERATION(0x4e, LSR, Absolute,    6) \
OPERATION(0x4f, SRE, Absolute,    6) \
OPERATION(0x50, BVC, Relative,    2) \
OPERATION(0x51, EOR, IndirectY,   5, 1) \
OPERATION(0x52, XXX, Implied,     0) \
OPERATION(0x53, SRE, IndirectY,   8) \
OPERATION(0x54, NOP, ZeroPageX,   4) \
OPERATION(0x55, EOR, ZeroPageX,   4) \
OPERATION(0x56, LSR, ZeroPageX,   6) \
OPERATION(0x57, SRE, ZeroPageX,   6) \
OPERATION(0x58, CLI, Implied,     2) \
OPERATION(0x59, EOR, AbsoluteY,   4, 1) \
OPERATION(0x5a, NOP, Implied,     2) \
OPERATION(0x5b, SRE, AbsoluteY,   7) \
OPERATION(0x5c, NOP, AbsoluteX,   4, 1) \
OPERATION(0x5d, EOR, AbsoluteX,   4, 1) \
OPERATION(0x5e, LSR, AbsoluteX,   7) \
OPERATION(0x5f, SRE, AbsoluteX,   7) \
OPERATION(0x60, RTS, Implied,     6) \
OPERATION(0x61, ADC, IndirectX,   6) \
OPERATION(0x62, XXX, Implied,     0) \
OPERATION(0x63, RRA, IndirectX,   8) \
OPERATION(0x64, NOP, ZeroPage,    3) \
OPERATION(0x65, ADC, ZeroPage,    3) \
OPERATION(0x66, ROR, ZeroPage,    5) \
OPERATION(0x67, RRA, ZeroPage,    5) \
OPERATION(0x68, PLA, Implied,     4) \
OPERATION(0x69, ADC, Immediate,   2) \
OPERATION(0x6a, ROR, Accumulator, 2) \
OPERATION(0x6b, ARR, Immediate,   2) \
OPERATION(0x6c, JMP, Indirect,    5) \
OPERATION(0x6d, ADC, Absolute,    4) \
OPERATION(0x6e, ROR, Absolute,    6) \
OPERATION(0x6f, RRA, Absolute,    6) \
OPERATION(0x70, BVS, Relative,    2) \
OPERATION(0x71, ADC, IndirectY,   5, 1) \
OPERATION(0x72, XXX, Implied,     0) \
OPERATION(0x73, RRA, IndirectY,   8) \
OPERATION(0x74, NOP, ZeroPageX,   4) \
OPERATION(0x75, ADC, ZeroPageX,   4) \
OPERATION(0x76, ROR, ZeroPageX,   6) \
OPERATION(0x77, RRA, ZeroPageX,   6) \
OPERATION(0x78, SEI, Implied,     2) \
OPERATION(0x79, ADC, AbsoluteY,   4, 1) \
OPERATION(0x7a, NOP, Implied,     2) \
OPERATION(0x7b, RRA, AbsoluteY,   7) \
OPERATION(0x7c, NOP, AbsoluteX,   4, 1) \
OPERATION(0x7d, ADC, AbsoluteX,   4, 1) \
OPERATION(0x7e, ROR, AbsoluteX,   7) \
OPERATION(0x7f, RRA, AbsoluteX,   7) \
OPERATION(0x80, NOP, Immediate,   2) \
OPERATION(0x81, STA, IndirectX,   6) \
OPERATION(0x82, NOP, Immediate,   2) \
OPERATION(0x83, SAX, IndirectX,   6) \
OPERATION(0x84, STY, ZeroPage,    3) \
OPERATION(0x85, STA, ZeroPage,    3) \
OPERATION(0x86, STX, ZeroPage,    3) \
OPERATION(0x87, SAX, ZeroPage,    3) \
OPERATION(0x88, DEY, Implied,     2) \
OPERATION(0x89, NOP, Immediate,   2) \
OPERATION(0x8a, TXA, Implied,     2) \
OPERATION(0x8b, XAA, Immediate,   2) \
OPERATION(0x8c, STY, Absolute,    4) \
OPERATION(0x8d, STA, Absolute,    4) \
OPERATION(0x8e, STX, Absolute,    4) \
OPERATION(0x8f, SAX, Absolute,    4) \
OPERATION(0x90, BCC, Relative,    2) \
OPERATION(0x91, STA, IndirectY,   6) \
OPERATION(0x92, XXX, Implied,     0) \
OPERATION(0x93, AHX, IndirectY,   6) \
OPERATION(0x94, STY, ZeroPageX,   4) \
OPERATION(0x95, STA, ZeroPageX,   4) \
OPERATION(0x96, STX, ZeroPageY,   4) \
OPERATION(0x97, SAX, ZeroPageY,   4) \
OPERATION(0x98, TYA, Implied,     2) \
OPERATION(0x99, STA, AbsoluteY,   5) \
OPERATION(0x9a, TXS, Implied,     2) \
OPERATION(0x9b, TAS, AbsoluteY,   5) \
OPERATION(0x9c, SHY, AbsoluteX,   5) \
OPERATION(0x9d, STA, AbsoluteX,   5) \
OPERATION(0x9e, SHX, AbsoluteY,   5) \
OPERATION(0x9f, AHX, AbsoluteY,   5) \
OPERATION(0xa0, LDY, Immediate,   2) \
OPERATION(0xa1, LDA, IndirectX,   6) \
OPERATION(0xa2, LDX, Immediate,   2) \
OPERATION(0xa3, LAX, IndirectX,   6) \
OPERATION(0xa4, LDY, ZeroPage,    3) \
OPERATION(0xa5, LDA, ZeroPage,    3) \
OPERATION(0xa6, LDX, ZeroPage,    3) \
OPERATION(0xa7, LAX, ZeroPage,    3) \
OPERATION(0xa8, TAY, Implied,     2) \
OPERATION(0xa9, LDA, Immediate,   2) \
OPERATION(0xaa, TAX, Implied,     2) \
OPERATION(0xab, LAX, Immediate,   2) \
OPERATION(0xac, LDY, Absolute,    4) \
OPERATION(0xad, LDA, Absolute,    4) \
OPERATION(0xae, LDX, Absolute,    4) \
OPERATION(0xaf, LAX, Absolute,    4) \
OPERATION(0xb0, BCS, Relative,    2) \
OPERATION(0xb1, LDA, IndirectY,   5, 1) \
OPERATION(0xb2, XXX, Implied,     0) \
OPERATION(0xb3, LAX, IndirectY,   5, 1) \
OPERATION(0xb4, LDY, ZeroPageX,   4) \
OPERATION(0xb5, LDA, ZeroPageX,   4) \
OPERATION(0xb6, LDX, ZeroPageY,   4) \
OPERATION(0xb7, LAX, ZeroPageY,   4) \
OPERATION(0xb8, CLV, Implied,     2) \
OPERATION(0xb9, LDA, AbsoluteY,   4, 1) \
OPERATION(0xba, TSX, Implied,     2) \
OPERATION(0xbb, LAS, AbsoluteY,   4, 1) \
OPERATION(0xbc, LDY, AbsoluteX,   4, 1) \
OPERATION(0xbd, LDA, AbsoluteX,   4, 1) \
OPERATION(0xbe, LDX, AbsoluteY,   4, 1) \
OPERATION(0xbf, LAX, AbsoluteY,   4, 1) \
OPERATION(0xc0, CPY, Immediate,   2) \
OPERATION(0xc1, CMP, IndirectX,   6) \
OPERATION(0xc2, NOP, Immediate,   2) \
OPERATION(0xc3, DCP, IndirectX,   8) \
OPERATION(0xc4, CPY, ZeroPage,    3) \
OPERATION(0xc5, CMP, ZeroPage,    3) \
OPERATION(0xc6, DEC, ZeroPage,    5) \
OPERATION(0xc7, DCP, ZeroPage,    5) \
OPERATION(0xc8, INY, Implied,     2) \
OPERATION(0xc9, CMP, Immediate,   2) \
OPERATION(0xca, DEX, Implied,     2) \
OPERATION(0xcb, AXS, Immediate,   2) \
OPERATION(0xcc, CPY, Absolute,    4) \
OPERATION(0xcd, CMP, Absolute,    4) \
OPERATION(0xce, DEC, Absolute,    6) \
OPERATION(0xcf, DCP, Absolute,    6) \
OPERATION(0xd0, BNE, Relative,    2) \
OPERATION(0xd1, CMP, IndirectY,   5, 1) \
OPERATION(0xd2, XXX, Implied,     0) \
OPERATION(0xd3, DCP, IndirectY,   8) \
OPERATION(0xd4, NOP, ZeroPageX,   4) \
OPERATION(0xd5, CMP, ZeroPageX,   4) \
OPERATION(0xd6, DEC, ZeroPageX,   6) \
OPERATION(0xd7, DCP, ZeroPageX,   6) \
OPERATION(0xd8, CLD, Implied,     2) \
OPERATION(0xd9, CMP, AbsoluteY,   4, 1) \
OPERATION(0xda, NOP, Implied,     2) \
OPERATION(0xdb, DCP, AbsoluteY,   7) \
OPERATION(0xdc, NOP, AbsoluteX,   4) \
OPERATION(0xdd, CMP, AbsoluteX,   4, 1) \
OPERATION(0xde, DEC, AbsoluteX,   7) \
OPERATION(0xdf, DCP, AbsoluteX,   7) \
OPERATION(0xe0, CPX, Immediate,   2) \
OPERATION(0xe1, SBC, IndirectX,   6) \
OPERATION(0xe2, NOP, Immediate,   2) \
OPERATION(0xe3, ISC, IndirectX,   8) \
OPERATION(0xe4, CPX, ZeroPage,    3) \
OPERATION(0xe5, SBC, ZeroPage,    3) \
OPERATION(0xe6, INC, ZeroPage,    5) \
OPERATION(0xe7, ISC, ZeroPage,    5) \
OPERATION(0xe8, INX, Implied,     2) \
OPERATION(0xe9, SBC, Immediate,   2) \
OPERATION(0xea, NOP, Implied,     2) \
OPERATION(0xeb, SBC, Immediate,   2) \
OPERATION(0xec, CPX, Absolute,    4) \
OPERATION(0xed, SBC, Absolute,    4) \
OPERATION(0xee, INC, Absolute,    6) \
OPERATION(0xef, ISC, Absolute,    6) \
OPERATION(0xf0, BEQ, Relative,    2) \
OPERATION(0xf1, SBC, IndirectY,   5, 1) \
OPERATION(0xf2, XXX, Implied,     0) \
OPERATION(0xf3, ISC, IndirectY,   8) \
OPERATION(0xf4, NOP, ZeroPageX,   4) \
OPERATION(0xf5, SBC, ZeroPageX,   4) \
OPERATION(0xf6, INC, ZeroPageX,   6) \
OPERATION(0xf7, ISC, ZeroPageX,   6) \
OPERATION(0xf8, SED, Implied,     2) \
OPERATION(0xf9, SBC, AbsoluteY,   4, 1) \
OPERATION(0xfa, NOP, Implied,     2) \
OPERATION(0xfb, ISC, AbsoluteY,   7) \
OPERATION(0xfc, NOP, AbsoluteX,   4, 1) \
OPERATION(0xfd, SBC, AbsoluteX,   4, 1) \
OPERATION(0xfe, INC, AbsoluteX,   7) \
OPERATION(0xff, ISC, AbsoluteX,   7) \


