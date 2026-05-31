#include "ircclient.h"
#include "ircparser.h"
#include "config/config.h"
#include "version.h"

#include <QFile>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslSocket>
#include <QTimer>

IrcClient::IrcClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
{
    connect(m_socket, &QSslSocket::connected,       this, &IrcClient::onConnected);
    connect(m_socket, &QSslSocket::disconnected,    this, &IrcClient::onDisconnected);
    connect(m_socket, &QSslSocket::readyRead,       this, &IrcClient::onReadyRead);
    connect(m_socket, &QSslSocket::sslErrors,       this, &IrcClient::onSslErrors);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &IrcClient::onErrorOccurred);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &IrcClient::doReconnect);

    m_pingTimer = new QTimer(this);
    m_pingTimer->setInterval(30000);
    connect(m_pingTimer, &QTimer::timeout, this, &IrcClient::sendPing);
}

IrcClient::~IrcClient() = default;

void IrcClient::connectToServer(const ServerConfig &cfg)
{
    m_reconnectTimer->stop();
    m_intentionalDisconnect = false;
    m_reconnectDelay = 5;

    m_host            = cfg.host;
    m_port            = cfg.port;
    m_ssl             = cfg.ssl;
    m_nick            = cfg.nick;
    m_user            = cfg.user;
    m_realname        = cfg.realname;
    m_password        = cfg.password;
    m_saslUser        = cfg.saslUser;
    m_saslPassword    = cfg.saslPassword;
    m_saslExternal    = cfg.saslExternal;
    m_saslPending     = false;
    m_nickservPassword = cfg.nickservPassword;
    m_bouncerType     = cfg.bouncerType;
    m_bouncerNetwork  = cfg.bouncerNetwork;

    if (m_saslExternal && !cfg.clientCertFile.isEmpty() && !cfg.clientKeyFile.isEmpty()) {
        QFile certFile(cfg.clientCertFile);
        if (certFile.open(QIODevice::ReadOnly)) {
            const QSslCertificate cert(certFile.readAll(), QSsl::Pem);
            if (!cert.isNull())
                m_socket->setLocalCertificate(cert);
        }
        QFile keyFile(cfg.clientKeyFile);
        if (keyFile.open(QIODevice::ReadOnly)) {
            const QByteArray keyData = keyFile.readAll();
            QSslKey key(keyData, QSsl::Rsa, QSsl::Pem);
            if (key.isNull()) key = QSslKey(keyData, QSsl::Ec, QSsl::Pem);
            if (!key.isNull())
                m_socket->setPrivateKey(key);
        }
    }

    if (m_ssl)
        m_socket->connectToHostEncrypted(m_host, m_port);
    else
        m_socket->connectToHost(m_host, m_port);
}

bool IrcClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

quint32 IrcClient::localIpv4() const
{
    bool ok = false;
    const quint32 v4 = m_socket->localAddress().toIPv4Address(&ok);
    return ok ? v4 : 0;
}

