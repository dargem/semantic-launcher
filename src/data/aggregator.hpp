#pragma once

#include "src/utils/index_vector.hpp"
#include <QFileInfo>
#include <QProcess>
#include <src/data/result.hpp>
#include <unordered_set>

// A config file selecting what the aggregator should aggregate from
struct AggregatorConfig
{
    bool m_pacman{};
    bool m_yay{};
    bool m_desktop{}; // .desktop
    bool m_flatpaks{};
    bool m_appimages{};
};

void aggregate_pacman(siv::Vector<File>& files, std::unordered_set<std::string> membership)
{
    // pacman -Qqe get explicitly installed packages
    QProcess get_explicit_packages;
    get_explicit_packages.start("pacman", {"-Qqe"});
    get_explicit_packages.waitForFinished();
    QStringList pkg_names = QString(get_explicit_packages.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

    // filter out which packages aren't executable
    QProcess check_executable;
    QStringList check_args = {"-Ql"};
    QStringList processed_pkg_names{};

    std::vector<File> temp_files;
    temp_files.reserve(pkg_names.size());

    for (auto name : pkg_names)
    {
        check_args.push_back(name);
        check_executable.start("pacman", check_args);
        check_args.pop_back();
        check_executable.waitForFinished();
        QStringList lines = QString(get_explicit_packages.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

        for (auto path : lines)
        {
            if (path.endsWith("/"))
                continue; // Directory

            if (path.startsWith("/usr/bin/") || path.startsWith("/usr/local/bin/") || path.startsWith("/opt/"))
            {
                QFileInfo file(path);
                if (file.exists() && file.isExecutable() && file.isFile())
                {
                    temp_files.push_back(File{name.toStdString(), std::filesystem::path(path.toStdString())});
                }
            }
        }
    }

    // We have files but for each need to fill in description and add them
    QProcess get_info;
    QStringList args = {"-Qi"};
    for (auto& file : temp_files)
    {
        args += QString::fromStdString(file.m_name);
    }
    get_info.start("pacman", args);
    get_info.waitForFinished();
    QString info_blob = get_info.readAllStandardOutput();

    // split on blank-line-separated blocks, one per package
    const QStringList blocks = info_blob.split("\n\n", Qt::SkipEmptyParts);
    for (auto const [i, block] : std::views::enumerate(blocks))
    {
        QString name, description;
        for (const QString& line : block.split('\n'))
        {
            if (line.startsWith("Description"))
            {
                temp_files[i].m_description = line.toStdString();

                if (!membership.contains(temp_files[i].m_name))
                {
                    // Insert if not already in it
                    membership.insert(temp_files[i].m_name);
                    files.push_back(temp_files[i]);
                }
            }
        }
    }
}

siv::Vector<File>&& aggregate_results(AggregatorConfig config)
{
    siv::Vector<File> files;

    // Want to dedup entries which we do through checking membership in the set
    std::unordered_set<std::string> membership;

    if (config.m_pacman)
    {
        aggregate_pacman(files, membership);
    }
}
