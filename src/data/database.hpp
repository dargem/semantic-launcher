#pragma once

#include <vector>
#include <cstddef>
#include <usearch/index.hpp>
#include "src/engine/result.hpp"

// Manages searchable files and provides 
class Database {
public:
    Database();

    // Uses exact / fuzzy match to get best n results, result size is <= n, sorted by strength
    std::vector<Result> get_match_best(std::string_view str, size_t n) const; 

    // Uses semantic search to get best n results, result size is <= n, sorted by strength
    std::vector<Result> get_semantic_best(std::string_view str, size_t num) const;
private:
      
};