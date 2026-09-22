#pragma once

#include "src/data/sources/i_aggregate.hpp"
#include <QFileInfo>
#include <filesystem>

// Aggregates from .desktop files
class Desktop : public IAggregate
{
public:
    Desktop();
    bool check_applicable() const override;
    void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const override;
    ~Desktop() override = default;

    // Takes a .desktop file entry and serializes it into a File
    static File load_entry(const QFileInfo&);

private:
    std::optional<std::vector<std::filesystem::path>> m_desktop_folders; // Path to .desktop folders
};