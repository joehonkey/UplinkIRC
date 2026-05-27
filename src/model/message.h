#pragma once

#include <QString>
#include <QDateTime>

enum class MessageType {
    Privmsg,
    Notice,
    Action,
    Server,    // server/status text
    Join,
    Part,
    Quit,
    Kick,
    Nick,
    Topic,
    Error,
};

struct Message {
    QDateTime   timestamp;
    QString     nick;
    QString     text;
    MessageType type{MessageType::Privmsg};

    static Message make(MessageType t, const QString &nick, const QString &text)
    {
        return { QDateTime::currentDateTime(), nick, text, t };
    }
};
