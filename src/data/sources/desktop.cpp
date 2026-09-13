#include "src/data/sources/desktop.hpp"
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

void Desktop::aggregate(siv::Vector<File>& files, std::unordered_set<std::string> membership) const
{
    QDir dir(QString::fromStdString(m_desktop_folder->string()));

    const auto desktop_files = dir.entryInfoList({"*.desktop"}, QDir::Files);

    for (const QFileInfo& desktop_file : desktop_files)
    {
        QSettings file(desktop_file.path(), QSettings::IniFormat);
        file.beginGroup("Desktop Entry");

        // Get our file, need to consider its already in files
        const std::string name = file.value("Name").toString().toStdString();
        const std::string icon = file.value("Icon").toString().toStdString();

        file.endGroup();
    }
}