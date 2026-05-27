#include "ircclient.h"
#include "ircparser.h"
#include "config/config.h"

#include <QSslSocket>

IrcClient::IrcClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
{
    connect(m_socket, &QSslSocket::connected,       this, &IrcClient::onConnected);
    connect(m_socket, &QSslSocket::disconnected,    this, &IrcClient::onDisconnected);
    connect(m_socket, &QSslSocket::readyRead,       this, &IrcClient::onReadyRead);
    connect(m_socket, &QSslSocket::sslErrors,       this, &IrcClient::onSslErrors);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &IrcClient::onErrorOccurred);
}

IrcClient::~IrcClient() = default;

void IrcClient::connectToServer(const ServerConfig &cfg)
{
    m_host     = cfg.host;
    m_port     = cfg.port;
    m_ssl      = cfg.ssl;
    m_nick     = cfg.nick;
    m_user     = cfg.user;
    m_realname = cfg.realname;
    m_password = cfg.password;

    if (m_ssl)
        m_socket->connectToHostEncrypted(m_host, m_port);
    else
        m_socket->connectToHost(m_host, m_port);
}

bool IrcClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void IrcClient::quit(const QString &reason)
{
    sendRaw("QUIT :" + reason);
    m_socket->disconnectFromHost();
}

void IrcClient::join(const QString &channel, const QString &key)
{
    sendRaw(key.isEmpty() ? "JOIN " + channel : "JOIN " + channel + " " + key);
}

void IrcClient::part(const QString &channel, const QString &reason)
{
    sendRaw(reason.isEmpty() ? "PART " + channel : "PART " + channel + " :" + reason);
}

void IrcClient::privmsg(const QString &target, const QString &text)
{
    sendRaw("PRIVMSG " + target + " :" + text);
}

void IrcClient::notice(const QString &target, const QString &text)
{
    sendRaw("NOTICE " + target + " :" + text);
}

void IrcClient::setNick(const QString &nick)
{
    m_nick = nick;
    sendRaw("NICK " + nick);
}

void IrcClient::sendRaw(const QString &line)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    emit rawReceived(">> " + line);
    m_socket->write((line + "\r\n").toUtf8());
}

// ---------------------------------------------------------------------------
// Socket callbacks
// ---------------------------------------------------------------------------

void IrcClient::onConnected()
{
    // CAP negotiation before registration
    sendRaw("CAP LS 302");

    if (!m_password.isEmpty())
        sendRaw("PASS :" + m_password);

    sendRaw("NICK " + m_nick);
    sendRaw("USER " + m_user + " 0 * :" + m_realname);
}

void IrcClient::onDisconnected()
{
    m_namesBuffer.clear();
    m_requestedCaps.clear();
    emit disconnected(m_host);
}

void IrcClient::onReadyRead()
{
    m_buffer += QString::fromUtf8(m_socket->readAll());
    int idx;
    while ((idx = m_buffer.indexOf('\n')) != -1) {
        QString line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);
        if (line.isEmpty()) continue;
        emit rawReceived(line);
        processLine(line);
    }
}

void IrcClient::onSslErrors(const QList<QSslError> &errors)
{
    // For now ignore self-signed; will make configurable
    Q_UNUSED(errors)
    m_socket->ignoreSslErrors();
}

void IrcClient::onErrorOccurred(QAbstractSocket::SocketError)
{
    emit socketError(m_host, m_socket->errorString());
    emit disconnected(m_host);
}

// ---------------------------------------------------------------------------
// Line processing
// ---------------------------------------------------------------------------

void IrcClient::processLine(const QString &line)
{
    IrcMessage msg = IrcParser::parse(line);
    if (!msg.isValid()) return;

    const QString &cmd = msg.command;

    // PING
    if (cmd == "PING") {
        sendRaw("PONG :" + (msg.params.isEmpty() ? QString{} : msg.params.last()));
        return;
    }

    // CAP
    if (cmd == "CAP") {
        handleCap(msg.params, msg.trailing);
        return;
    }

    // Numerics
    bool isNumeric = false;
    cmd.toInt(&isNumeric);
    if (isNumeric) {
        handleNumeric(cmd, msg.params, msg.trailing);
        return;
    }

    // PRIVMSG / CTCP ACTION
    if (cmd == "PRIVMSG" && msg.params.size() >= 1) {
        const QString target = msg.params[0];
        const QString text   = msg.trailing;
        if (text.startsWith("\x01""ACTION ") && text.endsWith("\x01")) {
            emit actionReceived(m_host, target, msg.nick, text.mid(8, text.size() - 9));
        } else {
            emit messageReceived(m_host, target, msg.nick, text);
        }
        return;
    }

    // NOTICE
    if (cmd == "NOTICE" && msg.params.size() >= 1) {
        emit noticeReceived(m_host, msg.params[0], msg.nick, msg.trailing);
        return;
    }

    // JOIN
    if (cmd == "JOIN") {
        const QString channel = msg.params.isEmpty() ? msg.trailing : msg.params[0];
        if (msg.nick == m_nick)
            emit serverMessage(m_host, "Joined " + channel);
        emit userJoined(m_host, channel, msg.nick);
        return;
    }

    // PART
    if (cmd == "PART" && !msg.params.isEmpty()) {
        emit userParted(m_host, msg.params[0], msg.nick, msg.trailing);
        return;
    }

    // QUIT
    if (cmd == "QUIT") {
        emit userQuit(m_host, msg.nick, msg.trailing);
        return;
    }

    // NICK
    if (cmd == "NICK") {
        const QString newNick = msg.params.isEmpty() ? msg.trailing : msg.params[0];
        if (msg.nick == m_nick) {
            m_nick = newNick;
            emit selfNickChanged(m_host, newNick);
        }
        emit nickChanged(m_host, msg.nick, newNick);
        return;
    }

    // KICK
    if (cmd == "KICK" && msg.params.size() >= 2) {
        emit kicked(m_host, msg.params[0], msg.params[1], msg.nick, msg.trailing);
        return;
    }

    // MODE
    if (cmd == "MODE" && !msg.params.isEmpty()) {
        // Rebuild mode string from remaining params
        QStringList modeParts = msg.params.mid(1);
        if (!msg.trailing.isEmpty()) modeParts << msg.trailing;
        emit modesReceived(m_host, msg.params[0], modeParts.join(' '));
        return;
    }

    // TOPIC (live change)
    if (cmd == "TOPIC" && !msg.params.isEmpty()) {
        emit topicReceived(m_host, msg.params[0], msg.trailing);
        return;
    }
}

