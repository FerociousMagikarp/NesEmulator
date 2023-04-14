#include <iostream>
#include <memory>
#include <future>
#include "cartridge.h"
#include "command_line.h"
#include "emulator.h"
#include "sdl_application.h"
#include "virtual_device.h"

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

    std::shared_ptr<nes::VirtualDevice> device = std::make_shared<nes::VirtualDevice>();
    std::unique_ptr<nes::NesEmulator> nes_emulator = std::make_unique<nes::NesEmulator>();
    // 命令行配置参数设置
    CommandLine::GetInstance().Execute(nes_emulator, device);
    
    nes_emulator->SetVirtualDevice(device);
    // 卡带插入机器中
    nes_emulator->PutInCartridge(std::move(cartridge));

    SDLApplication application;
    if (!application.Init(device->GetWidth(), device->GetHeight()))
    {
        std::cout << "Unable to load nes file : " << CommandLine::GetInstance().GetNesPath() << std::endl;
        return 0;
    }
    application.SetVirtualDevice(device);

    bool running = true;

    auto future = std::async(std::launch::async, [emulator = std::move(nes_emulator), &running]()
	{
		emulator->Run(running);
	});

    application.Run(running);

    return 0;
}
