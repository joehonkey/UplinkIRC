#pragma once

#include "message.h"
#include <QDateTime>
#include <QString>
#include <QList>
#include <QSet>

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
    QString     nick;
    QChar       prefix{' '};
    QSet<QChar> prefixes;

    void recomputePrefix()
    {
        prefix = ' ';
        for (QChar p : std::as_const(prefixes))
            if (prefixRank(p) > prefixRank(prefix))
                prefix = p;
    }

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
    QSet<QString>    botNicks;  // lowercased nicks with +B channel user mode
    QHash<QString, QString> previews;  // url → persisted card HTML
    QSet<QString>           hiddenPreviews; // urls the user manually hid
    int              unread{0};
    int              mentions{0};
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
            int i = 0;
            while (i < n.size() && prefixRank(n[i]) > 0) {
                e.prefixes.insert(n[i]);
                ++i;
            }
            e.recomputePrefix();
            e.nick = n.mid(i);
            nicks.append(e);
        }
        std::sort(nicks.begin(), nicks.end());
    }

    void addNick(const QString &raw)
    {
        NickEntry e;
        int i = 0;
        while (i < raw.size() && prefixRank(raw[i]) > 0) {
            e.prefixes.insert(raw[i]);
            ++i;
        }
        e.recomputePrefix();
        e.nick = raw.mid(i);
        // avoid duplicates
        for (const auto &n : std::as_const(nicks))
            if (QString::compare(n.nick, e.nick, Qt::CaseInsensitive) == 0) return;
        nicks.append(e);
        std::sort(nicks.begin(), nicks.end());
    }

    void removeNick(const QString &nick)
    {
        nicks.removeIf([&](const NickEntry &e){
            return QString::compare(e.nick, nick, Qt::CaseInsensitive) == 0;
        });
    }

    void renameNick(const QString &oldNick, const QString &newNick)
    {
        for (auto &e : nicks)
            if (QString::compare(e.nick, oldNick, Qt::CaseInsensitive) == 0) {
                e.nick = newNick; break;
            }
        std::sort(nicks.begin(), nicks.end());
    }

    static constexpr int kPreviewCap = 100;

    void addPreview(const QString &url, const QString &html)
    {
        if (previews.size() >= kPreviewCap)
            previews.erase(previews.begin());
        previews.insert(url, html);
    }
};
