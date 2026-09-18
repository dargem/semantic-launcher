#include "src/launcher/launcher.hpp"

#include "src/configs.hpp"
#include "src/data/result.hpp"
#include "sys/wait.h"
#include <QProcess>
#include <qprocess.h>
#include <stdexcept>
#include <unistd.h>

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

    throw std::runtime_error("No terminal found, check configs.hpp");
}

bool Launcher::launch(File& f) const
{
    // Can be launching this file by command or terminal depending if its gui or not

    // Need to use a double fork to prevent zombies (spawn grandchild process)
    pid_t pid = fork();

    if (pid == 0)
    {
        // We are first child
        pid_t child_pid = fork();

        if (child_pid == 0)
        {
            // We are grandchild and can exec our process now
            switch (f.m_launch_type)
            {
            case LaunchType::TERMINAL:
                break;
            case LaunchType::DIRECT:
                break;
            }
        }
    }

    int status;
    waitpid(pid, &status, 0); // Blocking until spawned

    if (status == -1)
    {
        // Error has occurred need to log later
    }
}
