#pragma once

#include "src/ranker/i_ranker.hpp"

class FuzzyRanker : public IRanker
{
public:
    FuzzyRanker() = default;
    ~FuzzyRanker() override = default;

    void build_index(siv::Vector<File>& files) override;

    std::vector<Result> get_best(std::string_view query, size_t n, double cut_off = 0.0) const override;

private:
    const siv::Vector<File>* m_files{nullptr};
};
