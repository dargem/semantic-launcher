#pragma once

#include "src/embedder/embedder.hpp"
#include "src/ranker/i_ranker.hpp"
#include <usearch/index.hpp>
#include <usearch/index_dense.hpp>

class SemanticRanker : public IRanker
{
public:
    explicit SemanticRanker(Embedder embedder);
    ~SemanticRanker() override = default;

    void build_index(siv::Vector<File>& files) override;

    std::vector<Result> get_best(std::string_view query, size_t n) const override;

private:
    Embedder m_embedder;
    unum::usearch::index_dense_t m_vector_db;
    const siv::Vector<File>* m_files{nullptr};
};
