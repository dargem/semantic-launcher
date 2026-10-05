#pragma once

#include "src/data/result.hpp"
#include "src/ranker/i_ranker.hpp"
#include "src/utils/index_vector.hpp"
#include <cassert>
#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

// Manages searchable files and rankers
class Database
{
public:
    struct WeightedRanker
    {
        std::unique_ptr<IRanker> m_ranker;
        double m_weight{1.0};
    };

    explicit Database(std::vector<WeightedRanker> rankers);
    explicit Database(std::vector<std::unique_ptr<IRanker>> rankers);

    // Queries all managed rankers and fuses rankings using Weighted CombSUM
    std::vector<Result> get_best(std::string_view query, size_t n) const;

private:
    // Managed rankers with weights
    std::vector<WeightedRanker> m_rankers;

    // Sparse set of files
    siv::Vector<File> m_files;

    // Reverse mapping name -> siv::ID
    std::unordered_map<std::string, siv::ID> m_file_membership;
};
