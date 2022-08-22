#include "cpu.h"
#include <iostream>

namespace nes
{
    CPU6502::CPU6502()
    {
        std::cout << sizeof(*this) << std::endl;
    }

    CPU6502::~CPU6502()
    {

    }
}
