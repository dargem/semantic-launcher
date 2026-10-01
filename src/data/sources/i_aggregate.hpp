#pragma once

#include "src/data/result.hpp"
#include "src/utils/index_vector.hpp"
#include <optional>
#include <unordered_map>
#include <vector>

// Abstract aggregator
class IAggregate
{
public:
    // Validity check on whether this aggregation check should be used, i.e. pacman aggregator should check this system
    // is actually using pacman as its package manager
    virtual bool check_applicable() const = 0;

    // Take files and and a membership set, adds stuff it aggregates to files (checking for duplicates with membership)
    virtual void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID>& membership) const = 0;

    virtual ~IAggregate() = default;

protected:
    static std::vector<std::string> search_terms(const File& file)
    {
        std::vector<std::string> terms;
        if (!file.m_name.empty()) terms.push_back(file.m_name);
        if (!file.m_executable.empty())
        {
            // Some apps have the same executable (e.g. steam) with "specification" via args
            // So to stop improper collisions we concat filename with arguments
            std::string term = file.m_executable.filename();
            term += '\0'; // Linux filenames can't have a null terminator in them
            term += file.m_args;
            terms.push_back(term);
        }
        return terms;
    }

    static std::optional<siv::ID> find_membership(const std::unordered_map<std::string, siv::ID>& membership,
                                                  const File& file)
    {
        for (const auto& term : search_terms(file))
        {
            if (auto it = membership.find(term); it != membership.end()) { return it->second; }
        }
        return std::nullopt;
    }

    // Assumes the file you are registering is NOT a member already
    static void register_membership(std::unordered_map<std::string, siv::ID>& membership, const File& file, siv::ID id)
    {
        for (const auto& term : search_terms(file)) { membership[term] = id; }
    }
};