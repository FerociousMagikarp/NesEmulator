#include "ini_parser.h"
#include <assert.h>
#include <fstream>
#include <iostream>

namespace nes_support
{

    bool IniSection::ExistValue(std::string_view value) const noexcept
    {
        return m_value.contains(value);
    }

    const std::string_view IniSection::GetValue(std::string_view value) const noexcept
    {
        assert(ExistValue(value));
        return m_value.find(value)->second;
    }

    bool IniParser::ExistSection(std::string_view section) const noexcept
    {
        return m_sections.contains(section);
    }

    const IniSection& IniParser::GetSection(std::string_view section) const noexcept
    {
        assert(ExistSection(section));
        return m_sections.find(section)->second;
    }

    // 去除首尾空格
    void TrimString(std::string& str)
    {
        if (auto pos = str.find_first_not_of(" \t\n\r"); pos != std::string::npos)
        {
            str.erase(0, pos);
        }
        else
        {
            str = "";
            return;
        }

        if (auto pos = str.find_last_not_of(" \t\n\r"); pos != std::string::npos)
        {
            str.erase(pos + 1);
        }
    }

    bool IniParser::LoadFromFile(std::string_view filename)
    {
        std::ifstream ifs(filename.data());

        if (!ifs.is_open())
        {
            std::cout << "Config file " << filename << " open failed!\n";
            return false;
        }

        std::string line = "";
        std::size_t line_num = 0;

        while (std::getline(ifs, line))
        {
            line_num++;
            TrimString(line);

            if (auto pos = line.find('#'); pos != std::string::npos)
            {
                // 有注释
                line.erase(pos);
                if (auto last_pos = line.find_last_not_of(" \t"); last_pos != std::string::npos)
                    line.erase(last_pos + 1);
            }
            if (line.empty())
                continue;

            // 先把所有字符串都塞进去，防止后续添加的时候 string_view 失效
            m_ini_content.push_back(std::make_pair(line_num, std::move(line)));
        }

        IniSection* section = nullptr;

        // 这个时候就可以遍历 m_ini_content 了，注意不要再动这个 vector 了
        for (const auto& [num, str] : m_ini_content)
        {
            if (str.front() == '[' && str.back() == ']') // section
            {
                std::string_view s = str;
                s.remove_prefix(1);
                s.remove_suffix(1);

                auto iter = m_sections.find(s);
                if (iter == m_sections.end())
                {
                    auto [it, success] = m_sections.insert(std::make_pair(s, IniSection{}));
                    if (!success)
                    {
                        std::cout << "Section insert failed, at line " << num << " : " << str << std::endl;
                        return false;
                    }
                    iter = it;
                }
                section = &iter->second;
            }
            else if (auto equal_pos = str.find('='); equal_pos != std::string::npos)
            {
                if (section == nullptr)
                {
                    std::cout << "Please put key-value into a section, at line " << num << " : " << str << std::endl;
                    return false;
                }
                std::string_view key {str.c_str(), equal_pos};
                std::string_view value { str.c_str() + equal_pos + 1 };
                if (auto pos = key.find_last_not_of(" \t"); pos != std::string::npos)
                {
                    key.remove_suffix(key.size() - pos - 1);
                }
                if (auto pos = value.find_first_not_of(" \t"); pos != std::string::npos)
                {
                    value.remove_prefix(pos);
                }
                if (key.empty() || value.empty())
                {
                    std::cout << "Unknown expression, at line " << num << " : " << str << std::endl;
                    return false;
                }
                section->m_value[key] = value;
            }
            else
            {
                std::cout << "Ini parse failed, at line " << num << " : " << str << std::endl;
                return false;
            }
        }

        return true;
    }

}
