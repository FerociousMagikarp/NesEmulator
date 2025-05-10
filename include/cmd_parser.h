#pragma once

#include <string>
#include "def.h"

namespace nes_support
{
    // 需要按顺序调用
    bool        CMDParse(int argc, char** argv) noexcept;
    std::string GetCMDError() noexcept; // 这个里面包括了Help
    bool        CheckCMDParam();
    nes::Config CreateConfigFromCMD() noexcept; // 这个接口从CMD命令行仲创建一个配置文件
    void        CMDClear() noexcept; // 清除了以后就不要再调用上面的接口了
}
