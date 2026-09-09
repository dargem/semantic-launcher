#pragma once

#include "src/utils/index_vector.hpp"
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

void aggregate_pacman(siv::Vector<Result>& results, std::unordered_set<std::string> membership)
{
    // pacman -Qqe get explicitly installed packages
    QProcess get_explicit_packages;
    get_explicit_packages.start("pacman", {"-Qqe"});
    get_explicit_packages.waitForFinished();
    QStringList pkg_names = QString(get_explicit_packages.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

    QProcess get_info;
    QStringList args = {"-Qi"};
    args += pkg_names;
    get_info.start("pacman", args);
    get_info.waitForFinished();
    QString info_blob = get_info.readAllStandardOutput();

    // split on blank-line-separated blocks, one per package
    const QStringList blocks = info_blob.split("\n\n", Qt::SkipEmptyParts);
    for (const QString& block : blocks)
    {
        QString name, description;
        for (const QString& line : block.split('\n'))
        {
            if (line.startsWith("Name"))
                name = line.section(':', 1).trimmed();
            else if (line.startsWith("Description"))
                description = line.section(':', 1).trimmed();
        }

        if (!membership.contains(name.toStdString()))
        {
            membership.insert(name.toStdString());
            results.push_back(Result())
        }
    }
}

siv::Vector<Result>&& aggregate_results(AggregatorConfig config)
{
    siv::Vector<Result> results;

    // Want to dedup entries which we do through checking membership in the set
    std::unordered_set<std::string> membership;

    if (config.m_pacman)
    {
        aggregate_pacman(results, membership);
    }
}
