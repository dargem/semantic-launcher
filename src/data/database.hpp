#pragma once

#include "src/embedder/embedder.hpp"
#include "src/engine/result.hpp"
#include <cassert>
#include <cstddef>
#include <string_view>
#include <usearch/index.hpp>
#include <usearch/index_dense.hpp>
#include <vector>

// Manages searchable files and provides
// Should be templated with the number of dimensions the embedding model has
class Database
{
public:
    Database(Embedder embedder);

    // Uses exact / fuzzy match to get best n results, result size is <= n, sorted by strength
    std::vector<Result> get_match_best(std::string_view query, size_t n) const;

    // Uses semantic search to get best n results, result size is <= n, sorted by strength
    std::vector<Result> get_semantic_best(std::string_view query, size_t num) const;

private:
    Embedder m_embedder;
    // Possibly replace this with my own solution later?
    unum::usearch::index_dense_t m_vector_db;
};