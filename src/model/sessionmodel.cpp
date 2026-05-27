#include "sessionmodel.h"
#include "irc/ircclient.h"

SessionModel::SessionModel(QObject *parent)
    : QObject(parent)
{}

void SessionModel::loadConfig(const Config &cfg)
{
    m_config = cfg;
    for (const ServerConfig &sc : cfg.servers) {
        ServerSession sess;
        sess.name = sc.name;
        sess.host = sc.host;
        sess.nick = sc.nick;
        m_sessions.append(sess);
        emit serverAdded(sc.host);

        auto *client = new IrcClient(this);
        attachClient(client, sc);
        m_clients.append(client);
        client->connectToServer(sc);
    }
}

ServerSession *SessionModel::session(const QString &host)
{
    for (auto &s : m_sessions)
        if (s.host == host) return &s;
    return nullptr;
}

Channel *SessionModel::channel(const QString &host, const QString &name)
{
    auto *s = session(host);
    return s ? s->get(name) : nullptr;
}

IrcClient *SessionModel::clientFor(const QString &host)
{
    for (IrcClient *cl : m_clients)
        if (cl->host() == host) return cl;
    return nullptr;
}

void SessionModel::sendMessage(const QString &host, const QString &target, const QString &text)
{
    auto *cl = clientFor(host);
    if (!cl) return;
    cl->privmsg(target, text);
    // Echo own message into the model
    auto *sess = session(host);
    if (sess)
        postMessage(host, target, Message::make(MessageType::Privmsg, sess->nick, text));
}

void SessionModel::sendRaw(const QString &host, const QString &line)
{
    auto *cl = clientFor(host);
    if (cl) cl->sendRaw(line);
}

void SessionModel::sendJoin(const QString &host, const QString &channel, const QString &key)
{
    auto *cl = clientFor(host);
    if (cl) cl->join(channel, key);
}

void SessionModel::sendPart(const QString &host, const QString &channel, const QString &reason)
{
    auto *cl = clientFor(host);
    if (cl) cl->part(channel, reason);
}

void SessionModel::sendNick(const QString &host, const QString &nick)
{
    auto *cl = clientFor(host);
    if (cl) cl->setNick(nick);
}

void SessionModel::sendAction(const QString &host, const QString &target, const QString &text)
{
    auto *cl = clientFor(host);
    if (!cl) return;
    cl->privmsg(target, "\x01""ACTION " + text + "\x01");
    if (auto *sess = session(host))
        postMessage(host, target, Message::make(MessageType::Action, sess->nick, text));
}

void SessionModel::setActive(const QString &host, const QString &ch)
{
    m_activeHost    = host;
    m_activeChannel = ch;
    // Clear unread on the newly active channel
    if (auto *c = channel(host, ch)) {
        c->unread = 0;
        emit unreadChanged(host, ch, 0);
    }
}

// ---------------------------------------------------------------------------
// Wire up a client to all our handler slots
// ---------------------------------------------------------------------------

