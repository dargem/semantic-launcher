#pragma once

#include "src/data/sources/appimage.hpp"
#include "src/data/sources/application.hpp"
#include "src/data/sources/i_aggregate.hpp"
#include <QCoreApplication>
#include <QStandardPaths>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// Config file, can move to json later if necessary

// What aggregators the program will try to aggregate files from
// There is a detection system inplace additionally also
// e.g. if pacman cannot be found for example it will be skipped

namespace configs
{

// The embedding model to use, located in the models/ directory
inline static const std::string MODEL_NAME = "model.gguf";

inline std::string resolve_model_path(const std::string& name = MODEL_NAME)
{
    if (const char* env = std::getenv("SEMANTIC_LAUNCHER_MODEL_PATH"))
    {
        if (std::filesystem::exists(env)) return env;
    }

    std::vector<std::filesystem::path> search_dirs = {
        std::filesystem::current_path() / "models",
        std::filesystem::current_path(),
    };

    // Check directory relative to the running binary (e.g. build dir or install prefix)
    QString app_dir = QCoreApplication::applicationDirPath();
    if (!app_dir.isEmpty())
    {
        std::filesystem::path bin_dir = app_dir.toStdString();
        search_dirs.push_back(bin_dir / "models");
        search_dirs.push_back(bin_dir / ".." / "models");
        search_dirs.push_back(bin_dir / ".." / "share" / "semantic-launcher" / "models");
    }

    // Standard system & user data locations (e.g. ~/.local/share/semantic-launcher/models,
    // /usr/share/semantic-launcher/models)
    for (const auto& loc : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation))
    {
        search_dirs.push_back(std::filesystem::path(loc.toStdString()) / "models");
        search_dirs.push_back(std::filesystem::path(loc.toStdString()));
    }

    search_dirs.push_back(std::filesystem::path("/usr/local/share/semantic-launcher/models"));
    search_dirs.push_back(std::filesystem::path("/usr/share/semantic-launcher/models"));

    for (const auto& dir : search_dirs)
    {
        std::error_code ec;
        auto path = dir / name;
        if (std::filesystem::exists(path, ec)) return path.lexically_normal().string();
    }
    return "models/" + name;
}

inline auto const AGGREGATORS = []
{
    std::vector<std::unique_ptr<IAggregate>> agg;
    // agg.push_back(std::make_unique<Pacman>());
    agg.push_back(std::make_unique<Application>());
    agg.push_back(std::make_unique<AppImage>());
    return agg;
}();

// CombSUM weights for rankers
inline constexpr double FUZZY_WEIGHT = 0.4;
inline constexpr double SEMANTIC_WEIGHT = 0.8;

// The terminals to check for on startup, if your terminal is not here it will not "see" it
// Earlier on the list == higher preference, if an undesired terminal is being used reorder it,
// or just delete the one you don't want
inline static const std::vector<std::string> TERMINALS = {
    "kitty", "alacritty", "wezterm", "konsole", "gnome-terminal", "xfce4-terminal", "foot", "xterm"};

// The fuzzy match will use an exact match small strings (smaller than cutoff)
inline constexpr size_t EXACT_MATCH_SIZE_CUTOFF = 2;

} // namespace configs