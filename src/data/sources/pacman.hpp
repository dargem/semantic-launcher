#pragma once

#include "src/data/sources/aggregator.hpp"

class Pacman : public IAggregator
{
    bool check_applicable() const;

    void aggregate(siv::Vector<File>& files, std::unordered_set<std::string> membership) const;
};