void SessionModel::attachClient(IrcClient *cl, const ServerConfig &cfg)
{
    connect(cl, &IrcClient::connected,       this, &SessionModel::onConnected);
    connect(cl, &IrcClient::disconnected,    this, &SessionModel::onDisconnected);
    connect(cl, &IrcClient::socketError,     this, &SessionModel::onSocketError);
    connect(cl, &IrcClient::messageReceived, this, &SessionModel::onMessage);
    connect(cl, &IrcClient::noticeReceived,  this, &SessionModel::onNotice);
    connect(cl, &IrcClient::actionReceived,  this, &SessionModel::onAction);
    connect(cl, &IrcClient::userJoined,      this, &SessionModel::onUserJoined);
    connect(cl, &IrcClient::userParted,      this, &SessionModel::onUserParted);
    connect(cl, &IrcClient::userQuit,        this, &SessionModel::onUserQuit);
    connect(cl, &IrcClient::nickChanged,     this, &SessionModel::onNickChanged);
    connect(cl, &IrcClient::kicked,          this, &SessionModel::onKicked);
    connect(cl, &IrcClient::topicReceived,   this, &SessionModel::onTopicReceived);
    connect(cl, &IrcClient::modesReceived,   this, &SessionModel::onModesReceived);
    connect(cl, &IrcClient::namesReceived,   this, &SessionModel::onNamesReceived);
    connect(cl, &IrcClient::serverMessage,   this, &SessionModel::onServerMessage);
    connect(cl, &IrcClient::selfNickChanged, this, &SessionModel::onSelfNickChanged);

    // Pre-create server buffer and configured channels in the session
    auto *sess = session(cfg.host);
    if (!sess) return;
    sess->serverBuffer(); // ensure "(server)" exists
    for (const auto &ch : cfg.channels) {
        auto &c  = sess->getOrCreate(ch.name);
        c.name   = ch.name;
        emit channelAdded(cfg.host, ch.name);
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void SessionModel::postMessage(const QString &host, const QString &target, const Message &msg)
{
    auto *sess = session(host);
    if (!sess) return;

    auto &ch = sess->getOrCreate(target);
    if (ch.name.isEmpty()) ch.name = target;
    ch.addMessage(msg);

    const bool isActive = (host == m_activeHost && target.toLower() == m_activeChannel.toLower());
    if (!isActive && msg.type == MessageType::Privmsg)
        ++ch.unread;

    emit messageAdded(host, target, msg);
    if (!isActive && msg.type == MessageType::Privmsg)
        emit unreadChanged(host, target, ch.unread);
}

// ---------------------------------------------------------------------------
// Handlers
// ---------------------------------------------------------------------------

void SessionModel::onConnected(const QString &host)
{
    auto *sess = session(host);
    if (!sess) return;
    sess->connected = true;
    emit serverConnected(host);

    // Auto-join configured channels
    for (const ServerConfig &sc : m_config.servers) {
        if (sc.host != host) continue;
        for (const ChannelConfig &cc : sc.channels) {
            for (IrcClient *cl : m_clients) {
                if (cl->host() == host) {
                    cl->join(cc.name, cc.password);
                    break;
                }
            }
        }
    }
}

void SessionModel::onDisconnected(const QString &host)
{
    auto *sess = session(host);
    if (!sess) return;
    sess->connected = false;
    // Clear all nick lists
    for (auto &ch : sess->channels)
        ch.nicks.clear();
    emit serverDisconnected(host);
    postMessage(host, "(server)", Message::make(MessageType::Server, "", "Disconnected."));
}

void SessionModel::onSocketError(const QString &host, const QString &error)
{
    postMessage(host, "(server)", Message::make(MessageType::Error, "", "Error: " + error));
}

void SessionModel::onMessage(const QString &host, const QString &target,
                             const QString &nick, const QString &text)
{
    postMessage(host, target, Message::make(MessageType::Privmsg, nick, text));
}

void SessionModel::onNotice(const QString &host, const QString &target,
                            const QString &nick, const QString &text)
{
    const QString dest = target.startsWith('#') ? target : "(server)";
    postMessage(host, dest, Message::make(MessageType::Notice, nick, text));
}

void SessionModel::onAction(const QString &host, const QString &target,
                            const QString &nick, const QString &text)
{
    postMessage(host, target, Message::make(MessageType::Action, nick, text));
}

void SessionModel::onUserJoined(const QString &host, const QString &channel, const QString &nick)
{
    auto *sess = session(host);
    if (!sess) return;
    auto &ch = sess->getOrCreate(channel);
    if (ch.name.isEmpty()) ch.name = channel;

    const bool isSelf = sess->nick.toLower() == nick.toLower();
    if (isSelf) {
        ch.joined = true;
        emit channelAdded(host, channel);
    }
    ch.addNick(nick);
    emit nickListChanged(host, channel);
    postMessage(host, channel, Message::make(MessageType::Join, nick,
        isSelf ? "You joined " + channel : nick + " joined"));
}

void SessionModel::onUserParted(const QString &host, const QString &channel,
                                const QString &nick, const QString &reason)
{
    auto *sess = session(host);
    if (!sess) return;
    auto *ch = sess->get(channel);

    const bool isSelf = sess->nick.toLower() == nick.toLower();
    if (isSelf && ch) {
        ch->joined = false;
        ch->nicks.clear();
        emit nickListChanged(host, channel);
    } else if (ch) {
        ch->removeNick(nick);
        emit nickListChanged(host, channel);
    }
    postMessage(host, channel, Message::make(MessageType::Part, nick,
        reason.isEmpty() ? nick + " left" : nick + " left (" + reason + ")"));
}

void SessionModel::onUserQuit(const QString &host, const QString &nick, const QString &reason)
{
    auto *sess = session(host);
    if (!sess) return;
    for (auto &ch : sess->channels) {
        bool found = false;
        for (const auto &e : std::as_const(ch.nicks))
            if (e.nick.toLower() == nick.toLower()) { found = true; break; }
        if (!found) continue;
        ch.removeNick(nick);
        emit nickListChanged(host, ch.name);
        postMessage(host, ch.name, Message::make(MessageType::Quit, nick,
            reason.isEmpty() ? nick + " quit" : nick + " quit (" + reason + ")"));
    }
}

void SessionModel::onNickChanged(const QString &host, const QString &oldNick, const QString &newNick)
{
    auto *sess = session(host);
    if (!sess) return;
    for (auto &ch : sess->channels) {
        bool found = false;
        for (const auto &e : std::as_const(ch.nicks))
            if (e.nick.toLower() == oldNick.toLower()) { found = true; break; }
        if (!found) continue;
        ch.renameNick(oldNick, newNick);
        emit nickListChanged(host, ch.name);
        postMessage(host, ch.name, Message::make(MessageType::Nick, oldNick,
            oldNick + " is now known as " + newNick));
    }
}

void SessionModel::onKicked(const QString &host, const QString &channel,
                            const QString &nick, const QString &by, const QString &reason)
{
    auto *sess = session(host);
    if (!sess) return;
    auto *ch = sess->get(channel);
    if (ch) {
        ch->removeNick(nick);
        emit nickListChanged(host, channel);
    }
    postMessage(host, channel, Message::make(MessageType::Kick, by,
        nick + " was kicked by " + by + " (" + reason + ")"));
}

void SessionModel::onTopicReceived(const QString &host, const QString &channel, const QString &topic)
{
    auto *sess = session(host);
    if (!sess) return;
    auto &ch = sess->getOrCreate(channel);
    ch.topic = topic;
    emit topicChanged(host, channel, topic);
}

void SessionModel::onModesReceived(const QString &host, const QString &channel, const QString &modes)
{
    auto *sess = session(host);
    if (!sess) return;
    auto *ch = sess->get(channel);
    if (ch) ch->modes = modes;
    postMessage(host, channel, Message::make(MessageType::Server, "", "Mode " + channel + " " + modes));
}

void SessionModel::onNamesReceived(const QString &host, const QString &channel, const QStringList &nicks)
{
    auto *sess = session(host);
    if (!sess) return;
    auto &ch = sess->getOrCreate(channel);
    ch.setNicks(nicks);
    emit nickListChanged(host, channel);
}

void SessionModel::onServerMessage(const QString &host, const QString &text)
{
    postMessage(host, "(server)", Message::make(MessageType::Server, "", text));
}

void SessionModel::onSelfNickChanged(const QString &host, const QString &nick)
{
    auto *sess = session(host);
    if (sess) sess->nick = nick;
    emit selfNickChanged(host, nick);
}
