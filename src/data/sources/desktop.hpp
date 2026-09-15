#pragma once

#include "src/data/sources/i_aggregate.hpp"
#include <filesystem>

// Aggregates from .desktop files
class Desktop : public IAggregate
{
public:
    Desktop();
    bool check_applicable() const override;
    void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const override;

private:
    std::optional<std::filesystem::path> m_desktop_folder; // Path to .desktop folder
};