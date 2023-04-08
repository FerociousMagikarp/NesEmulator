#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class NesEmulator;

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
    void Execute(const std::unique_ptr<NesEmulator>& emulator);
    inline const std::string& GetError() const { return m_error; }
    inline const std::string& GetNesPath() const { return m_nes_path; }

private:
    CommandLine() = default;

    std::string m_error = "";
    std::string m_nes_path = "";
    std::unordered_map<std::string, std::string> m_commands;
};
