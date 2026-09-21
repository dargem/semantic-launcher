#include "src/data/sources/pacman.hpp"

#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <unordered_set>
#include <vector>

// Check if pacman is available
bool Pacman::check_applicable() const
{
    QString pacman = QStandardPaths::findExecutable("pacman");
    return !pacman.isEmpty();
}

void Pacman::aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const
{
    // pacman -Qqe get explicitly installed packages
    QProcess get_explicit_packages;
    get_explicit_packages.start("pacman", {"-Qqe"});
    get_explicit_packages.waitForFinished();
    QStringList pkg_names = QString(get_explicit_packages.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

    // filter out which packages aren't executable
    QProcess check_executable;
    QStringList check_args = {"-Ql"};

    struct TempFile {
        std::string pkg_name;
        File file;
    };

    std::vector<TempFile> temp_files;
    temp_files.reserve(pkg_names.size());

    for (auto name : pkg_names)
    {
        check_args.push_back(name);
        check_executable.start("pacman", check_args);
        check_args.pop_back();
        check_executable.waitForFinished();
        QStringList lines = QString(check_executable.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

        for (auto line : lines)
        {
            QString path = line.section(' ', 1); // everything after the first space
            if (path.endsWith("/"))
                continue; // Directory

            if (path.startsWith("/usr/bin/") || path.startsWith("/usr/local/bin/") || path.startsWith("/opt/"))
            {
                QFileInfo file_info(path);
                if (file_info.exists() && file_info.isExecutable() && file_info.isFile())
                {
                    std::string exec_name = file_info.fileName().toStdString();
                    temp_files.push_back(TempFile{
                        name.toStdString(),
                        File{exec_name, std::filesystem::path(path.toStdString())}
                    });
                }
            }
        }
    }

    if (temp_files.size() == 0)
    {
        // If we don't append any args we're going to get nonsense output and thing will segfault
        // return early as we have nothing to aggregate for this edge case, this case likely occurs due to a bug
        // so probably log but later job
        return;
    }

    // We have files but for each need to fill in description and add them
    QProcess get_info;
    QStringList args = {"-Qi"};
    QStringList unique_pkg_names;
    std::unordered_set<std::string> unique_pkgs;
    for (auto& temp : temp_files)
    {
        if (!unique_pkgs.contains(temp.pkg_name))
        {
            unique_pkgs.insert(temp.pkg_name);
            unique_pkg_names += QString::fromStdString(temp.pkg_name);
        }
    }

    args += unique_pkg_names;
    get_info.start("pacman", args);
    get_info.waitForFinished();
    QString info_blob = get_info.readAllStandardOutput();

    // Parse the info_blob into a map of package_name -> description
    std::unordered_map<std::string, std::string> pkg_descriptions;
    const QStringList blocks = info_blob.split("\n\n", Qt::SkipEmptyParts);
    for (const QString& block : blocks)
    {
        QString name, description;
        for (const QString& line : block.split('\n'))
        {
            if (line.startsWith("Name"))
            {
                name = line.section(':', 1).trimmed();
            }
            else if (line.startsWith("Description"))
            {
                description = line.section(':', 1).trimmed();
            }
        }
        if (!name.isEmpty())
        {
            pkg_descriptions[name.toStdString()] = description.toStdString();
        }
    }

    // Now assign descriptions and add to files
    for (auto& temp : temp_files)
    {
        if (pkg_descriptions.contains(temp.pkg_name))
        {
            temp.file.m_description = pkg_descriptions[temp.pkg_name];
        }

        if (!membership.contains(temp.file.m_name))
        {
            siv::ID id = files.push_back(temp.file);
            membership.emplace(temp.file.m_name, id);
        }
    }
}