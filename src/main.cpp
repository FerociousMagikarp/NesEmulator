#include <iostream>
#include <memory>
#include "cartridge.h"
#include "command_line.h"
#include "emulator.h"

int main(int argc, char *argv[])
{
    if (!CommandLine::GetInstance().Init(argc, argv))
    {
        std::cout << CommandLine::GetInstance().GetError() << std::endl;
        return 0;
    }
    
    // 加载卡带
    std::unique_ptr<nes::Cartridge> cartridge = std::make_unique<nes::Cartridge>();
    if (!cartridge->LoadFromFile(CommandLine::GetInstance().GetNesPath().c_str()))
    {
        std::cout << "Unable to load nes file : " << CommandLine::GetInstance().GetNesPath() << std::endl;
        return 0;
    }

    // 卡带插入机器中
    nes::NesEmulator emulator;
    emulator.PutInCartridge(std::move(cartridge));
    emulator.Run();

    return 0;
}
