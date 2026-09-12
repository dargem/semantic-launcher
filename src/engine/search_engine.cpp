#include "src/engine/search_engine.hpp"
#include <QProcess>
#include <QString>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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
        for (size_t i = 0; i < common_size; ++i)
        {
            m_results[i] = results[i];
        }
        emit dataChanged(index(0), index(common_size - 1));
    }

    // Insert new rows if the new list is larger
    if (new_size > old_size)
    {
        beginInsertRows(QModelIndex(), old_size, new_size - 1);
        for (size_t i = old_size; i < new_size; ++i)
        {
            m_results.append(results[i]);
        }
        endInsertRows();
    }
}

Q_INVOKABLE void SearchEngine::search(const QString& query)
{
    if (query.size() == 0)
    {
        m_model.set_results(QList<Result>());
        return;
    }

    auto results = m_database.get_match_best(query.toStdString(), 3, 0.4);
    results.append_range(m_database.get_semantic_best(query.toStdString(), 3, 0.3));

    // need to dedup results
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

// Launching result of that index
Q_INVOKABLE void SearchEngine::launch(int index)
{
    Result result = m_model.get_result(index);
    if (result.m_file.m_executable.empty())
    {
        return;
    }

    std::filesystem::path executable = result.m_file.m_executable;

    // Helper to check if it's a TUI app
    auto is_tui_app = [](const std::filesystem::path& path) -> bool
    {
        QProcess ldd;
        ldd.start("ldd", {QString::fromStdString(path.string())});
        if (!ldd.waitForFinished(1000))
        {
            return false;
        }

        QString output = ldd.readAllStandardOutput();
        if (output.contains("not a dynamic executable"))
        {
            // It's a script. Most scripts in /usr/bin are CLI/TUI tools unless they launch a GUI.
            return true;
        }

        // List of GUI libraries. If the binary links to any of these, it's a GUI app.
        static const QStringList gui_libs = {"libX11.so",
                                             "libwayland-client.so",
                                             "libxcb.so",
                                             "libgtk-3.so",
                                             "libgtk-4.so",
                                             "libgdk-3.so",
                                             "libgdk-4.so",
                                             "libQt6Gui.so",
                                             "libQt5Gui.so",
                                             "libQt6Widgets.so",
                                             "libQt5Widgets.so",
                                             "libSDL2.so",
                                             "libSDL-1.2.so",
                                             "libglfw.so",
                                             "libglut.so"};

        for (const auto& lib : gui_libs)
        {
            if (output.contains(lib))
            {
                return false; // It's a GUI app
            }
        }

        return true; // If it doesn't link to GUI libs, assume TUI/CLI
    };

    // Helper to find an available terminal emulator
    auto find_terminal_emulator = []() -> std::string
    {
        static const std::vector<std::string> terminals = {
            "kitty", "alacritty", "wezterm", "konsole", "gnome-terminal", "xfce4-terminal", "foot", "xterm"};

        for (const auto& term : terminals)
        {
            QProcess which;
            which.start("which", {QString::fromStdString(term)});
            if (which.waitForFinished() && which.exitCode() == 0)
            {
                return term;
            }
        }
        return "";
    };

    bool is_tui = is_tui_app(executable);

    pid_t pid = fork();
    if (pid == 0)
    {
        // Double fork to prevent zombie processes
        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            // Grandchild process
            // Redirect stdout/stderr to /dev/null to avoid cluttering our launcher's output
            int dev_null = open("/dev/null", O_WRONLY);
            if (dev_null >= 0)
            {
                dup2(dev_null, STDOUT_FILENO);
                dup2(dev_null, STDERR_FILENO);
                close(dev_null);
            }

            if (is_tui)
            {
                std::string term = find_terminal_emulator();
                if (!term.empty())
                {
                    std::string exec_str = executable.string();
                    if (term == "gnome-terminal")
                    {
                        execlp(term.c_str(), term.c_str(), "--", exec_str.c_str(), nullptr);
                    }
                    else if (term == "wezterm")
                    {
                        execlp(term.c_str(), term.c_str(), "start", "--", exec_str.c_str(), nullptr);
                    }
                    else
                    {
                        execlp(term.c_str(), term.c_str(), "-e", exec_str.c_str(), nullptr);
                    }
                }
            }

            // If not TUI, or if terminal launch failed, run directly
            execl(executable.c_str(), executable.c_str(), nullptr);
            execlp(executable.c_str(), executable.c_str(), nullptr);

            std::cerr << "Failed to execute: " << executable << std::endl;
            _exit(1);
        }
        _exit(0);
    }

    int status;
    waitpid(pid, &status, 0);
}