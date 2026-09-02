#include "src/engine/search_engine.hpp"
#include <stdexcept>
#include <iostream>

QVariant SearchResultModel::data(const QModelIndex &index, int role) const 
{
    const auto& e = m_results[index.row()];

    switch(role) 
    {
        case NameRole:
            return QString::fromStdString(e.name);
        case IconRole:
            return QString::fromStdString(e.icon.string());
        case ExecRole:
            return QString::fromStdString(e.executable.string());
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


Q_INVOKABLE void SearchEngine::search(const QString& query) {
    std::cout << query.toStdString() << std::endl;
    // The current query being searched for, change the results in response to this
}

// Launching index x
Q_INVOKABLE void SearchEngine::launch(int index) {
    // For now just pass but later fork and launch app
}