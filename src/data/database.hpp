#pragma once

#include "src/engine/result.hpp"
#include <cstddef>
#include <string_view>
#include <usearch/index.hpp>
#include <usearch/index_dense.hpp>
#include <vector>

// Manages searchable files and provides
class Database
{
public:
    Database();

    // Uses exact / fuzzy match to get best n results, result size is <= n, sorted by strength
    std::vector<Result> get_match_best(std::string_view query, size_t n) const;

    // Uses semantic search to get best n results, result size is <= n, sorted by strength
    std::vector<Result> get_semantic_best(std::string_view query, size_t num) const;

private:
    constexpr static size_t EMBEDDING_DIMENSIONS = 364; // Holder for now
    // Possibly replace this with my own solution later?
    unum::usearch::index_dense_t m_vector_db;

    // Could consider not double
    std::vector<std::array<double, EMBEDDING_DIMENSIONS>> m_embeddings;
};