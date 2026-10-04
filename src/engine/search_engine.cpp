#include "src/engine/search_engine.hpp"
#include "src/configs.hpp"
#include "src/ranker/fuzzy_ranker.hpp"
#include "src/ranker/semantic_ranker.hpp"
#include <QProcess>
#include <QString>
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

SearchEngine::SearchEngine(Launcher launcher, QObject* parent)
    : QObject(parent),
      m_model(),
      m_database(
          []
          {
              std::vector<std::unique_ptr<IRanker>> rankers;
              rankers.push_back(std::make_unique<FuzzyRanker>());
              rankers.push_back(std::make_unique<SemanticRanker>(Embedder(configs::resolve_model_path())));
              return rankers;
          }()),
      m_launcher(launcher)
{
}

QVariant SearchResultModel::data(const QModelIndex& index, int role) const
{
    const auto& e = m_results[index.row()];

    switch (role)
    {
    case NameRole: return QString::fromStdString(e.m_file.m_name);
    case IconRole: return e.m_file.m_icon.has_value() ? QString::fromStdString(e.m_file.m_icon->string()) : QString("");
    case ExecRole: return QString::fromStdString(e.m_file.m_executable.string());
    case ScoreRole: return e.m_score;
    case DescriptionRole: return QString::fromStdString(e.m_file.m_description);
    case IsTerminalRole: return e.m_file.m_launch_type == LaunchType::TERMINAL;
    }

    throw std::runtime_error("Invalid Role Requested");
}

QHash<int, QByteArray> SearchResultModel::roleNames() const
{
    return {{NameRole, "Name"},
            {IconRole, "Icon"},
            {ExecRole, "execPath"},
            {ScoreRole, "Score"},
            {DescriptionRole, "Description"},
            {IsTerminalRole, "IsTerminal"}};
}

void SearchResultModel::set_results(const QList<Result>& results)
{
    size_t old_size = m_results.size();
    size_t new_size = results.size();

    // Trim excess if new is smaller
    if (new_size < old_size)
    {
        beginRemoveRows(QModelIndex(), new_size, old_size - 1);
        m_results = m_results.mid(0, new_size);
        endRemoveRows();
    }

    // Fill in with new data
    size_t common_size = std::min(old_size, new_size);
    if (common_size > 0)
    {
        for (size_t i = 0; i < common_size; ++i) { m_results[i] = results[i]; }
        emit dataChanged(index(0), index(common_size - 1));
    }

    // Insert new rows if the new list is larger
    if (new_size > old_size)
    {
        beginInsertRows(QModelIndex(), old_size, new_size - 1);
        for (size_t i = old_size; i < new_size; ++i) { m_results.append(results[i]); }
        endInsertRows();
    }
}

Q_INVOKABLE void SearchEngine::search(const QString& query)
{
    if (query.isEmpty())
    {
        m_model.set_results(QList<Result>());
        return;
    }

    auto results = m_database.get_best(query.toStdString(), 5, 0.3);

    m_model.set_results(QList<Result>(results.begin(), results.end()));
}

// Launching result of that index
Q_INVOKABLE void SearchEngine::launch(int index)
{
    Result result = m_model.get_result(index);
    if (result.m_file.m_executable.empty()) { return; }

    m_launcher.launch(result.m_file);
}