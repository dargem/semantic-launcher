#include <rapidfuzz/rapidfuzz_all.hpp>

#include "src/data/aggregator.hpp"
#include "src/data/database.hpp"
#include <limits>

using unum::usearch::index_dense_t;
using unum::usearch::metric_kind_t;
using unum::usearch::metric_punned_t;
using unum::usearch::scalar_kind_t;

Database::Database(Embedder embedder)
    : m_embedder(embedder),
      m_vector_db(index_dense_t::make(
          metric_punned_t(embedder.get_dimensionality(), metric_kind_t::l2sq_k, scalar_kind_t::f32_k)))
{
    m_files = aggregate_results(AggregatorConfig{true, false, false, false, false});
    m_vector_db.reserve(m_files.size());
    for (size_t i{}; i < m_files.size(); ++i)
    {
        // We want to use our stable indexes as keys for our embedding db
        siv::ID key = m_files.createHandleFromData(i).getID();
        File& file = m_files[key];

        // Consider also embedding name in future
        std::vector<float> embeddings = embedder.embed(file.m_description);
        m_vector_db.add(key, embeddings.data());
    }
};

std::vector<Result> Database::get_semantic_best(std::string_view query, size_t num) const
{
    auto embedding = m_embedder.embed(query);
    auto results = m_vector_db.search(embedding.data(), num);

    std::vector<Result> out;
    out.reserve(results.size());

    for (size_t i{}; i < results.size(); ++i)
    {
        const int key = results[i].member.key;
        const float score = results[i].distance;
        out.push_back(Result{m_files[key], score});
    }

    std::sort(out.begin(), out.end(), [](Result& a, Result& b) { return a.m_score < b.m_score; });
    std::reverse(out.begin(), out.end()); // Descending order

    return out;
}

// Use a fuzzy string match
std::vector<Result> Database::get_match_best(std::string_view query, size_t n) const
{
    rapidfuzz::fuzz::CachedRatio<char> scorer(query);

    std::vector<Result> best_n;
    best_n.reserve(n); // May not use n in case < n results to match

    for (auto option : m_files)
    {
        double score = scorer.similarity(option.m_name); // Consider using description later

        if (best_n.size() < n)
        {
            best_n.push_back(Result(option, score));
            continue;
        }

        auto worst_it =
            std::min_element(best_n.begin(), best_n.end(), [](Result& a, Result& b) { return a.m_score < b.m_score; });

        if (worst_it->m_score < score)
        {
            // Do a replacement
            worst_it->m_score = score;
            worst_it->m_file = option;
        }
    }

    std::sort(best_n.begin(), best_n.end(), [](Result& a, Result& b) { return a.m_score < b.m_score; });
    std::reverse(best_n.begin(), best_n.end()); // Descending order
    return best_n;
}