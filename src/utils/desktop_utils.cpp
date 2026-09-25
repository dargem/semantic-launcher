#include "src/utils/desktop_utils.hpp"
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <iostream>

File DesktopUtils::load_entry(const QFileInfo& desktop_file)
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
