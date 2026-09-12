#pragma once
#include <filesystem>
#include <optional>
#include <string>

// Whether its terminal or direct launched
enum class LaunchType
{
    TERMINAL = 0, // Will terminal default
    DIRECT = 1,   // Might want a 3rd member for if its iffy...
};

struct File
{
    std::string m_name;
    std::filesystem::path m_executable;
    std::string m_description; // Short description of the file
    LaunchType m_launch_type;
    std::optional<std::filesystem::path> m_icon{};
};

/* A result of the search */
struct Result
{
    File m_file;
    float m_score; // Score of a result: [0, 1] 1 is ideal
};