#include "src/ranker/fuzzy_desc_ranker.hpp"
#include <rapidfuzz/fuzz.hpp>
#include <rapidfuzz/rapidfuzz_all.hpp>
#include <src/configs.hpp>

// Should change this to be a keyword match with more thought...
void FuzzyDescRanker::build_index(siv::Vector<File>& files) { m_files = &files; }

std::vector<Result> FuzzyDescRanker::get_best(std::string_view query, size_t n) const
{
    if (query.empty() || n == 0) { return {}; }

    rapidfuzz::fuzz::CachedPartialRatio<char> scorer(query);
    rapidfuzz::fuzz::CachedRatio<char> exact_scorer(query);

    std::vector<Result> best_n;
    best_n.reserve(n);

    auto eval = [&](std::string_view opt) -> double
    {
        if (opt.size() <= configs::EXACT_MATCH_SIZE_CUTOFF || query.size() <= configs::EXACT_MATCH_SIZE_CUTOFF)
        {
            if (opt == query) return 1.0;
            if (opt.starts_with(query)) return 0.9;
            return 0.0;
        }

        return scorer.similarity(opt) / 100.0;
    };

    for (const auto& option : *m_files)
    {
        std::string_view opt = option.m_description;

        double score = eval(opt);

        if (score <= 0.0) continue;

        if (best_n.size() < n)
        {
            best_n.push_back(Result(option, score));
            continue;
        }

        auto worst_it = std::min_element(
            best_n.begin(), best_n.end(), [](const Result& a, const Result& b) { return a.m_score < b.m_score; });

        if (worst_it->m_score < score)
        {
            worst_it->m_score = score;
            worst_it->m_file = option;
        }
    }

    std::sort(best_n.begin(), best_n.end(), [](const Result& a, const Result& b) { return a.m_score > b.m_score; });
    return best_n;
}
