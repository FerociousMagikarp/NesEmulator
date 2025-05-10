#pragma once

#include <unordered_map>
#include <string_view>
#include <string>
#include <vector>

namespace nes_support
{
    struct IniSection
    {
        public:
            bool ExistValue(std::string_view value) const noexcept;
            const std::string_view GetValue(std::string_view value) const noexcept;

        private:
            std::unordered_map<std::string_view, std::string_view> m_value;

            friend class IniParser;
    };

    class IniParser
    {
        public:
            bool LoadFromFile(std::string_view filename);
            bool ExistSection(std::string_view section) const noexcept;
            const IniSection& GetSection(std::string_view section) const noexcept;
        
        private:
            std::unordered_map<std::string_view, IniSection> m_sections;
            std::vector<std::pair<std::size_t, std::string>> m_ini_content; // 这里存储一下全部配置，反正也没多大，里面存的内容是(行号，文本)
    };

} // nes_support
