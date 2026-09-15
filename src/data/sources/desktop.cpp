#include "src/data/sources/desktop.hpp"
#include "src/data/result.hpp"
#include "src/utils/index_vector.hpp"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
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
        QSettings file(desktop_file.path(), QSettings::IniFormat);
        file.beginGroup("Desktop Entry");

        // Get our file, need to consider its already in files
        const std::string name = file.value("Name").toString().toStdString();
        const std::string comment = file.value("Comment").toString().toStdString();
        const std::string exec = file.value("Exec").toString().toStdString();
        const std::string icon = file.value("Icon").toString().toStdString();
        const bool is_terminal = [&]
        {
            const QString term = file.value("Terminal").toString();
            return term == "true";
        }();

        file.endGroup();

        LaunchType lt = is_terminal ? LaunchType::TERMINAL : LaunchType::DIRECT;

        if (membership.contains(name))
        {
            // We will do an overwrite as desktop is high priority
            siv::ID id = membership[name];
            File& file = files[id];

            file.m_name = name;
            file.m_description = comment;
            file.m_executable = exec;
            file.m_icon = icon;
            file.m_launch_type = lt;
            continue;
        }

        // Make a new entry else-wise
        siv::ID id = files.push_back(File{name, exec, comment, lt, icon});
        membership.emplace(name, id);
    }
}