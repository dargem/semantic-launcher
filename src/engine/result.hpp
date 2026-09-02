#pragma once
#include <string>
#include <filesystem>

/* A result of the search */
struct Result {
    std::string name;
    std::filesystem::path icon;
    std::filesystem::path executable;
    double score; // Score of a result: [0, 1] 1 is ideal
};