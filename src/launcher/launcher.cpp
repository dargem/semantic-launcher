#include "src/launcher/launcher.hpp"

#include "src/configs.hpp"
#include "src/data/result.hpp"
#include <QProcess>
#include <QProcessEnvironment>
#include <qprocess.h>
#include <stdexcept>

Launcher::Launcher()
{
    for (auto term : configs::TERMINALS)
    {
        QProcess which;
        which.start("which", {QString::fromStdString(term)});
        if (which.waitForFinished() && which.exitCode() == 0)
        {
            m_terminal_name = term;
            break;
        }
    }

    if (!m_terminal_name.empty())
        return;
    throw std::runtime_error("No terminal found, check configs.hpp");
}

void Launcher::launch(File& f) const
{
    QString program = QString::fromStdString(f.m_executable.string());
    QStringList arguments = QProcess::splitCommand(QString::fromStdString(f.m_args));

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("QT_WAYLAND_SHELL_INTEGRATION");
    process.setProcessEnvironment(env);

    switch (f.m_launch_type)
    {
    case LaunchType::TERMINAL:
    {
        QString inner_cmd = program;
        for (const QString& arg : arguments)
            inner_cmd += " " + arg;
        inner_cmd += "; exec bash";

        QStringList term_args = {"-e", "bash", "-c", inner_cmd};
        process.setProgram(QString::fromStdString(m_terminal_name));
        process.setArguments(term_args);
        process.startDetached();
        break;
    }
    case LaunchType::DIRECT:
    {
        process.setProgram(program);
        process.setArguments(arguments);
        process.startDetached();
        break;
    }
    }
}