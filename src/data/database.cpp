#include "src/data/database.hpp"

#include "src/configs.hpp"
#include <algorithm>
#include <unordered_map>

// No rankers should be null
Database::Database(std::vector<std::unique_ptr<IRanker>> rankers) : m_rankers(std::move(rankers))
{
    for (const auto& aggregator : configs::AGGREGATORS)
    {
        if (!aggregator->check_applicable()) { continue; }
        aggregator->aggregate(m_files, m_file_membership);
    }

    for (auto& ranker : m_rankers) { ranker->build_index(m_files); }
}

std::vector<Result> Database::get_best(std::string_view query, size_t n, double cut_off) const
{
    if (query.empty() || n == 0) { return {}; }

    std::vector<Result> all_results;
    for (const auto& ranker : m_rankers)
    {
        auto ranker_results = ranker->get_best(query, n, cut_off);
        all_results.append_range(std::move(ranker_results));
    }

    // Deduplicate results by file name, keeping the highest score
    std::unordered_map<std::string, Result> merged_results;
    merged_results.reserve(all_results.size());

    for (const auto& result : all_results)
    {
        const auto& key = result.m_file.m_name;
        auto [it, inserted] = merged_results.try_emplace(key, result);
        if (!inserted && it->second.m_score < result.m_score) { it->second = result; }
    }

    std::vector<Result> ranked_results;
    ranked_results.reserve(merged_results.size());
    for (auto& [_, result] : merged_results) { ranked_results.push_back(std::move(result)); }

    std::sort(ranked_results.begin(),
              ranked_results.end(),
              [](const Result& a, const Result& b) { return a.m_score > b.m_score; });

    if (cut_off > 0.0)
    {
        ranked_results.erase(std::remove_if(ranked_results.begin(),
                                            ranked_results.end(),
                                            [cut_off](const Result& result) { return result.m_score < cut_off; }),
                             ranked_results.end());
    }

    if (ranked_results.size() > n) { ranked_results.resize(n); }

    return ranked_results;
}