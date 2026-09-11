#pragma once

#include "src/data/result.hpp"
#include "src/embedder/embedder.hpp"
#include "src/utils/index_vector.hpp"
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
    // Strength is [0, 1] and no result will be < than cutoff
    std::vector<Result> get_match_best(std::string_view query, size_t n, double cut_off = 0.0) const;

    // Uses semantic search to get best n results, result size is <= n, sorted by strength
    // Strength is [0, 1] and no results will be < than cutoff
    std::vector<Result> get_semantic_best(std::string_view query, size_t n, double cut_off = 0.0) const;

private:
    Embedder m_embedder;

    // Possibly replace this with my own solution later?
    unum::usearch::index_dense_t m_vector_db;

    // Sparse set of files
    siv::Vector<File> m_files;
};