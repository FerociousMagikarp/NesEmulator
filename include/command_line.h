#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace nes
{
    class NesEmulator;
    class VirtualDevice;
}

enum class CommandOption
{
    Scale,
};

class CommandLine
{
public:
    static CommandLine& GetInstance()
    {
        static CommandLine instance;
        return instance;
    }
    ~CommandLine() = default;

    bool Init(int argc, char** argv);
    void Execute(const std::unique_ptr<nes::NesEmulator>& emulator, const std::shared_ptr<nes::VirtualDevice>& device);
    inline const std::string& GetError() const { return m_error; }
    inline const std::string& GetNesPath() const { return m_nes_path; }

private:
    CommandLine() = default;

    std::string m_error = "";
    std::string m_nes_path = "";

    using CommandParamType = std::variant<int, std::string>;
    std::unordered_map<CommandOption, CommandParamType> m_commands;
};
