#include "cmd_parser.h"
#include "extern/ct_cmd.hpp"
#include "ini_parser.h"
#include <filesystem>
#include <iostream>
#include <concepts>
#include <charconv>


namespace nes_support
{
    constexpr std::string_view PROGRAM_NAME = R"(
  _   _                _____                       _           _                  
 | \ | |   ___   ___  | ____|  _ __ ___    _   _  | |   __ _  | |_    ___    _ __ 
 |  \| |  / _ \ / __| |  _|   | '_ ` _ \  | | | | | |  / _` | | __|  / _ \  | '__|
 | |\  | |  __/ \__ \ | |___  | | | | | | | |_| | | | | (_| | | |_  | (_) | | |   
 |_| \_|  \___| |___/ |_____| |_| |_| |_|  \__,_| |_|  \__,_|  \__|  \___/  |_|   
 )";


    struct rom_file
    {
        using value_type = std::string;
        constexpr static bool must = true;
        constexpr static bool ignore_name = true;
        constexpr static std::string_view message = "the nes rom file path";
    };

    struct config_file
    {
        using value_type = std::string;
        constexpr static bool must = true;
        constexpr static char short_name = 'c';
        constexpr static std::string_view message = "the config file path";
        constexpr static std::string_view default_value = "./config.ini";
    };

    // 用一下我搞的这个奇怪的玩具
    auto parser_ptr = ct_cmd::make_parser_ptr<rom_file, config_file>();
    auto ini_parser_ptr = std::make_unique<IniParser>();

    bool CMDParse(int argc, char** argv) noexcept
    {
        bool parse_res = parser_ptr->parse(argc, argv);
        if (!parse_res || parser_ptr->is_help())
            return false;
        return true;
    }

    bool CheckCMDParam()
    {
        if (const auto& rom_file_path = parser_ptr->get<rom_file>();!std::filesystem::exists(rom_file_path))
        {
            std::cout << "Do not exist rom file" << rom_file_path << ", please check." << std::endl;
            return false;
        }

        const auto& config_file_path = parser_ptr->get<config_file>();
        if (!std::filesystem::exists(config_file_path))
        {
            std::cout << "Do not exist config file" << config_file_path << ", please check." << std::endl;
            return false;
        }

        if (!ini_parser_ptr->LoadFromFile(config_file_path))
        {
            std::cout << "Config file" << config_file_path << " load failed, please check." << std::endl;
            return false;
        }

        return true;
    }

    template <typename T>
    void SetValue(T& val, const IniSection& section, std::string_view key_name);

    template <>
    void SetValue<nes::KeyCode>(nes::KeyCode& key_code, const IniSection& section, std::string_view key_name)
    {
        if (!section.ExistValue(key_name))
            return;
        auto val = section.GetValue(key_name);
        auto key_iter = nes::KeyMap.find(val);
        if (key_iter == nes::KeyMap.end())
            return;
        key_code = key_iter->second;
    }

    template <>
    void SetValue<std::string>(std::string& value, const IniSection& section, std::string_view key_name)
    {
        if (!section.ExistValue(key_name))
            return;
        auto val = section.GetValue(key_name);
        value = std::string{val};
    }

    template <typename T>
    concept number_type = std::integral<T> || std::floating_point<T>;

    template <number_type T>
    void SetValue(T& value, const IniSection& section, std::string_view key_name)
    {
        if (!section.ExistValue(key_name))
            return;
        auto val = section.GetValue(key_name);

        T final_value;
        const auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), final_value);
        if (ec == std::errc{})
            value = final_value;
    }

    nes::Config CreateConfigFromCMD() noexcept
    {
        nes::Config config;

        config.RomPath = parser_ptr->get<rom_file>();

        // Player1
        if (ini_parser_ptr->ExistSection("controller1"))
        {
            const auto& section = ini_parser_ptr->GetSection("controller1");
            SetValue(config.Player1.A, section, "button_a");
            SetValue(config.Player1.B, section, "button_b");
            SetValue(config.Player1.TurboA, section, "turbo_a");
            SetValue(config.Player1.TurboB, section, "turbo_b");
            SetValue(config.Player1.Up, section, "up");
            SetValue(config.Player1.Down, section, "down");
            SetValue(config.Player1.Left, section, "left");
            SetValue(config.Player1.Right, section, "right");
            SetValue(config.Player1.Select, section, "select");
            SetValue(config.Player1.Start, section, "start");
        }

        // Player2
        if (ini_parser_ptr->ExistSection("controller2"))
        {
            const auto& section = ini_parser_ptr->GetSection("controller2");
            SetValue(config.Player2.A, section, "button_a");
            SetValue(config.Player2.B, section, "button_b");
            SetValue(config.Player2.TurboA, section, "turbo_a");
            SetValue(config.Player2.TurboB, section, "turbo_b");
            SetValue(config.Player2.Up, section, "up");
            SetValue(config.Player2.Down, section, "down");
            SetValue(config.Player2.Left, section, "left");
            SetValue(config.Player2.Right, section, "right");
            SetValue(config.Player2.Select, section, "select");
            SetValue(config.Player2.Start, section, "start");
        }

        // 快捷键
        if (ini_parser_ptr->ExistSection("shortcut_key"))
        {
            const auto& section = ini_parser_ptr->GetSection("shortcut_key");
            SetValue(config.ShortcutKeys.Load, section, "load");
            SetValue(config.ShortcutKeys.Save, section, "save");
        }

        // 基础设置
        if (ini_parser_ptr->ExistSection("base_config"))
        {
            const auto& section = ini_parser_ptr->GetSection("base_config");
            SetValue(config.Base.Scale, section, "scale");
            config.Base.Scale = std::clamp(config.Base.Scale, 1, 10);
            SetValue(config.Base.JoystickDeadZone, section, "joystick_deadzone");
        }

        return config;
    }

    consteval std::string_view GetCompileYear()
    {
        std::string_view res{__DATE__};
        res.remove_prefix(7);
        return res;
    }

    std::string GetCMDError() noexcept
    {
        if (parser_ptr->is_help())
        {
            auto msg = parser_ptr->generate_help_message();
            constexpr std::string_view COMPILE_YEAR = GetCompileYear();
            std::string head = std::format("{}\nMIT License\nCopyright (c) 2023 - {} FerociousMagikarp\n\n", PROGRAM_NAME, COMPILE_YEAR);
            return head + msg;
        }
        else
        {
            return std::string{parser_ptr->get_error()};
        }
    }

    void CMDClear() noexcept
    {
        parser_ptr = nullptr;
        ini_parser_ptr = nullptr;
    }
}
