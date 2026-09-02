#include "src/engine/search_engine.hpp"
#include <stdexcept>

QVariant SearchResultModel::data(const QModelIndex &index, int role) const 
{
    const auto& e = m_results[index.row()];

    switch(role) 
    {
        case NameRole:
            return QString::fromStdString(e.name);
        case IconRole:
            break; // Return icon path later
        case ExecRole:
            break; // Return execution path later
    }

    throw std::runtime_error("Invalid Role Requested");
}

QHash<int, QByteArray> SearchResultModel::roleNames() const 
{
    return {
        { NameRole, "Name" },
        { IconRole, "Icon" },
        { ExecRole, "execPath" }
    };
}

void SearchResultModel::setResults(const QList<Result>& results) {
    m_results = results;
}