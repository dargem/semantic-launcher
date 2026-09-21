#pragma once

#include "src/data/sources/i_aggregate.hpp"
#include <QFileInfo>
#include <filesystem>

// A .desktop file can be loaded into a desktop entry
// Drops some irrelevant data
struct DesktopEntry
{
    std::string m_name;
    std::string m_comment;
    std::string m_exec;
    std::string m_icon;
    LaunchType m_launch_type;
};

// Aggregates from .desktop files
class Desktop : public IAggregate
{
public:
    Desktop();
    bool check_applicable() const override;
    void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const override;
    ~Desktop() override = default;

    // Takes a .desktop file entry and serializes it into a DesktopEntry
    static DesktopEntry load_entry(const QFileInfo&);

private:
    std::optional<std::filesystem::path> m_desktop_folder; // Path to .desktop folder
};