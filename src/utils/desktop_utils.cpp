#include "src/utils/desktop_utils.hpp"
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <iostream>

namespace
{
std::string read_entry_string(const QSettings& settings, const QString& key)
{
    const QVariant val = settings.value(key);
    if (val.typeId() == QMetaType::QStringList)
    {
        // QSettings parses unquoted commas as list delimiters
        return val.toStringList().join(", ").toStdString();
    }
    return val.toString().toStdString();
}
} // namespace

std::optional<File> DesktopUtils::load_entry(const QFileInfo& desktop_file)
{
    QSettings file(desktop_file.filePath(), QSettings::IniFormat);
    file.beginGroup("Desktop Entry");

    // Get our file, need to consider its already in files
    const std::string name = read_entry_string(file, "Name");
    const std::string comment = read_entry_string(file, "Comment");
    const std::string exec = read_entry_string(file, "Exec");
    const std::string icon = read_entry_string(file, "Icon");
    const bool no_display = file.value("NoDisplay").toBool();
    const bool appstream_ignore = file.value("X-AppStream-Ignore").toBool();

    if (exec == "/usr/bin/false" || no_display || appstream_ignore)
    {
        file.endGroup();
        return std::nullopt;
    }

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
            if (arg.contains(' ')) { filtered_args.append("\"" + arg + "\""); }
            else
            {
                filtered_args.append(arg);
            }
        }
        args = filtered_args.join(' ').toStdString();
    }

    std::optional<std::filesystem::path> icon_path;
    if (!icon.empty()) { icon_path = std::filesystem::path(icon); }

    return File{name, std::filesystem::path(exec_path), args, comment, lt, icon_path};
}
