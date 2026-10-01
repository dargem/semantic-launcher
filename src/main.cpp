#include <LayerShellQt/window.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSocketNotifier>
#include <QWindow>
#include <QtCore/qglobal.h>

#include <csignal>
#include <iostream>
#include <optional>
#include <sys/socket.h>
#include <unistd.h>

#include "src/daemon/ipc.hpp"
#include "src/engine/search_engine.hpp"

static int sig_fd[2];

static void signal_handler(int sig)
{
    char a = static_cast<char>(sig);
    ::write(sig_fd[0], &a, sizeof(a));
}

static void setup_signals(QObject* parent, std::function<void()> on_toggle)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sig_fd)) return;

    auto* sn = new QSocketNotifier(sig_fd[1], QSocketNotifier::Read, parent);
    QObject::connect(sn,
                     &QSocketNotifier::activated,
                     [on_toggle]()
                     {
                         char sig = 0;
                         if (::read(sig_fd[1], &sig, sizeof(sig)) > 0)
                         {
                             if (sig == SIGUSR1) on_toggle();
                             else if (sig == SIGINT || sig == SIGTERM) QGuiApplication::quit();
                         }
                     });

    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGUSR1, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

static void print_usage(const char* prog)
{
    std::cout << "Usage: " << prog << " [options]\n\n"
              << "Options:\n"
              << "  -d, --daemon     Run as background daemon (UI starts hidden)\n"
              << "  -t, --toggle     Toggle launcher UI on running daemon\n"
              << "      --show       Show launcher UI on running daemon\n"
              << "      --hide       Hide launcher UI on running daemon\n"
              << "  -s, --status     Check if daemon is running\n"
              << "  -q, --quit       Stop running daemon\n"
              << "  -h, --help       Display this help message\n";
}

int main(int argc, char* argv[])
{
    std::optional<ipc::Command> command;

    for (int i = 1; i < argc; ++i)
    {
        auto parsed = ipc::parse_command(argv[i]);
        if (parsed == ipc::Command::Unknown)
        {
            std::cerr << "Unknown argument: " << argv[i] << "\n";
            print_usage(argv[0]);
            return 1;
        }
        command = parsed;
    }

    if (command == ipc::Command::Help)
    {
        print_usage(argv[0]);
        return 0;
    }

    // Client mode (toggle, show, hide, quit, status)
    if (command.has_value() && *command != ipc::Command::Daemon)
    {
        QString response;
        if (ipc::send_command(*command, &response))
        {
            std::cout << response.toStdString() << "\n";
            return 0;
        }
        std::cerr << "semantic-launcher: daemon is not running.\n"
                  << "Hint: Start it with 'systemctl --user start semantic-launcher' or '" << argv[0] << " --daemon'\n";
        return 1;
    }

    // If launched without args and daemon is already running, toggle it
    if (!command.has_value())
    {
        QString response;
        if (ipc::send_command(ipc::Command::Toggle, &response, 1000))
        {
            std::cout << response.toStdString() << "\n";
            return 0;
        }
    }

    // Daemon / Foreground mode
    try
    {
        qputenv("QT_QPA_PLATFORM", "wayland");
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "layer-shell");

        QGuiApplication app(argc, argv);
        QGuiApplication::setQuitOnLastWindowClosed(false);

        std::cout << "[daemon] Indexing applications..." << std::endl;
        Launcher launcher;
        SearchEngine search_engine(launcher);
        std::cout << "[daemon] Indexing complete. Launcher ready." << std::endl;

        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("searchEngine", &search_engine);
        engine.loadFromModule("SemanticLauncher", "Main");

        if (engine.rootObjects().isEmpty()) return -1;

        auto* window = qobject_cast<QWindow*>(engine.rootObjects().constFirst());
        if (!window) return -1;

        window->setVisible(false);

        auto* layer_shell = LayerShellQt::Window::get(window);
        layer_shell->setLayer(LayerShellQt::Window::LayerOverlay);
        layer_shell->setAnchors(LayerShellQt::Window::AnchorTop);
        layer_shell->setMargins(QMargins(0, 100, 0, 0));
        layer_shell->setExclusiveEdge(LayerShellQt::Window::AnchorTop);
        layer_shell->setDesiredSize(window->size());
        layer_shell->setExclusiveZone(0);
        layer_shell->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        layer_shell->setScope(QStringLiteral("semantic-launcher"));
        layer_shell->setWantsToBeOnActiveScreen(true);
        layer_shell->setActivateOnShow(true);

        QObject::connect(window,
                         &QWindow::widthChanged,
                         window,
                         [window, layer_shell]() { layer_shell->setDesiredSize(window->size()); });
        QObject::connect(window,
                         &QWindow::heightChanged,
                         window,
                         [window, layer_shell]() { layer_shell->setDesiredSize(window->size()); });

        auto show_launcher = [window, layer_shell]()
        {
            layer_shell->setWantsToBeOnActiveScreen(true);
            window->show();
            window->raise();
            window->requestActivate();
        };

        auto hide_launcher = [window]() { window->hide(); };

        auto toggle_launcher = [&]()
        {
            if (window->isVisible()) hide_launcher();
            else show_launcher();
        };

        ipc::Server server(
            [&](ipc::Command cmd, QLocalSocket* client)
            {
                switch (cmd)
                {
                case ipc::Command::Toggle:
                    toggle_launcher();
                    client->write("ok: toggle\n");
                    break;
                case ipc::Command::Show:
                    show_launcher();
                    client->write("ok: show\n");
                    break;
                case ipc::Command::Hide:
                    hide_launcher();
                    client->write("ok: hide\n");
                    break;
                case ipc::Command::Status:
                    client->write(window->isVisible() ? "running (visible)\n" : "running (hidden)\n");
                    break;
                case ipc::Command::Ping: client->write("pong\n"); break;
                case ipc::Command::Quit:
                    client->write("ok: quitting\n");
                    client->flush();
                    server.close();
                    app.quit();
                    break;
                default: client->write("error: unknown command\n"); break;
                }
            });

        if (!server.listen()) return 1;

        setup_signals(&app, toggle_launcher);

        if (command != ipc::Command::Daemon) show_launcher();

        return app.exec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}