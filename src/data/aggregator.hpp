#pragma once

#include "src/utils/index_vector.hpp"
#include <src/data/result.hpp>
#include <unordered_set>

// A config file selecting what the aggregator should aggregate from
struct AggregatorConfig
{
    bool m_pacman{};
    bool m_yay{};
    bool m_desktop{}; // .desktop
    bool m_flatpaks{};
    bool m_appimages{};
};

void aggregate_pacman(siv::Vector<Result>& results, std::unordered_set<std::string> membership)
{
    // pacman -Qqe get explicitly installed packages

    // pacman -Ql gets all files owned by installed package, find if any executable

    // pacman -Qi <name> to get descriptions for survivors

    // Do a dedup add into results
}

siv::Vector<Result>&& aggregate_results(AggregatorConfig config)
{
    siv::Vector<Result> results;

    // Want to dedup entries which we do through checking membership in the set
    std::unordered_set<std::string> membership;

    if (config.m_pacman)
    {
        aggregate_pacman(results, membership);
    }
}
