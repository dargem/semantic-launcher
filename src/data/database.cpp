#include <rapidfuzz/rapidfuzz_all.hpp>

#include "rapidfuzz/fuzz.hpp"
#include "src/configs.hpp"
#include "src/data/database.hpp"

using unum::usearch::index_dense_t;
using unum::usearch::metric_kind_t;
using unum::usearch::metric_punned_t;
using unum::usearch::scalar_kind_t;

Database::Database(Embedder embedder)
    : m_embedder(embedder),
      m_vector_db(index_dense_t::make(
          metric_punned_t(embedder.get_dimensionality(), metric_kind_t::l2sq_k, scalar_kind_t::f32_k)))
{
    for (const auto& aggregator : configs::AGGREGATORS)
    {
        if (!aggregator->check_applicable())
        {
            continue;
        }
        aggregator->aggregate(m_files, m_file_membership);
    }

    m_vector_db.reserve(m_files.size());
    for (size_t i{}; i < m_files.size(); ++i)
    {
        // We want to use our stable indexes as keys for our embedding db
        siv::ID key = m_files.createHandleFromData(i).getID();
        File& file = m_files[key];

        // Consider also embedding name in future
        if (file.m_description.empty())
        {
            continue; // Skip if this file has no description
        }
        std::vector<float> embeddings = embedder.embed(file.m_description);
        m_vector_db.add(key, embeddings.data());
    }
};

std::vector<Result> Database::get_semantic_best(std::string_view query, size_t num, double cut_off) const
{
    auto embedding = m_embedder.embed(query);
    auto results = m_vector_db.search(embedding.data(), num);

    std::vector<Result> out;
    out.reserve(results.size());

    for (size_t i{}; i < results.size(); ++i)
    {
        const int key = results[i].member.key;
        const float score = 1.0f / (1.0f + results[i].distance);
        if (score >= cut_off)
        {
            out.push_back(Result{m_files[key], score});
        }
    }

    std::sort(out.begin(), out.end(), [](const Result& a, const Result& b) { return a.m_score > b.m_score; });

    return out;
}

// Use a fuzzy string match
std::vector<Result> Database::get_match_best(std::string_view query, size_t n, double cut_off) const
{
    // The partial ratio scorer does not care about lengths which can be good
    // e.g. search for first word of a 5 word app it size dif should not disregard it
    rapidfuzz::fuzz::CachedPartialRatio<char> scorer(query);

    // But there is issue if the query is very short its easily 100%
    rapidfuzz::fuzz::CachedRatio<char> exact_scorer(query);

    std::vector<Result> best_n;
    best_n.reserve(n); // May not use n in case < n results to match

    auto eval = [&](std::string_view opt) -> double
    {
        if (opt.size() <= configs::EXACT_MATCH_SIZE_CUTOFF || query.size() <= configs::EXACT_MATCH_SIZE_CUTOFF)
        {
            // we use an exact match
            if (opt == query)
                return 1.0;
            if (opt.starts_with(query))
                return 0.9;
            // Can do a contains if wanted as well
            return 0.0;
        }

        return scorer.similarity(opt) / 100.0; // Normalize
    };

    for (auto option : m_files)
    {
        std::string_view opt = option.m_name;

        if (opt.size() + 1 < query.size())
        {
            // If the query is a lot larger than option size probably not correct
            continue;
        }

        double score = eval(opt);

        if (score < cut_off)
            continue;

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

    std::sort(best_n.begin(), best_n.end(), [](const Result& a, const Result& b) { return a.m_score > b.m_score; });
    return best_n;
}