#pragma once

#include "src/ranker/i_ranker.hpp"

class FuzzyDescRanker : public IRanker
{
public:
    FuzzyDescRanker() = default;
    ~FuzzyDescRanker() override = default;

    void build_index(siv::Vector<File>& files) override;

    std::vector<Result> get_best(std::string_view query, size_t n) const override;

private:
    const siv::Vector<File>* m_files{nullptr};
};
