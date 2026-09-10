#pragma once
#include <filesystem>
#include <optional>
#include <string>

struct File
{
    std::string m_name;
    std::filesystem::path m_executable;
    std::string m_description; // Short description of the file
    std::optional<std::filesystem::path> m_icon{};
};

/* A result of the search */
struct Result
{
    File m_file;
    float m_score; // Score of a result: [0, 1] 1 is ideal
};