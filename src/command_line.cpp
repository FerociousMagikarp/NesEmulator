#include "command_line.h"

constexpr const char* HELP_MESSAGE = 
"adgdgdffdgdf";


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
            else if (tmp == "-s" || tmp == "--scale")
            {
                if (i + 1 >= argc)
                {
                    m_error = "Command " + tmp + " must have a parameter.";
                    return false;
                }
                m_commands.insert(std::pair(std::move(tmp), argv[++i]));
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