// ---------------------------------------------------------------------------
// CAP negotiation
// ---------------------------------------------------------------------------

void IrcClient::handleCap(const QStringList &params, const QString &trailing)
{
    if (params.size() < 2) return;
    const QString subCmd = params[1].toUpper();

    if (subCmd == "LS") {
        // Request caps we care about
        QStringList want;
        const QStringList available = trailing.split(' ', Qt::SkipEmptyParts);
        const QStringList desired = {
            "multi-prefix", "away-notify", "server-time",
            "message-tags", "batch", "labeled-response"
        };
        for (const QString &cap : desired)
            if (available.contains(cap))
                want << cap;

        if (!want.isEmpty()) {
            m_requestedCaps = QSet<QString>(want.begin(), want.end());
            sendRaw("CAP REQ :" + want.join(' '));
        } else {
            sendRaw("CAP END");
        }
        return;
    }

    if (subCmd == "ACK") {
        sendRaw("CAP END");
        return;
    }

    if (subCmd == "NAK") {
        sendRaw("CAP END");
        return;
    }
}

// ---------------------------------------------------------------------------
// Numeric handlers
// ---------------------------------------------------------------------------

void IrcClient::handleNumeric(const QString &cmd, const QStringList &params, const QString &trailing)
{
    const int n = cmd.toInt();

    switch (n) {
    case 1:   // RPL_WELCOME
        emit connected(m_host);
        emit serverMessage(m_host, trailing);
        break;

    case 2:   // RPL_YOURHOST
    case 3:   // RPL_CREATED
    case 4:   // RPL_MYINFO
    case 5:   // RPL_ISUPPORT
    case 251: // RPL_LUSERCLIENT
    case 252: // RPL_LUSEROP
    case 253: // RPL_LUSERUNKNOWN
    case 254: // RPL_LUSERCHANNELS
    case 255: // RPL_LUSERME
    case 265: // RPL_LOCALUSERS
    case 266: // RPL_GLOBALUSERS
    case 372: // RPL_MOTD
    case 375: // RPL_MOTDSTART
        emit serverMessage(m_host, trailing);
        break;

    case 376: // RPL_ENDOFMOTD
    case 422: // ERR_NOMOTD
        emit serverMessage(m_host, trailing);
        // Auto-join configured channels — handled by MainWindow on connected()
        break;

    case 332: // RPL_TOPIC
        if (params.size() >= 2)
            emit topicReceived(m_host, params[1], trailing);
        break;

    case 333: // RPL_TOPICWHOTIME — ignore, no signal needed yet
        break;

    case 353: { // RPL_NAMREPLY
        // params: me '=' '#channel'  trailing: nick list
        if (params.size() >= 3) {
            const QString channel = params[2];
            const QStringList nicks = trailing.split(' ', Qt::SkipEmptyParts);
            m_namesBuffer[channel] += nicks;
        }
        break;
    }

    case 366: { // RPL_ENDOFNAMES
        if (params.size() >= 2) {
            const QString channel = params[1];
            emit namesReceived(m_host, channel, m_namesBuffer.take(channel));
            emit namesDone(m_host, channel);
        }
        break;
    }

    case 433: // ERR_NICKNAMEINUSE
        setNick(m_nick + "_");
        break;

    case 431: // ERR_NONICKNAMEGIVEN
    case 432: // ERR_ERRONEUSNICKNAME
        emit serverMessage(m_host, "Nick error: " + trailing);
        break;

    default:
        if (!trailing.isEmpty())
            emit serverMessage(m_host, trailing);
        break;
    }
}
