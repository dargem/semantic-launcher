#pragma once

#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <functional>
#include <string_view>

namespace ipc
{

enum class Command
{
    Toggle,
    Show,
    Hide,
    Quit,
    Ping,
    Status,
    Daemon,
    Help,
    Unknown
};

Command parse_command(std::string_view str);
Command parse_command(const char* str);
QString command_to_string(Command cmd);
QString socket_path();

bool send_command(Command command, QString* response = nullptr, int timeout_ms = 2000);

class Server : public QObject
{
    Q_OBJECT

public:
    using Handler = std::function<void(Command, QLocalSocket*)>;

    explicit Server(Handler handler, QObject* parent = nullptr);
    ~Server() override;

    bool listen();
    void close();

private slots:
    void onNewConnection();

private:
    QLocalServer m_server;
    QString m_path;
    Handler m_handler;
};

} // namespace ipc
