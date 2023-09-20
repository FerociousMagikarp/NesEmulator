#include "command_line.h"
#include "emulator.h"
#include "virtual_device.h"
#include <algorithm>
#include <sstream>

constexpr const char* HELP_MESSAGE = 
"usage : NesEmulator [rom_file] [options]...\n\n"
"options:\n"
"-scale scale                    scale of the window, the param must be an integer in [1, 10]\n"
"\n"
"other commands to be continue\n";


bool CommandLine::Init(int argc, char **argv)
{
    if (argc <= 1)
    {
        m_error = "Please type the nes file path, use '-h' or '--help' for help.";
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            // 这里这么写效率会低，先这么搞
            std::string tmp = argv[i];
            if (tmp == "-h" || tmp == "--help")
            {
                m_error = HELP_MESSAGE;
                return false;
            }
            else if (tmp == "-scale")
            {
                if (i + 1 >= argc)
                {
                    m_error = "Command " + tmp + " must follow a parameter.";
                    return false;
                }
                std::stringstream ss;
                ss << argv[++i];
                int scale;
                ss >> scale;
                scale = std::clamp(scale, 1, 10);
                m_commands.insert(std::pair(CommandOption::Scale, scale));
            }
            else
            {
                m_error = "Unrecognise command : " + tmp;
                return false;
            }
        }
        else
        {
            m_nes_path = argv[i];
        }
    }

    if (m_nes_path.empty())
    {
        m_error = "Please type the nes file path, use '-h' or '--help' for help.";
        return false;
    }

    return true;
}

void CommandLine::Execute(const std::unique_ptr<nes::NesEmulator>& emulator, const std::shared_ptr<nes::VirtualDevice>& device)
{
    // 先暂时这么写，之后如果命令多了就重构
    for (auto iter = m_commands.cbegin(); iter != m_commands.cend(); ++iter)
    {
        switch (iter->first)
        {
            case CommandOption::Scale:
                device->SetScale(std::get<int>(iter->second));
                break;
        };
    }
}
