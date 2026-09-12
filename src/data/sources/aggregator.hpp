#pragma once

#include "src/data/result.hpp"
#include "src/utils/index_vector.hpp"
#include <unordered_set>

// Abstract aggregator
class IAggregator
{
public:
    // Validity check on whether this aggregation check should be used, i.e. pacman aggregator should check this system
    // is actually using pacman as its package manager
    virtual bool check_applicable() const;

    // Take files and and a membership set, adds stuff it aggregates to files (checking for duplicates with membership)
    virtual void aggregate(siv::Vector<File>& files, std::unordered_set<std::string> membership) const;
};