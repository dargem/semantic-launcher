#pragma once
#include <filesystem>
#include <string>

struct File
{
    std::string name;
    std::filesystem::path executable;
    std::filesystem::path icon;
};

/* A result of the search */
struct Result
{
    File file;
    float score; // Score of a result: [0, 1] 1 is ideal
};