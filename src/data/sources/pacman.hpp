#pragma once

#include "src/data/sources/aggregator.hpp"

// Aggregates from pacman
class Pacman : public IAggregator
{
    bool check_applicable() const override;

    void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const override;
};