void IrcClient::quit(const QString &reason)
{
    m_intentionalDisconnect = true;
    m_reconnectTimer->stop();
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

void IrcClient::privmsg(const QString &target, const QString &text, const QString &replyToMsgid)
{
    if (replyToMsgid.isEmpty())
        sendRaw("PRIVMSG " + target + " :" + text);
    else
        sendRaw("@+draft/reply=" + replyToMsgid + " PRIVMSG " + target + " :" + text);
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

void IrcClient::sendTyping(const QString &channel, const QString &state)
{
    sendRaw("@+typing=" + state + " TAGMSG " + channel);
}

void IrcClient::requestHistory(const QString &target, int limit)
{
    if (!m_ackedCaps.contains("chathistory")) return;
    sendRaw(QString("CHATHISTORY LATEST %1 * %2").arg(target).arg(limit));
}

void IrcClient::markRead(const QString &target, const QDateTime &ts)
{
    if (!m_ackedCaps.contains("soju.im/read")) return;
    sendRaw("MARKREAD " + target + " timestamp=" + ts.toUTC().toString(Qt::ISODateWithMs));
}

static QString redactRawForLog(const QString &line)
{
    const QString cmd = line.section(' ', 0, 0).toUpper();
    if (cmd == "PASS")
        return "PASS :<redacted>";
    if (cmd == "AUTHENTICATE")
        return "AUTHENTICATE <redacted>";
    if (line.contains("NickServ", Qt::CaseInsensitive) &&
        line.contains("IDENTIFY", Qt::CaseInsensitive))
        return "<NickServ IDENTIFY redacted>";
    return line;
}

void IrcClient::sendRaw(const QString &line)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    emit rawReceived(">> " + redactRawForLog(line));
    m_socket->write((line + "\r\n").toUtf8());
}

// ---------------------------------------------------------------------------
// Socket callbacks
// ---------------------------------------------------------------------------

void IrcClient::onConnected()
{
    m_reconnectTimer->stop();
    m_reconnectDelay = 5;
    m_pingPending = false;
    m_pingTimer->start();
    QTimer::singleShot(2000, this, &IrcClient::sendPing);

    sendRaw("CAP LS 302");

    if (!m_password.isEmpty())
        sendRaw("PASS :" + m_password);

    sendRaw("NICK " + m_nick);
    sendRaw("USER " + m_user + " 0 * :" + m_realname);
}

void IrcClient::onDisconnected()
{
    m_pingTimer->stop();
    m_pingPending = false;
    m_namesBuffer.clear();
    m_requestedCaps.clear();
    m_ackedCaps.clear();
    m_capLsBuffer.clear();
    m_batches.clear();
    m_saslPending = false;
    emit disconnected(m_host);
    scheduleReconnect();
    m_intentionalDisconnect = false;
}

static constexpr qsizetype kMaxPendingBuffer = 64 * 1024;
static constexpr qsizetype kMaxIrcLine       = 8192;
static constexpr int       kMaxOpenBatches   = 8;
static constexpr int       kMaxBatchMessages = 1000;

void IrcClient::onReadyRead()
{
    m_buffer += QString::fromUtf8(m_socket->readAll());

    if (m_buffer.size() > kMaxPendingBuffer) {
        emit socketError(m_host, "Server sent oversized unterminated data; disconnecting");
        m_socket->disconnectFromHost();
        m_buffer.clear();
        return;
    }

    int idx;
    while ((idx = m_buffer.indexOf('\n')) != -1) {
        QString line = m_buffer.left(idx);
        m_buffer.remove(0, idx + 1);
        if (line.endsWith('\r'))
            line.chop(1);
        if (line.isEmpty()) continue;
        if (line.size() > kMaxIrcLine) {
            emit socketError(m_host, "Dropped oversized IRC line from server");
            continue;
        }
        emit rawReceived(line);
        processLine(line);
    }
}

void IrcClient::onSslErrors(const QList<QSslError> &errors)
{
    QStringList msgs;
    for (const auto &e : errors)
        msgs << e.errorString();
    emit socketError(m_host, "TLS error: " + msgs.join("; "));
    m_socket->disconnectFromHost();
}

void IrcClient::onErrorOccurred(QAbstractSocket::SocketError)
{
    emit socketError(m_host, m_socket->errorString());
    // disconnected() and scheduleReconnect() are handled by onDisconnected()
    // which Qt emits after every socket error that closes the connection.
}

void IrcClient::scheduleReconnect()
{
    if (m_intentionalDisconnect || m_reconnectTimer->isActive() || m_host.isEmpty()) return;
    emit serverMessage(m_host, QString("Reconnecting in %1s…").arg(m_reconnectDelay));
    m_reconnectTimer->start(m_reconnectDelay * 1000);
    m_reconnectDelay = qMin(m_reconnectDelay * 2, 60);
}

void IrcClient::sendPing()
{
    if (m_pingPending) return;
    m_pingPending = true;
    m_pingClock.start();
    sendRaw("PING :uplink_rtt");
}

void IrcClient::doReconnect()
{
    emit reconnecting(m_host);
    emit serverMessage(m_host, "Reconnecting…");
    m_saslPending = false;
    if (m_ssl)
        m_socket->connectToHostEncrypted(m_host, m_port);
    else
        m_socket->connectToHost(m_host, m_port);
}

// ---------------------------------------------------------------------------
// Line processing
// ---------------------------------------------------------------------------

void IrcClient::processLine(const QString &line)
{
    IrcMessage msg = IrcParser::parse(line);
    if (!msg.isValid()) return;

    const QString &cmd = msg.command;

    if (cmd == "PING") {
        sendRaw("PONG :" + (msg.params.isEmpty() ? QString{} : msg.params.last()));
        return;
    }

    if (cmd == "PONG") {
        if (m_pingPending && !msg.params.isEmpty() && msg.params.last() == "uplink_rtt") {
            m_pingPending = false;
            emit pingRtt(m_host, static_cast<int>(m_pingClock.elapsed()));
        }
        return;
    }

    if (cmd == "CAP") {
        handleCap(msg.params, msg.trailing);
        return;
    }

    if (cmd == "AUTHENTICATE") {
        if (!msg.params.isEmpty() && msg.params[0] == "+") {
            if (m_saslExternal) {
                sendRaw("AUTHENTICATE +");
            } else {
                const QByteArray payload =
                    QByteArray("\0", 1) + m_saslUser.toUtf8() +
                    QByteArray("\0", 1) + m_saslPassword.toUtf8();
                sendRaw("AUTHENTICATE " + QString::fromLatin1(payload.toBase64()));
            }
        }
        return;
    }

    if (cmd == "BATCH") {
        handleBatch(msg.params);
        return;
    }

    if (cmd == "BOUNCER") {
        handleBouncer(msg.params, msg.trailing);
        return;
    }

    if (cmd == "MARKREAD" && msg.params.size() >= 2) {
        const QString target = msg.params[0];
        const QString tsTag  = msg.params[1]; // "timestamp=<iso>"
        if (tsTag.startsWith("timestamp=")) {
            const QDateTime ts = QDateTime::fromString(tsTag.mid(10), Qt::ISODateWithMs);
            emit readMarkerReceived(m_host, target, ts);
        }
        return;
    }

    bool isNumeric = false;
    cmd.toInt(&isNumeric);
    if (isNumeric) {
        handleNumeric(cmd, msg.params, msg.trailing);
        return;
    }

    // If this message belongs to an active batch, buffer it
    const QString batchRef = msg.tags.value("batch");
    if (!batchRef.isEmpty() && m_batches.contains(batchRef)) {
        auto &batch = m_batches[batchRef];
        if (batch.msgs.size() >= kMaxBatchMessages) {
            emit errorMessage(m_host, "IRC batch too large; dropping extra messages");
            return;
        }
        BatchInfo::Msg bm;
        bm.command    = cmd;
        bm.nick       = msg.nick;
        bm.params     = msg.params;
        bm.trailing   = msg.trailing;
        bm.serverTime = msg.serverTime;
        bm.msgid      = msg.tags.value("msgid");
        bm.replyTo    = msg.tags.value("+draft/reply");
        batch.msgs.append(bm);
        return;
    }

    const QDateTime serverTime = msg.serverTime;

    // PRIVMSG / CTCP
    if (cmd == "PRIVMSG" && msg.params.size() >= 1) {
        const QString target = msg.params[0];
        const QString text   = msg.trailing;

        // ZNC self-message echo
        const bool isSelf = (msg.nick == m_nick) ||
                            msg.tags.contains("znc.in/self-message");

        const QString msgid   = msg.tags.value("msgid");
        const QString replyTo = msg.tags.value("+draft/reply");
        if (text.startsWith('\x01') && text.endsWith('\x01')) {
            const QString ctcp    = text.mid(1, text.size() - 2);
            const QString ctcpCmd = ctcp.section(' ', 0, 0).toUpper();
            if (ctcpCmd == "ACTION") {
                emit actionReceived(m_host, isSelf ? target : target,
                                    msg.nick, ctcp.mid(7), serverTime, false, msgid);
            } else if (ctcpCmd == "VERSION") {
                const QString rkey = msg.nick + ":VERSION";
                const qint64  now  = QDateTime::currentMSecsSinceEpoch();
                if (now - m_ctcpTimestamps.value(rkey, 0) >= 5000) {
                    m_ctcpTimestamps.insert(rkey, now);
                    sendRaw("NOTICE " + msg.nick + " :\x01VERSION UplinkIRC " UPLINKIRC_VERSION "\x01");
                    emit serverMessage(m_host, "CTCP VERSION from " + msg.nick);
                }
            } else if (ctcpCmd == "PING") {
                const QString rkey = msg.nick + ":PING";
                const qint64  now  = QDateTime::currentMSecsSinceEpoch();
                if (now - m_ctcpTimestamps.value(rkey, 0) >= 5000) {
                    m_ctcpTimestamps.insert(rkey, now);
                    QString payload = ctcp.section(' ', 1).left(32);
                    sendRaw("NOTICE " + msg.nick + " :\x01PING " + payload + "\x01");
                }
            } else if (ctcpCmd == "DCC") {
                const QString dccType = ctcp.section(' ', 1, 1).toUpper();
                if (dccType == "SEND") {
                    const QString fn = ctcp.section(' ', 2, 2);
                    bool ok1, ok2, ok3;
                    const quint32 ip       = ctcp.section(' ', 3, 3).toUInt(&ok1);
                    const quint16 port     = ctcp.section(' ', 4, 4).toUShort(&ok2);
                    const qint64  filesize = ctcp.section(' ', 5, 5).toLongLong(&ok3);
                    if (ok1 && ok2 && ok3 && !fn.isEmpty())
                        emit dccSendReceived(m_host, msg.nick, fn, ip, port, filesize);
                    else
                        emit serverMessage(m_host, "DCC SEND from " + msg.nick + " (malformed)");
                } else {
                    emit serverMessage(m_host, "DCC " + dccType + " from " + msg.nick + " (unsupported)");
                }
            } else {
                emit serverMessage(m_host, "CTCP " + ctcpCmd + " from " + msg.nick);
            }
        } else {
            emit messageReceived(m_host, target, msg.nick, text, serverTime, false, msgid, replyTo);
        }
        return;
    }

    if (cmd == "NOTICE" && msg.params.size() >= 1) {
        const QString text = msg.trailing;
        if (text.startsWith('\x01') && text.endsWith('\x01')) {
            const QString ctcp    = text.mid(1, text.size() - 2);
            const QString ctcpCmd = ctcp.section(' ', 0, 0).toUpper();
            if (ctcpCmd == "PING") {
                bool ok = false;
                const qint64 sent = ctcp.section(' ', 1).toLongLong(&ok);
                const qint64 rtt  = QDateTime::currentMSecsSinceEpoch() - sent;
                emit ctcpPingReply(m_host, msg.nick, ok ? rtt : -1);
            } else if (ctcpCmd == "TIME") {
                emit ctcpTimeReply(m_host, msg.nick, ctcp.section(' ', 1));
            } else {
                emit noticeReceived(m_host, msg.params[0], msg.nick,
                                    "CTCP reply: " + ctcp, serverTime, false, {}, {});
            }
        } else {
            emit noticeReceived(m_host, msg.params[0], msg.nick, text, serverTime, false,
                                msg.tags.value("msgid"), msg.tags.value("+draft/reply"));
        }
        return;
    }

    if (cmd == "TAGMSG" && !msg.params.isEmpty()) {
        const QString typing = msg.tags.value("+typing");
        if (!typing.isEmpty())
            emit typingReceived(m_host, msg.params[0], msg.nick, typing);
        return;
    }

    if (cmd == "JOIN") {
        const QString channel = msg.params.isEmpty() ? msg.trailing : msg.params[0];
        if (msg.nick == m_nick)
            emit serverMessage(m_host, "Joined " + channel);
        emit userJoined(m_host, channel, msg.nick);
        return;
    }

    if (cmd == "PART" && !msg.params.isEmpty()) {
        emit userParted(m_host, msg.params[0], msg.nick, msg.trailing);
        return;
    }

    if (cmd == "QUIT") {
        emit userQuit(m_host, msg.nick, msg.trailing);
        return;
    }

    if (cmd == "NICK") {
        const QString newNick = msg.params.isEmpty() ? msg.trailing : msg.params[0];
        if (newNick.toLower() == m_nick.toLower()) {
            m_nick = newNick;
            emit selfNickChanged(m_host, newNick);
        }
        emit nickChanged(m_host, msg.nick, newNick);
        return;
    }

    if (cmd == "KICK" && msg.params.size() >= 2) {
        emit kicked(m_host, msg.params[0], msg.params[1], msg.nick, msg.trailing);
        return;
    }

    if (cmd == "MODE" && !msg.params.isEmpty()) {
        QStringList modeParts = msg.params.mid(1);
        if (!msg.trailing.isEmpty()) modeParts << msg.trailing;
        emit modesReceived(m_host, msg.params[0], modeParts.join(' '));
        return;
    }

    if (cmd == "TOPIC" && !msg.params.isEmpty()) {
        emit topicReceived(m_host, msg.params[0], msg.trailing);
        return;
    }
}

// ---------------------------------------------------------------------------
// BATCH
// ---------------------------------------------------------------------------

void IrcClient::handleBatch(const QStringList &params)
{
    if (params.isEmpty()) return;
    const QString refArg = params[0];

    if (refArg.startsWith('+')) {
        if (m_batches.size() >= kMaxOpenBatches) {
            emit errorMessage(m_host, "Too many open IRC batches; ignoring");
            return;
        }
        const QString ref   = refArg.mid(1);
        const QString type  = params.size() > 1 ? params[1] : QString();
        const QString param = params.size() > 2 ? params[2] : QString();
        BatchInfo bi;
        bi.type  = type;
        bi.param = param;
        m_batches.insert(ref, bi);
    } else if (refArg.startsWith('-')) {
        deliverBatch(refArg.mid(1));
    }
}

void IrcClient::deliverBatch(const QString &ref)
{
    if (!m_batches.contains(ref)) return;
    const BatchInfo batch = m_batches.take(ref);

    const bool isHistory = (batch.type == "chathistory" ||
                            batch.type == "znc.in/batch/playback");

    for (const BatchInfo::Msg &bm : batch.msgs) {
        if (bm.command == "PRIVMSG" && bm.params.size() >= 1) {
            const QString target = bm.params[0];
            const QString text   = bm.trailing;
            if (text.startsWith('\x01') && text.endsWith('\x01')) {
                const QString ctcp = text.mid(1, text.size() - 2);
                if (ctcp.startsWith("ACTION "))
                    emit actionReceived(m_host, target, bm.nick,
                                        ctcp.mid(7), bm.serverTime, isHistory, bm.msgid);
            } else {
                emit messageReceived(m_host, target, bm.nick,
                                     text, bm.serverTime, isHistory, bm.msgid, bm.replyTo);
            }
        } else if (bm.command == "NOTICE" && bm.params.size() >= 1) {
            emit noticeReceived(m_host, bm.params[0], bm.nick,
                                bm.trailing, bm.serverTime, isHistory, bm.msgid, bm.replyTo);
        }
    }
}

// ---------------------------------------------------------------------------
// BOUNCER (soju)
// ---------------------------------------------------------------------------

void IrcClient::handleBouncer(const QStringList &params, const QString &trailing)
{
    Q_UNUSED(trailing)
    if (params.isEmpty()) return;
    const QString sub = params[0].toUpper();

    if (sub == "NETWORK" && params.size() >= 2) {
        // Parse key=value pairs from params[1]
        QHash<QString,QString> attrs;
        for (const QString &part : params[1].split(';', Qt::SkipEmptyParts)) {
            const int eq = part.indexOf('=');
            if (eq != -1) attrs.insert(part.left(eq), part.mid(eq+1));
        }
        const QString id    = attrs.value("id");
        const QString name  = attrs.value("name");
        const bool connected = (attrs.value("state") == "connected");
        emit bouncerNetworkReceived(m_host, id, name, connected);
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
        // Buffer multi-line responses (params[2] == "*" means more lines coming)
        const bool more = (params.size() >= 3 && params[2] == "*");
        m_capLsBuffer += trailing.split(' ', Qt::SkipEmptyParts);
        if (more) return;

        const QStringList available = m_capLsBuffer;
        m_capLsBuffer.clear();

        // Servers may advertise caps as "name=value" (e.g. "sasl=PLAIN,EXTERNAL")
        auto hasAvailable = [&](const QString &cap) {
            for (const QString &a : available)
                if (a == cap || a.startsWith(cap + "="))
                    return true;
            return false;
        };

        QStringList desired = {
            "multi-prefix", "away-notify", "server-time",
            "message-tags", "batch", "labeled-response", "draft/typing",
            "chathistory", "echo-message",
        };

        // ZNC-specific caps
        if (m_bouncerType == BouncerType::ZNC) {
            desired << "znc.in/playback"
                    << "znc.in/self-message"
                    << "znc.in/batch";
        }

        // Soju-specific caps
        if (m_bouncerType == BouncerType::Soju) {
            desired << "soju.im/bouncer-networks"
                    << "soju.im/bouncer-networks-notify"
                    << "soju.im/read"
                    << "soju.im/no-implicit-names";
        }

        if ((!m_saslUser.isEmpty() && !m_saslPassword.isEmpty()) || m_saslExternal)
            desired << "sasl";

        QStringList want;
        for (const QString &cap : desired)
            if (hasAvailable(cap))
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
        const QStringList acked = trailing.split(' ', Qt::SkipEmptyParts);
        for (const QString &cap : acked)
            m_ackedCaps.insert(cap.startsWith('-') ? cap.mid(1) : cap);

        if (acked.contains("sasl") &&
            ((!m_saslUser.isEmpty() && !m_saslPassword.isEmpty()) || m_saslExternal)) {
            m_saslPending = true;
            sendRaw(m_saslExternal ? "AUTHENTICATE EXTERNAL" : "AUTHENTICATE PLAIN");
        } else {
            sendRaw("CAP END");
        }

        // Request soju network list once caps are acked
        if (m_bouncerType == BouncerType::Soju &&
            m_ackedCaps.contains("soju.im/bouncer-networks")) {
            sendRaw("BOUNCER LISTNETWORKS");
        }
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
        if (!m_nickservPassword.isEmpty()) {
            sendRaw("PRIVMSG NickServ :IDENTIFY " + m_nickservPassword);
            emit serverMessage(m_host, "Sent NickServ IDENTIFY");
        }
        // ZNC: request playback of all buffers since last seen
        if (m_bouncerType == BouncerType::ZNC &&
            m_ackedCaps.contains("znc.in/playback")) {
            sendRaw("PRIVMSG *playback :PLAY * 0");
        }
        break;

    case 2:   // RPL_YOURHOST
    case 3:   // RPL_CREATED
    case 4:   // RPL_MYINFO
    case 5:   // RPL_ISUPPORT
    case 251: case 252: case 253: case 254: case 255:
    case 265: case 266:
    case 372: // RPL_MOTD
    case 375: // RPL_MOTDSTART
        emit serverMessage(m_host, trailing);
        break;

    case 376: // RPL_ENDOFMOTD
    case 422: // ERR_NOMOTD
        emit serverMessage(m_host, trailing);
        break;

    case 324: // RPL_CHANNELMODEIS
        if (params.size() >= 3) {
            QStringList modeParts = params.mid(2);
            emit modesReceived(m_host, params[1], modeParts.join(' '));
        }
        break;

    case 332: // RPL_TOPIC
        if (params.size() >= 2)
            emit topicReceived(m_host, params[1], trailing);
        break;

    case 333: break; // RPL_TOPICWHOTIME

    case 353: { // RPL_NAMREPLY
        if (params.size() >= 3) {
            const QString channel = params[2];
            const QStringList nicks = trailing.split(' ', Qt::SkipEmptyParts);
            m_namesBuffer[channel] += nicks;
        }
        break;
    }

    case 352: { // RPL_WHOREPLY — <client> <channel> <user> <host> <server> <nick> <flags> :<hop> <real>
        if (params.size() >= 7) {
            const QString channel = params[1];
            const QString nick    = params[5];
            const QString flags   = params[6];
            if (channel.startsWith('#') || channel.startsWith('&'))
                emit whoEntryReceived(m_host, channel, nick, flags);
        }
        break;
    }

    case 366: { // RPL_ENDOFNAMES
        if (params.size() >= 2) {
            const QString channel = params[1];
            emit namesReceived(m_host, channel, m_namesBuffer.take(channel));
            emit namesDone(m_host, channel);
            sendRaw("MODE " + channel);
            sendRaw("WHO " + channel);
            // Request chat history on join
            if (m_ackedCaps.contains("chathistory"))
                requestHistory(channel, 100);
        }
        break;
    }

    case 900: // RPL_LOGGEDIN
        emit serverMessage(m_host, trailing);
        break;

    case 903: // RPL_SASLSUCCESS
        emit serverMessage(m_host, "SASL authentication successful");
        m_saslPending = false;
        sendRaw("CAP END");
        break;

    case 904: case 905: case 906:
        emit serverMessage(m_host, "SASL authentication failed: " + trailing);
        m_saslPending = false;
        sendRaw("CAP END");
        break;

    case 433: // ERR_NICKNAMEINUSE
        setNick(m_nick + "_");
        break;

    case 431: case 432:
        emit errorMessage(m_host, "Nick error: " + trailing);
        break;

    default:
        if (!trailing.isEmpty()) {
            if (n >= 400)
                emit errorMessage(m_host, trailing);
            else
                emit serverMessage(m_host, trailing);
        }
        break;
    }
}
