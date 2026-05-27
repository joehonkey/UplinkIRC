#pragma once

#include "channel.h"
#include <QString>
#include <QHash>

struct ServerSession {
    QString  name;        // display name from config
    QString  host;
    QString  nick;
    bool     connected{false};

    // key = channel name lowercased
    QHash<QString, Channel> channels;

    Channel &getOrCreate(const QString &name)
    {
        const QString key = name.toLower();
        if (!channels.contains(key)) {
            Channel ch;
            ch.name = name;
            channels.insert(key, ch);
        }
        return channels[key];
    }

    Channel *get(const QString &name)
    {
        return channels.contains(name.toLower()) ? &channels[name.toLower()] : nullptr;
    }

    // Server-level messages go into a pseudo-channel called "(server)"
    Channel &serverBuffer() { return getOrCreate("(server)"); }
};
