#pragma once

#include <QString>
#include <QStringList>
#include <QList>

struct ChannelConfig {
    QString name;
    QString password;
};

struct ServerConfig {
    QString            name;
    QString            host;
    quint16            port{6697};
    bool               ssl{true};
    QString            nick;
    QString            user;
    QString            realname;
    QString            password;   // PASS / bouncer
    QList<ChannelConfig> channels;
};

struct UiConfig {
    QString  theme{"default"};
    QString  icon{"maindefault"};
    bool     showNickPrefix{true};
    bool     showTopic{true};
    bool     showEmojiButton{false};
    bool     coloredNicks{true};
};

struct Config {
    QList<ServerConfig> servers;
    UiConfig            ui;

    bool needsNickSetup() const;

    static Config    load(const QString &path);
    static void      save(const Config &cfg, const QString &path);
    static QString   defaultPath();
    static void      ensureExists(const QString &path);
};
