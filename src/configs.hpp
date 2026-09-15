#pragma once

#include "src/data/sources/desktop.hpp"
#include "src/data/sources/i_aggregate.hpp"
#include "src/data/sources/pacman.hpp"

#include <memory>

// Config file, can move to json later if necessary

// What aggregators the program will try to aggregate files from
// There is a detection system inplace additionally also
// e.g. if pacman cannot be found for example it will be skipped

namespace configs
{

inline auto const AGGREGATORS = []
{
    std::vector<std::unique_ptr<IAggregate>> agg;
    agg.push_back(std::make_unique<Pacman>());
    agg.push_back(std::make_unique<Desktop>());
    return agg;
}();

// Search uses the top K of a search type given they are above the score
inline constexpr size_t SEMANTIC_ACCEPTED_K = 3;
inline constexpr double SEMANTIC_ACCEPTED_SCORE = 0.5;

inline constexpr size_t FUZZY_ACCEPTED_K = 3;
inline constexpr double FUZZY_ACCEPTED_SCORE = 0.6;

} // namespace configs