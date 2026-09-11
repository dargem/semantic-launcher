#include "src/engine/search_engine.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>

QVariant SearchResultModel::data(const QModelIndex& index, int role) const
{
    const auto& e = m_results[index.row()];

    switch (role)
    {
        case NameRole:
            return QString::fromStdString(e.m_file.m_name);
        case IconRole:
            return QString::fromStdString(e.m_file.m_icon->string());
        case ExecRole:
            return QString::fromStdString(e.m_file.m_executable.string());
        case ScoreRole:
            return e.m_score;
        case DescriptionRole:
            return QString::fromStdString(e.m_file.m_description);
    }

    throw std::runtime_error("Invalid Role Requested");
}

QHash<int, QByteArray> SearchResultModel::roleNames() const
{
    return {{NameRole, "Name"},
            {IconRole, "Icon"},
            {ExecRole, "execPath"},
            {ScoreRole, "Score"},
            {DescriptionRole, "Description"}};
}

void SearchResultModel::set_results(const QList<Result>& results)
{
    beginResetModel();
    m_results = results;
    endResetModel();
}

Q_INVOKABLE void SearchEngine::search(const QString& query)
{
    auto results = m_database.get_match_best(query.toStdString(), 3);
    results.append_range(m_database.get_semantic_best(query.toStdString(), 3));

    std::unordered_map<std::string, Result> merged_results;
    merged_results.reserve(results.size());

    for (const auto& result : results)
    {
        const auto key = result.m_file.m_executable.string();
        auto [it, inserted] = merged_results.try_emplace(key, result);
        if (!inserted && it->second.m_score < result.m_score)
        {
            it->second = result;
        }
    }

    std::vector<Result> ranked_results;
    ranked_results.reserve(merged_results.size());
    for (auto& [_, result] : merged_results)
    {
        ranked_results.push_back(result);
    }

    std::sort(ranked_results.begin(),
              ranked_results.end(),
              [](const Result& a, const Result& b) { return a.m_score > b.m_score; });

    m_model.set_results(QList<Result>(ranked_results.begin(), ranked_results.end()));
}

// Launching index x
Q_INVOKABLE void SearchEngine::launch(int index)
{
    // For now just pass but later fork and launch app
}