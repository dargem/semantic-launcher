#pragma once

#include "src/data/sources/i_aggregate.hpp"

// Aggregates from pacman
class Pacman : public IAggregate
{
    bool check_applicable() const override;

    void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const override;
};