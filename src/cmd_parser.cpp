#include "cmd_parser.h"
#include "ini_parser.h"
#include <filesystem>
#include <iostream>
#include <concepts>
#include <charconv>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <algorithm>


namespace nes_support
{
    constexpr std::string_view PROGRAM_NAME = R"(
  _   _                _____                       _           _                  
 | \ | |   ___   ___  | ____|  _ __ ___    _   _  | |   __ _  | |_    ___    _ __ 
 |  \| |  / _ \ / __| |  _|   | '_ ` _ \  | | | | | |  / _` | | __|  / _ \  | '__|
 | |\  | |  __/ \__ \ | |___  | | | | | | | |_| | | | | (_| | | |_  | (_) | | |   
 |_| \_|  \___| |___/ |_____| |_| |_| |_|  \__,_| |_|  \__,_|  \__|  \___/  |_|   
 )";

    struct CMDArg
    {
        std::string_view name = "";
        std::string_view message = "";
        std::string_view default_val = ""; // 这里用字符串存一下，真正用的时候再转出去
        std::string_view val = "";
        char short_name = '\0';
        bool has_set_val = false;
    };

    class CMDParser
    {
        public:
            std::unordered_map<std::string_view, CMDArg> m_args;
            std::unordered_map<char, std::string_view> m_short_full_map;
            std::vector<std::string_view> m_must_args;
            std::string_view m_ignore_name = "";
            std::string_view m_program_name = "";
            std::string m_error = "";

            void AddParam(std::string_view name, char short_name = '\0', std::string_view message = "", std::string_view default_val = "", bool must = false, bool ignore = false)
            {
                assert(!name.empty());
                assert(!m_args.contains(name));
                assert(!m_short_full_map.contains(short_name));
                auto [iter, success] = m_args.insert(std::make_pair(name, CMDArg{}));
                if (!success)
                    return;
                auto& arg = iter->second;
                arg.name = name;
                arg.short_name = short_name;
                arg.message = message;
                arg.default_val = default_val;
                if (short_name != '\0')
                    m_short_full_map[short_name] = name;
                if (must)
                    m_must_args.push_back(name);
                if (ignore)
                    m_ignore_name = name;
            }

            std::string_view Get(std::string_view name)
            {
                auto iter = m_args.find(name);
                if (iter == m_args.end())
                    return "";
                if (!iter->second.has_set_val)
                    return iter->second.default_val;
                return iter->second.val;
            }

            bool Exist(std::string_view name)
            {
                auto iter = m_args.find(name);
                if (iter == m_args.end())
                    return false;
                return iter->second.has_set_val;
            }

            bool Parse(int argc, char** argv)
            {
                if (argc == 0)
                    return false;

                m_program_name = argv[0];

                std::string_view last_name = m_ignore_name;

                for (int i = 1; i < argc; i++)
                {
                    std::string_view arg(argv[i]);
                    if (arg.empty())
                    {
                        return false;
                    }
                    if (arg[0] != '-') // 参数
                    {
                        auto iter = m_args.find(last_name);
                        if (iter != m_args.end())
                        {
                            iter->second.val = arg;
                            iter->second.has_set_val = true;
                        }
                        last_name = m_ignore_name;
                    }
                    else if (arg.size() >= 2 && arg[1] == '-') // 长参数名
                    {
                        arg.remove_prefix(2);
                        if (!m_args.contains(arg))
                        {
                            m_error = std::string{"Undefined option: --"} + arg.data() + "\n";
                            return false;
                        }
                        last_name = arg;

                        auto arg_iter = m_args.find(last_name);
                        assert(arg_iter != m_args.end());
                        arg_iter->second.has_set_val = true;
                    }
                    else // 短参数名
                    {
                        arg.remove_prefix(1);
                        if (arg.size() != 1 || !m_short_full_map.contains(arg[0]))
                        {
                            m_error = std::string{"Undefined option short name: -"} + arg.data() + "\n";
                            return false;
                        }
                        auto iter = m_short_full_map.find(arg[0]);
                        last_name = iter->second;

                        auto arg_iter = m_args.find(last_name);
                        assert(arg_iter != m_args.end());
                        arg_iter->second.has_set_val = true;
                    }
                }

                // 检查一下must参数是不是都设置了
                for (const auto name : m_must_args)
                {
                    assert(m_args.contains(name));
                    auto iter = m_args.find(name);
                    if (!iter->second.has_set_val && iter->second.default_val.empty())
                    {
                        m_error += "Must set argument --";
                        m_error += name;
                        m_error += '\n';
                    }
                }
                if (!m_error.empty())
                    return false;

                return true;
            }

    };

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
    auto parser_ptr = std::make_unique<CMDParser>();
    auto ini_parser_ptr = std::make_unique<IniParser>();

    bool CMDParse(int argc, char** argv) noexcept
    {
        parser_ptr->AddParam("help", '?', "show help");
        parser_ptr->AddParam("rom_file", '\0', "the nes rom file path", "", true, true);
        parser_ptr->AddParam("config_file", 'c', "the config file path", "./config.ini", true);

        bool parse_res = parser_ptr->Parse(argc, argv);
        if (!parse_res || parser_ptr->Exist("help"))
            return false;
        return true;
    }

    bool CheckCMDParam()
    {
        if (const auto& rom_file_path = parser_ptr->Get("rom_file");!std::filesystem::exists(rom_file_path))
        {
            std::cout << "Do not exist rom file" << rom_file_path << ", please check." << std::endl;
            return false;
        }

        const auto& config_file_path = parser_ptr->Get("config_file");
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

        config.RomPath = parser_ptr->Get("rom_file");

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
            SetValue(config.ShortcutKeys.Screenshot, section, "screenshot");
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
        if (parser_ptr->Exist("help"))
        {
            constexpr int help_space = 16;
            constexpr std::string_view COMPILE_YEAR = GetCompileYear();
            std::string head = std::string{PROGRAM_NAME} + "\nMIT License\nCopyright (c) 2023 - " + COMPILE_YEAR.data() + " FerociousMagikarp\n\n";
            std::string msg = "usage: NesEmulator <[--rom_file] rom_file> [options] ...\n\noptions:\n";
            for (const auto& [name, arg] : parser_ptr->m_args)
            {
                msg += "  ";
                std::string next = "    --";
                if (arg.short_name != '\0')
                {
                    next[0] = '-';
                    next[1] = arg.short_name;
                    next[2] = ',';
                }
                msg += next;
                msg += name;
                msg.append(static_cast<std::size_t>(std::max(1, help_space - static_cast<int>(name.size()))), ' ');
                msg += arg.message;
                msg += "\n";
            }
            return head + msg;
        }
        else
        {
            return parser_ptr->m_error;
        }
    }

    void CMDClear() noexcept
    {
        parser_ptr = nullptr;
        ini_parser_ptr = nullptr;
    }
}
