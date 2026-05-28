#pragma once

#include <QObject>
#include <QSslSocket>
#include <QHash>
#include <QSet>

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

    QString currentNick() const { return m_nick; }
    QString host()        const { return m_host; }
    bool    isConnected() const;

signals:
    void connected(const QString &server);
    void disconnected(const QString &server);
    void socketError(const QString &server, const QString &error);

    // chat
    void messageReceived(const QString &server, const QString &target,
                         const QString &nick,   const QString &text);
    void noticeReceived (const QString &server, const QString &target,
                         const QString &nick,   const QString &text);
    void actionReceived (const QString &server, const QString &target,
                         const QString &nick,   const QString &text);

    // channel state
    void userJoined   (const QString &server, const QString &channel, const QString &nick);
    void userParted   (const QString &server, const QString &channel, const QString &nick, const QString &reason);
    void userQuit     (const QString &server, const QString &nick,    const QString &reason);
    void nickChanged  (const QString &server, const QString &oldNick, const QString &newNick);
    void kicked       (const QString &server, const QString &channel, const QString &nick,
                       const QString &by,     const QString &reason);

    // channel info
    void topicReceived    (const QString &server, const QString &channel, const QString &topic);
    void modesReceived    (const QString &server, const QString &channel, const QString &modes);
    void namesReceived    (const QString &server, const QString &channel, const QStringList &nicks);
    void namesDone        (const QString &server, const QString &channel);

    // server / misc
    void serverMessage(const QString &server, const QString &text);
    void rawReceived  (const QString &line);
    void selfNickChanged(const QString &server, const QString &newNick);
    void typingReceived(const QString &server, const QString &channel,
                        const QString &nick,   const QString &state);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSslErrors(const QList<QSslError> &errors);
    void onErrorOccurred(QAbstractSocket::SocketError error);

private:
    void processLine(const QString &line);
    void handleCap   (const QStringList &params, const QString &trailing);
    void handleNumeric(const QString &cmd, const QStringList &params, const QString &trailing);

    QSslSocket  *m_socket;
    QString      m_host;
    quint16      m_port{6697};
    bool         m_ssl{true};
    QString      m_nick;
    QString      m_user;
    QString      m_realname;
    QString      m_password;
    QString      m_buffer;

    QSet<QString>              m_requestedCaps;
    QHash<QString, QStringList> m_namesBuffer; // channel -> partial nick list
};
