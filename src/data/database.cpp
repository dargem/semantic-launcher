#include "src/data/database.hpp"

#include "src/configs.hpp"
#include <algorithm>
#include <unordered_map>

// No rankers should be null
Database::Database(std::vector<WeightedRanker> rankers) : m_rankers(std::move(rankers))
{
    for (const auto& aggregator : configs::AGGREGATORS)
    {
        if (!aggregator->check_applicable()) { continue; }
        aggregator->aggregate(m_files, m_file_membership);
    }

    for (auto& wr : m_rankers) { wr.m_ranker->build_index(m_files); }
}

Database::Database(std::vector<std::unique_ptr<IRanker>> rankers)
    : Database(
          [](std::vector<std::unique_ptr<IRanker>> r)
          {
              std::vector<WeightedRanker> wrs;
              wrs.reserve(r.size());
              for (auto& ranker : r) { wrs.push_back(WeightedRanker{.m_ranker = std::move(ranker), .m_weight = 1.0}); }
              return wrs;
          }(std::move(rankers)))
{
}

std::vector<Result> Database::get_best(std::string_view query, size_t n) const
{
    if (query.empty() || n == 0 || m_rankers.empty()) { return {}; }

    struct Candidate
    {
        File m_file;
        double m_weighted_score{0.0};
    };

    std::unordered_map<std::string, Candidate> candidates;
    const size_t candidate_pool_size = std::max(n, size_t{50});
    double total_weight = 0.0;

    for (const auto& weighted_ranker : m_rankers)
    {
        if (weighted_ranker.m_weight <= 0.0) { continue; }

        total_weight += weighted_ranker.m_weight;
        auto ranker_results = weighted_ranker.m_ranker->get_best(query, candidate_pool_size);

        for (const auto& result : ranker_results)
        {
            const auto& key = result.m_file.m_name;
            const double weighted_score = weighted_ranker.m_weight * static_cast<double>(result.m_score);

            auto [it, inserted] = candidates.try_emplace(key, Candidate{result.m_file, 0.0});
            it->second.m_weighted_score += weighted_score;
        }
    }

    if (candidates.empty() || total_weight <= 0.0) { return {}; }

    std::vector<Result> ranked_results;
    ranked_results.reserve(candidates.size());

    for (auto& [_, candidate] : candidates)
    {
        const double normalized_score = candidate.m_weighted_score / total_weight;
        if (normalized_score <= 0.0) { continue; }

        ranked_results.emplace_back(std::move(candidate.m_file), normalized_score);
    }

    std::sort(ranked_results.begin(),
              ranked_results.end(),
              [](const Result& a, const Result& b) { return a.m_score > b.m_score; });

    if (ranked_results.size() > n) { ranked_results.resize(n); }

    return ranked_results;
}