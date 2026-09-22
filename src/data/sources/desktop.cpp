#include "src/data/sources/desktop.hpp"
#include "src/data/result.hpp"
#include "src/utils/index_vector.hpp"
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <algorithm>
#include <iostream>
#include <qsettings.h>
#include <ranges>

Desktop::Desktop()
{
    QStringList desktop_dir = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    std::vector<std::filesystem::path> out;
    out.reserve(desktop_dir.size());
    for (const auto& str : desktop_dir)
    {
        out.push_back(str.toStdString());
    }
    if (out.size())
        m_desktop_folders = out;
}

bool Desktop::check_applicable() const
{
    if (m_desktop_folders->empty())
        return false;

    auto dirs =
        m_desktop_folders.value() | std::views::transform([](const std::filesystem::path& path)
                                                          { return QDir(QString::fromStdString(path.string())); });

    return std::any_of(dirs.begin(), dirs.end(), [](const QDir& dir) { return dir.exists(); });
}

void Desktop::aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const
{
    auto desktop_files =
        m_desktop_folders.value() |
        std::views::transform([](const std::filesystem::path& path)
                              { return QDir(QString::fromStdString(path.string())); }) |
        std::views::filter([](const QDir& dir) { return dir.exists(); }) |
        std::views::transform([](const QDir& dir) { return dir.entryInfoList({"*.desktop"}, QDir::Files); }) |
        std::views::join;

    for (const QFileInfo& desktop_file : desktop_files)
    {
        File entry = load_entry(desktop_file);

        if (membership.contains(entry.m_name))
        {
            // We will do an overwrite as desktop is high priority
            siv::ID id = membership[entry.m_name];
            File& file = files[id];

            file.m_name = entry.m_name;
            file.m_description = entry.m_description;
            file.m_executable = entry.m_executable;
            file.m_args = entry.m_args;
            file.m_icon = entry.m_icon;
            file.m_launch_type = entry.m_launch_type;
            continue;
        }

        // Make a new entry else-wise
        siv::ID id = files.push_back(entry);
        membership.emplace(entry.m_name, id);
    }
}

File Desktop::load_entry(const QFileInfo& desktop_file)
{
    QSettings file(desktop_file.filePath(), QSettings::IniFormat);
    file.beginGroup("Desktop Entry");

    // Get our file, need to consider its already in files
    const std::string name = file.value("Name").toString().toStdString();
    const std::string comment = file.value("Comment").toString().toStdString();
    const std::string exec = file.value("Exec").toString().toStdString();
    const std::string icon = file.value("Icon").toString().toStdString();

    std::cout << name << '\n';
    const bool is_terminal = [&]
    {
        const QString term = file.value("Terminal").toString();
        return term == "true";
    }();

    file.endGroup();

    LaunchType lt = is_terminal ? LaunchType::TERMINAL : LaunchType::DIRECT;

    QStringList split = QProcess::splitCommand(QString::fromStdString(exec));
    std::string exec_path;
    std::string args;
    if (!split.isEmpty())
    {
        exec_path = split.takeFirst().toStdString();
        QStringList filtered_args;
        for (const QString& arg : split)
        {
            if (arg.startsWith('%'))
            {
                continue; // Skip field codes like %u, %F, etc.
            }
            if (arg.contains(' '))
            {
                filtered_args.append("\"" + arg + "\"");
            }
            else
            {
                filtered_args.append(arg);
            }
        }
        args = filtered_args.join(' ').toStdString();
    }

    std::optional<std::filesystem::path> icon_path;
    if (!icon.empty())
    {
        icon_path = std::filesystem::path(icon);
    }

    return File{name, std::filesystem::path(exec_path), args, comment, lt, icon_path};
}