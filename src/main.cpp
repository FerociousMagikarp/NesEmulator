#include <iostream>
#include <memory>
#include <future>
#include "cartridge.h"
#include "command_line.h"
#include "emulator.h"
#include "def.h"
#include "sdl_application.h"
#include "virtual_device.h"

void SetInputControlConfig(SDLApplication& app, const nes::InputConfig config, nes::Player player)
{
    app.SetControl(config.A, nes::InputKey::A, player);
    app.SetControl(config.B, nes::InputKey::B, player);
    app.SetControl(config.Select, nes::InputKey::Select, player);
    app.SetControl(config.Start, nes::InputKey::Start, player);
    app.SetControl(config.Up, nes::InputKey::Up, player);
    app.SetControl(config.Down, nes::InputKey::Down, player);
    app.SetControl(config.Left, nes::InputKey::Left, player);
    app.SetControl(config.Right, nes::InputKey::Right, player);
    app.SetControl(config.TurboA, nes::InputKey::TurboA, player);
    app.SetControl(config.TurboB, nes::InputKey::TurboB, player);
}

void SetEmulatorControlConfig(SDLApplication& app, const nes::FuncConfig& config)
{
    app.SetEmulatorControl(config.Save, nes::EmulatorOperation::Save);
    app.SetEmulatorControl(config.Load, nes::EmulatorOperation::Load);
}

int main(int argc, char *argv[])
{
    if (!CommandLine::GetInstance().Init(argc, argv))
    {
        std::cout << CommandLine::GetInstance().GetError() << std::endl;
        return 0;
    }
    
    // 加载配置
    nes::Config config;

    // 加载卡带
    std::unique_ptr<nes::Cartridge> cartridge = std::make_unique<nes::Cartridge>();
    if (!cartridge->LoadFromFile(CommandLine::GetInstance().GetNesPath().c_str()))
    {
        std::cout << "Unable to load nes file : " << CommandLine::GetInstance().GetNesPath() << std::endl;
        return 0;
    }

    std::shared_ptr<nes::VirtualDevice> device = std::make_shared<nes::VirtualDevice>();
    std::shared_ptr<nes::NesEmulator> nes_emulator = std::make_shared<nes::NesEmulator>();
    // 命令行配置参数设置
    CommandLine::GetInstance().Execute(nes_emulator, device);
    
    nes_emulator->SetVirtualDevice(device);
    // 卡带插入机器中
    nes_emulator->PutInCartridge(std::move(cartridge));

    SDLApplication application(device, nes_emulator);
    if (!application.Init(device->GetWidth(), device->GetHeight()))
    {
        std::cout << "Application initialize failed!" << std::endl;
        return 0;
    }
    application.SetJoystickDeadZone(config.Base.JoystickDeadZone);
    SetInputControlConfig(application, config.Player1, nes::Player::Player1);
    SetInputControlConfig(application, config.Player2, nes::Player::Player2);
    SetEmulatorControlConfig(application, config.ShortcutKeys);

    bool running = true;

    auto future = std::async(std::launch::async, [emulator = std::move(nes_emulator), &running]()
	{
		emulator->Run(running);
	});

    application.Run(running);
    application.Terminate();

    return 0;
}
