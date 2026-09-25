#include "src/data/sources/appimage.hpp"
#include "src/utils/desktop_utils.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

bool AppImage::check_applicable() const
{
    return !QStandardPaths::findExecutable("find").isEmpty();
}

void AppImage::aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const
{
    // find ~ -iname "*.appimage"
    QProcess find_process;
    find_process.start("find", {QDir::homePath(), "-iname", "*.appimage"});
    if (!find_process.waitForFinished(10000)) // 10 seconds timeout for find
    {
        find_process.kill();
    }

    QStringList appimage_paths = QString(find_process.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

    for (const QString& appimage_path : appimage_paths)
    {
        if (appimage_path.isEmpty())
            continue;

        QFileInfo appimage_info(appimage_path);
        std::string fallback_name = appimage_info.completeBaseName().toStdString();

        bool processed = false;

        QTemporaryDir temp_dir;
        if (temp_dir.isValid())
        {
            QString temp_path = temp_dir.path();

            QProcess extract_process;
            extract_process.setWorkingDirectory(temp_path);
            extract_process.start(appimage_path, {"--appimage-extract", "*.desktop"});
            if (extract_process.waitForFinished(3000)) // 3 seconds timeout
            {
                QDir squash_dir(temp_path + "/squashfs-root");
                if (squash_dir.exists())
                {
                    QDirIterator it(
                        squash_dir.path(), QStringList() << "*.desktop", QDir::Files, QDirIterator::Subdirectories);
                    if (it.hasNext())
                    {
                        QString desktop_path = it.next();
                        QFileInfo desktop_info(desktop_path);
                        if (desktop_info.exists())
                        {
                            auto entry_opt = DesktopUtils::load_entry(desktop_info);
                            processed = true;
                            if (!entry_opt)
                            {
                                continue;
                            }
                            const File& entry = *entry_opt;
                            std::string name = entry.m_name.empty() ? fallback_name : entry.m_name;

                            if (!membership.contains(name))
                            {
                                siv::ID id = files.push_back(File{name,
                                                                  std::filesystem::path(appimage_path.toStdString()),
                                                                  entry.m_args,
                                                                  entry.m_description,
                                                                  entry.m_launch_type,
                                                                  entry.m_icon});
                                membership.emplace(name, id);
                            }
                        }
                    }
                }
            }
        }

        if (!processed)
        {
            if (!membership.contains(fallback_name))
            {
                siv::ID id = files.push_back(File{fallback_name,
                                                  std::filesystem::path(appimage_path.toStdString()),
                                                  "",
                                                  "AppImage Application",
                                                  LaunchType::DIRECT,
                                                  std::nullopt});
                membership.emplace(fallback_name, id);
            }
        }
    }
}
