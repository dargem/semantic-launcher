#include "src/daemon/ipc.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <iostream>

namespace ipc
{

Command parse_command(std::string_view str)
{
    while (!str.empty() && (str.front() == ' ' || str.front() == '\t' || str.front() == '\r' || str.front() == '\n'))
        str.remove_prefix(1);
    while (!str.empty() && (str.back() == ' ' || str.back() == '\t' || str.back() == '\r' || str.back() == '\n'))
        str.remove_suffix(1);

    if (str == "toggle" || str == "--toggle" || str == "-t") return Command::Toggle;
    if (str == "show" || str == "--show") return Command::Show;
    if (str == "hide" || str == "--hide") return Command::Hide;
    if (str == "quit" || str == "--quit" || str == "-q") return Command::Quit;
    if (str == "status" || str == "--status" || str == "-s") return Command::Status;
    if (str == "daemon" || str == "--daemon" || str == "-d") return Command::Daemon;
    if (str == "help" || str == "--help" || str == "-h") return Command::Help;
    if (str == "ping") return Command::Ping;

    return Command::Unknown;
}

Command parse_command(const char* str) { return str ? parse_command(std::string_view(str)) : Command::Unknown; }

QString command_to_string(Command cmd)
{
    switch (cmd)
    {
    case Command::Toggle: return QStringLiteral("toggle");
    case Command::Show: return QStringLiteral("show");
    case Command::Hide: return QStringLiteral("hide");
    case Command::Quit: return QStringLiteral("quit");
    case Command::Ping: return QStringLiteral("ping");
    case Command::Status: return QStringLiteral("status");
    case Command::Daemon: return QStringLiteral("daemon");
    case Command::Help: return QStringLiteral("help");
    case Command::Unknown: return QStringLiteral("unknown");
    }
    return QStringLiteral("unknown");
}

QString socket_path()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    return (dir.isEmpty() ? QDir::tempPath() : dir) + "/semantic-launcher.sock";
}

bool send_command(Command command, QString* response, int timeout_ms)
{
    QLocalSocket socket;
    socket.connectToServer(socket_path());
    if (!socket.waitForConnected(timeout_ms)) return false;

    QByteArray data = command_to_string(command).toUtf8() + "\n";
    if (socket.write(data) == -1 || !socket.waitForBytesWritten(timeout_ms)) return false;

    if (socket.waitForReadyRead(timeout_ms))
    {
        QByteArray resp = socket.readAll();
        if (response) *response = QString::fromUtf8(resp).trimmed();
    }
    socket.disconnectFromServer();
    return true;
}

Server::Server(Handler handler, QObject* parent)
    : QObject(parent), m_server(this), m_path(socket_path()), m_handler(std::move(handler))
{ connect(&m_server, &QLocalServer::newConnection, this, &Server::onNewConnection); }

Server::~Server() { close(); }

void Server::close()
{
    if (m_server.isListening()) m_server.close();
    QLocalServer::removeServer(m_path);
    QFile::remove(m_path);
}

bool Server::listen()
{
    QString ping_resp;
    if (send_command(Command::Ping, &ping_resp, 300))
    {
        std::cerr << "Another instance of semantic-launcher is already running.\n";
        return false;
    }

    close();
    QDir().mkpath(QFileInfo(m_path).dir().absolutePath());

    if (!m_server.listen(m_path))
    {
        std::cerr << "Failed to start IPC server: " << m_server.errorString().toStdString() << "\n";
        return false;
    }

    std::cout << "[daemon] Listening on: " << m_path.toStdString() << "\n";
    return true;
}

void Server::onNewConnection()
{
    while (m_server.hasPendingConnections())
    {
        QLocalSocket* client = m_server.nextPendingConnection();
        if (!client) continue;

        connect(client,
                &QLocalSocket::readyRead,
                this,
                [this, client]()
                {
                    QByteArray line = client->readLine();
                    Command cmd = parse_command(std::string_view(line.constData(), line.size()));

                    if (m_handler) m_handler(cmd, client);

                    client->flush();
                    client->disconnectFromServer();
                });
        connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);
    }
}

} // namespace ipc
