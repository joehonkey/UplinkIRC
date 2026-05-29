#pragma once

#include "message.h"
#include <QDateTime>
#include <QString>
#include <QList>

static constexpr int kMessageBufferCap = 2000;

// Nick prefix rank: ~ & @ % + (owner → admin → op → halfop → voice)
inline int prefixRank(QChar p)
{
    switch (p.unicode()) {
    case '~': return 5;
    case '&': return 4;
    case '@': return 3;
    case '%': return 2;
    case '+': return 1;
    default:  return 0;
    }
}

struct NickEntry {
    QString nick;
    QChar   prefix{' '};

    QString display() const
    {
        return (prefix != ' ' && prefix != '\0') ? QString(prefix) + nick : nick;
    }

    bool operator<(const NickEntry &o) const
    {
        int ra = prefixRank(prefix), rb = prefixRank(o.prefix);
        if (ra != rb) return ra > rb;
        return nick.toLower() < o.nick.toLower();
    }
};

struct Channel {
    QString          name;
    QString          topic;
    QString          modes;
    QList<NickEntry> nicks;
    QList<Message>   messages;
    int              unread{0};
    bool             joined{false};
    QDateTime        lastRead;  // soju.im/read marker

    void addMessage(const Message &msg)
    {
        messages.append(msg);
        if (messages.size() > kMessageBufferCap)
            messages.removeFirst();
    }

    void setNicks(const QStringList &raw)
    {
        nicks.clear();
        for (const QString &n : raw) {
            if (n.isEmpty()) continue;
            NickEntry e;
            QChar first = n[0];
            if (prefixRank(first) > 0) {
                e.prefix = first;
                e.nick   = n.mid(1);
            } else {
                e.nick = n;
            }
            nicks.append(e);
        }
        std::sort(nicks.begin(), nicks.end());
    }

    void addNick(const QString &raw)
    {
        NickEntry e;
        if (!raw.isEmpty() && prefixRank(raw[0]) > 0) {
            e.prefix = raw[0];
            e.nick   = raw.mid(1);
        } else {
            e.nick = raw;
        }
        // avoid duplicates
        for (const auto &n : std::as_const(nicks))
            if (n.nick.toLower() == e.nick.toLower()) return;
        nicks.append(e);
        std::sort(nicks.begin(), nicks.end());
    }

    void removeNick(const QString &nick)
    {
        nicks.removeIf([&](const NickEntry &e){
            return e.nick.toLower() == nick.toLower();
        });
    }

    void renameNick(const QString &oldNick, const QString &newNick)
    {
        for (auto &e : nicks)
            if (e.nick.toLower() == oldNick.toLower()) { e.nick = newNick; break; }
        std::sort(nicks.begin(), nicks.end());
    }
};
