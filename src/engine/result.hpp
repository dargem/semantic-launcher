#pragma once
#include <string>
#include <filesystem>

/* A result of the search */
struct Result {
    std::string name;
    std::filesystem::path icon;
    std::filesystem::path executable;
};