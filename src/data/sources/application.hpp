#pragma once

#include "src/data/sources/i_aggregate.hpp"
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

// Aggregates from .desktop files
class Application : public IAggregate
{
public:
    Application();
    bool check_applicable() const override;
    void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const override;
    ~Application() override = default;

private:
    std::optional<std::vector<std::filesystem::path>> m_desktop_folders; // Path to .desktop folders
};
