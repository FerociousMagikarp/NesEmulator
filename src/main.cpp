#include <iostream>
#include <memory>
#include <future>
#include "cartridge.h"
#include "cmd_parser.h"
#include "emulator.h"
#include "def.h"
#include "sdl_application.h"
#include "virtual_device.h"


int main(int argc, char *argv[])
{
    if (!nes_support::CMDParse(argc, argv))
    {
        std::cout << nes_support::GetCMDError() << std::endl;
        return 0;
    }
    
    if (!nes_support::CheckCMDParam())
    {
        return 0;
    }

    // 加载配置
    auto config = nes_support::CreateConfigFromCMD();
    nes_support::CMDClear();

    // 加载卡带
    std::unique_ptr<nes::Cartridge> cartridge = std::make_unique<nes::Cartridge>();
    if (!cartridge->LoadFromFile(config.RomPath))
    {
        std::cout << "Unable to load nes file : " << config.RomPath << std::endl;
        return 0;
    }

    std::shared_ptr<nes::VirtualDevice> device = std::make_shared<nes::VirtualDevice>();
    std::shared_ptr<nes::NesEmulator> nes_emulator = std::make_shared<nes::NesEmulator>();
    
    // 根据配置参数设置
    device->SetScale(config.Base.Scale);
    
    nes_emulator->SetVirtualDevice(device);
    // 卡带插入机器中
    nes_emulator->PutInCartridge(std::move(cartridge));

    SDLApplication application(device, nes_emulator);
    if (!application.Init(device->GetWidth(), device->GetHeight()))
    {
        std::cout << "Application initialize failed!" << std::endl;
        return 0;
    }
    application.SetConfig(config);

    bool running = true;

    auto future = std::async(std::launch::async, [emulator = std::move(nes_emulator), &running]()
	{
		emulator->Run(running);
	});

    application.Run(running);
    application.Terminate();

    return 0;
}
