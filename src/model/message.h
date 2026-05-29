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
    bool        isHistory{false};

    static Message make(MessageType t, const QString &nick, const QString &text,
                        const QDateTime &ts = {}, bool history = false)
    {
        return { ts.isValid() ? ts : QDateTime::currentDateTime(), nick, text, t, history };
    }
};
