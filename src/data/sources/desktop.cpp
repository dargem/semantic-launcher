#include "src/data/sources/desktop.hpp"
#include "src/data/result.hpp"
#include "src/utils/index_vector.hpp"
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <iostream>
#include <qsettings.h>

Desktop::Desktop()
{
    QString desktop_dir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    if (desktop_dir.size() != 0)
    {
        m_desktop_folder = desktop_dir.toStdString();
    }
}

bool Desktop::check_applicable() const
{
    QDir dir(QString::fromStdString(m_desktop_folder->string()));

    return dir.exists();
}

void Desktop::aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const
{
    QDir dir(QString::fromStdString(m_desktop_folder->string()));

    const auto desktop_files = dir.entryInfoList({"*.desktop"}, QDir::Files);

    for (const QFileInfo& desktop_file : desktop_files)
    {
        auto entry = load_entry(desktop_file);

        QStringList split = QProcess::splitCommand(QString::fromStdString(entry.m_exec));
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

        if (membership.contains(entry.m_name))
        {
            // We will do an overwrite as desktop is high priority
            siv::ID id = membership[entry.m_name];
            File& file = files[id];

            file.m_name = entry.m_name;
            file.m_description = entry.m_comment;
            file.m_executable = exec_path;
            file.m_args = args;
            file.m_icon = entry.m_icon;
            file.m_launch_type = entry.m_launch_type;
            continue;
        }

        // Make a new entry else-wise
        siv::ID id =
            files.push_back(File{entry.m_name, entry.m_exec, args, entry.m_comment, entry.m_launch_type, entry.m_icon});
        membership.emplace(entry.m_name, id);
    }
}

DesktopEntry Desktop::load_entry(const QFileInfo& desktop_file)
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
    return DesktopEntry{name, comment, exec, icon, lt};
}