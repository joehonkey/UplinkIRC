#pragma once

#include "serversession.h"
#include "config/config.h"
#include <QDateTime>
#include <QObject>
#include <QList>

class IrcClient;

class SessionModel : public QObject
{
    Q_OBJECT

public:
    explicit SessionModel(QObject *parent = nullptr);

    // Create a client for each server in config and start connecting
    void loadConfig(const Config &cfg);
    void addServer(const ServerConfig &sc);
    void removeServer(const QString &host);
    void updateServer(const QString &oldHost, const ServerConfig &sc);

    // Read access for UI
    const QList<ServerSession> &sessions() const { return m_sessions; }
    ServerSession *session(const QString &host);
    Channel       *channel(const QString &host, const QString &name);

    // Active selection — UI drives this
    void setActive(const QString &host, const QString &channel);
    QString activeHost()    const { return m_activeHost; }
    QString activeChannel() const { return m_activeChannel; }

    // Send on behalf of a session
    void sendMessage(const QString &host, const QString &target, const QString &text);
    void sendRaw    (const QString &host, const QString &line);
    void sendJoin   (const QString &host, const QString &channel, const QString &key = {});
    void sendPart   (const QString &host, const QString &channel, const QString &reason = {});
    void sendNick   (const QString &host, const QString &nick);
    void sendAction (const QString &host, const QString &target, const QString &text);
    void sendTyping (const QString &host, const QString &channel, const QString &state);
    void openPM    (const QString &host, const QString &nick);
    IrcClient *clientFor(const QString &host);

signals:
    // Structural changes — sidebar needs a repaint
    void serverAdded    (const QString &host);
    void serverConnected(const QString &host);
    void serverDisconnected(const QString &host);
    void channelAdded   (const QString &host, const QString &channel);
    void channelRemoved (const QString &host, const QString &channel);

    // Content changes — chat view needs updating
    void messageAdded(const QString &host, const QString &channel, const Message &msg);
    void topicChanged  (const QString &host, const QString &channel, const QString &topic);
    void modesChanged  (const QString &host, const QString &channel);
    void nickListChanged(const QString &host, const QString &channel);
    void unreadChanged(const QString &host, const QString &channel, int count);

    // Self
    void selfNickChanged(const QString &host, const QString &nick);

    // Typing
    void typingReceived(const QString &host, const QString &channel,
                        const QString &nick, const QString &state);

private:
    void attachClient(IrcClient *client, const ServerConfig &cfg);

    // IrcClient signal handlers
    void onConnected      (const QString &host);
    void onDisconnected   (const QString &host);
    void onSocketError    (const QString &host, const QString &error);
    void onMessage        (const QString &host, const QString &target,
                           const QString &nick, const QString &text,
                           const QDateTime &serverTime, bool isHistory);
    void onNotice         (const QString &host, const QString &target,
                           const QString &nick, const QString &text,
                           const QDateTime &serverTime, bool isHistory);
    void onAction         (const QString &host, const QString &target,
                           const QString &nick, const QString &text,
                           const QDateTime &serverTime, bool isHistory);
    void onUserJoined     (const QString &host, const QString &channel, const QString &nick);
    void onUserParted     (const QString &host, const QString &channel,
                           const QString &nick, const QString &reason);
    void onUserQuit       (const QString &host, const QString &nick, const QString &reason);
    void onNickChanged    (const QString &host, const QString &oldNick, const QString &newNick);
    void onKicked         (const QString &host, const QString &channel, const QString &nick,
                           const QString &by,   const QString &reason);
    void onTopicReceived  (const QString &host, const QString &channel, const QString &topic);
    void onModesReceived  (const QString &host, const QString &channel, const QString &modes);
    void onNamesReceived  (const QString &host, const QString &channel, const QStringList &nicks);
    void onServerMessage  (const QString &host, const QString &text);
    void onSelfNickChanged(const QString &host, const QString &nick);

    void postMessage(const QString &host, const QString &target, const Message &msg);

    QList<ServerSession> m_sessions;
    QList<IrcClient *>   m_clients;
    Config               m_config;

    QString m_activeHost;
    QString m_activeChannel;
};
