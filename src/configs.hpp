#pragma once

#include "src/data/sources/appimage.hpp"
#include "src/data/sources/desktop.hpp"
#include "src/data/sources/i_aggregate.hpp"
#include "src/data/sources/pacman.hpp"

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

inline auto const AGGREGATORS = []
{
    std::vector<std::unique_ptr<IAggregate>> agg;
    // agg.push_back(std::make_unique<Pacman>());
    agg.push_back(std::make_unique<Desktop>());
    agg.push_back(std::make_unique<AppImage>());
    return agg;
}();

// Search uses the top K of a search type given they are above the score
inline constexpr size_t SEMANTIC_ACCEPTED_K = 3;
inline constexpr double SEMANTIC_ACCEPTED_SCORE = 0.5;

inline constexpr size_t FUZZY_ACCEPTED_K = 3;
inline constexpr double FUZZY_ACCEPTED_SCORE = 0.6;

// The terminals to check for on startup, if your terminal is not here it will not "see" it
// Earlier on the list == higher preference, if an undesired terminal is being used reorder it,
// or just delete the one you don't want
inline static const std::vector<std::string> TERMINALS = {
    "kitty", "alacritty", "wezterm", "konsole", "gnome-terminal", "xfce4-terminal", "foot", "xterm"};

// The fuzzy match will use an exact match small strings (smaller than cutoff)
inline constexpr size_t EXACT_MATCH_SIZE_CUTOFF = 2;

} // namespace configs