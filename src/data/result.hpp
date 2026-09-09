#pragma once
#include <filesystem>
#include <string>

/* A result of the search */
struct Result
{
    std::string name;
    std::filesystem::path icon;
    std::filesystem::path executable;
    float score; // Score of a result: [0, 1] 1 is ideal
};