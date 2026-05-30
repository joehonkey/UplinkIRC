#pragma once

#include "config/config.h"
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QSslSocket>
#include <QElapsedTimer>
#include <QTimer>

struct ServerConfig;

class IrcClient : public QObject
{
    Q_OBJECT

public:
    explicit IrcClient(QObject *parent = nullptr);
    ~IrcClient() override;

    void connectToServer(const ServerConfig &cfg);
    void quit(const QString &reason = "UplinkIRC");

    void join(const QString &channel, const QString &key = {});
    void part(const QString &channel, const QString &reason = {});
    void privmsg(const QString &target, const QString &text);
    void notice(const QString &target, const QString &text);
    void setNick(const QString &nick);
    void sendTyping(const QString &channel, const QString &state);
    void sendRaw(const QString &line);

    // Bouncer helpers
    void requestHistory(const QString &target, int limit = 100);
    void markRead(const QString &target, const QDateTime &ts);

    QString currentNick() const { return m_nick; }
    QString host()        const { return m_host; }
    bool    isConnected() const;
    bool    hasCap(const QString &cap) const { return m_ackedCaps.contains(cap); }
    quint32 localIpv4()  const;

signals:
    void connected(const QString &server);
    void disconnected(const QString &server);
    void socketError(const QString &server, const QString &error);

    void messageReceived(const QString &server, const QString &target,
                         const QString &nick,   const QString &text,
                         const QDateTime &serverTime, bool isHistory);
    void noticeReceived (const QString &server, const QString &target,
                         const QString &nick,   const QString &text,
                         const QDateTime &serverTime, bool isHistory);
    void actionReceived (const QString &server, const QString &target,
                         const QString &nick,   const QString &text,
                         const QDateTime &serverTime, bool isHistory);

    void userJoined   (const QString &server, const QString &channel, const QString &nick);
    void userParted   (const QString &server, const QString &channel, const QString &nick, const QString &reason);
    void userQuit     (const QString &server, const QString &nick,    const QString &reason);
    void nickChanged  (const QString &server, const QString &oldNick, const QString &newNick);
    void kicked       (const QString &server, const QString &channel, const QString &nick,
                       const QString &by,     const QString &reason);

    void topicReceived    (const QString &server, const QString &channel, const QString &topic);
    void modesReceived    (const QString &server, const QString &channel, const QString &modes);
    void namesReceived    (const QString &server, const QString &channel, const QStringList &nicks);
    void namesDone        (const QString &server, const QString &channel);
    void whoEntryReceived (const QString &server, const QString &channel,
                           const QString &nick,   const QString &flags);

    void serverMessage  (const QString &server, const QString &text);
    void errorMessage   (const QString &server, const QString &text);
    void pingRtt        (const QString &host, int ms);
    void reconnecting   (const QString &host);
    void ctcpPingReply  (const QString &server, const QString &nick, qint64 rttMs);
    void ctcpTimeReply  (const QString &server, const QString &nick, const QString &timeStr);
    void rawReceived  (const QString &line);
    void selfNickChanged(const QString &server, const QString &newNick);
    void dccSendReceived(const QString &server, const QString &fromNick,
                         const QString &filename, quint32 ip, quint16 port, qint64 filesize);
    void typingReceived(const QString &server, const QString &channel,
                        const QString &nick,   const QString &state);

    // Bouncer-specific
    void bouncerNetworkReceived(const QString &server, const QString &netId,
                                const QString &netName, bool isConnected);
    void readMarkerReceived(const QString &server, const QString &target,
                            const QDateTime &ts);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSslErrors(const QList<QSslError> &errors);
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void doReconnect();
    void sendPing();

private:
    void processLine(const QString &line);
    void handleCap     (const QStringList &params, const QString &trailing);
    void handleNumeric (const QString &cmd, const QStringList &params, const QString &trailing);
    void handleBatch   (const QStringList &params);
    void handleBouncer (const QStringList &params, const QString &trailing);
    void deliverBatch  (const QString &ref);
    void scheduleReconnect();

    QSslSocket  *m_socket;
    QString      m_host;
    quint16      m_port{6697};
    bool         m_ssl{true};
    QString      m_nick;
    QString      m_user;
    QString      m_realname;
    QString      m_password;
    QString      m_saslUser;
    QString      m_saslPassword;
    bool         m_saslExternal{false};
    bool         m_saslPending{false};
    QString      m_nickservPassword;
    QString      m_buffer;

    BouncerType  m_bouncerType{BouncerType::None};
    QString      m_bouncerNetwork;

    QTimer      *m_reconnectTimer{nullptr};
    QTimer      *m_pingTimer{nullptr};
    QElapsedTimer m_pingClock;
    bool         m_pingPending{false};
    bool         m_intentionalDisconnect{false};
    int          m_reconnectDelay{5};

    QSet<QString>               m_requestedCaps;
    QSet<QString>               m_ackedCaps;
    QStringList                 m_capLsBuffer;
    QHash<QString, QStringList> m_namesBuffer;

    // Batch tracking
    struct BatchInfo {
        QString type;
        QString param; // e.g., channel for chathistory
        struct Msg {
            QString     command;
            QString     nick;
            QStringList params;
            QString     trailing;
            QDateTime   serverTime;
        };
        QList<Msg> msgs;
    };
    QHash<QString, BatchInfo> m_batches;

    // CTCP reply rate-limit: key = "nick:cmd", value = last-reply ms timestamp
    QHash<QString, qint64> m_ctcpTimestamps;
};
