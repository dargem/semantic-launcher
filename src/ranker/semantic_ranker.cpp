#include "src/ranker/semantic_ranker.hpp"

#include <algorithm>

using unum::usearch::index_dense_t;
using unum::usearch::metric_kind_t;
using unum::usearch::metric_punned_t;
using unum::usearch::scalar_kind_t;

SemanticRanker::SemanticRanker(Embedder embedder)
    : m_embedder(std::move(embedder)),
      m_vector_db(index_dense_t::make(
          metric_punned_t(m_embedder.get_dimensionality(), metric_kind_t::l2sq_k, scalar_kind_t::f32_k)))
{
}

void SemanticRanker::build_index(siv::Vector<File>& files)
{
    m_files = &files;
    m_vector_db.reserve(files.size());

    for (size_t i{}; i < files.size(); ++i)
    {
        siv::ID key = files.createHandleFromData(i).getID();
        const File& file = files[key];

        if (file.m_description.empty()) { continue; }

        std::vector<float> embeddings = m_embedder.embed(file.m_description);
        m_vector_db.add(key, embeddings.data());
    }
}

std::vector<Result> SemanticRanker::get_best(std::string_view query, size_t n, double cut_off) const
{
    if (!m_files || query.empty() || n == 0 || m_vector_db.size() == 0) { return {}; }

    auto embedding = m_embedder.embed(query);
    auto results = m_vector_db.search(embedding.data(), n);

    std::vector<Result> out;
    out.reserve(results.size());

    for (size_t i{}; i < results.size(); ++i)
    {
        const auto key = static_cast<siv::ID>(results[i].member.key);
        const float score = 1.0f / (1.0f + results[i].distance);
        if (score >= cut_off) { out.push_back(Result{(*m_files)[key], score}); }
    }

    std::sort(out.begin(), out.end(), [](const Result& a, const Result& b) { return a.m_score > b.m_score; });

    return out;
}
