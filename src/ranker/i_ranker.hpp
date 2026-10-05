#pragma once

#include "src/data/result.hpp"
#include "src/utils/index_vector.hpp"
#include <cstddef>
#include <string_view>
#include <vector>

class IRanker
{
public:
    virtual ~IRanker() = default;

    // Called once the database has aggregated all files
    virtual void build_index(siv::Vector<File>& files) = 0;

    // Returns the best n results matching the query, sorted descending by score
    virtual std::vector<Result> get_best(std::string_view query, size_t n) const = 0;
};
