#pragma once

#include "src/data/result.hpp"
#include <QFileInfo>
#include <optional>

class DesktopUtils
{
public:
    // Takes a .desktop file entry and serializes it into a File
    static std::optional<File> load_entry(const QFileInfo& desktop_file);
};
