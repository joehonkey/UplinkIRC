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
    QString            saslUser;
    QString            saslPassword;
    QList<ChannelConfig> channels;
};

struct FontSizes {
    int toolbar{10};    // hamburger button + app label
    int sidebar{10};    // server/channel tree
    int chat{10};       // message area
    int nickList{10};   // user list panel
    int nickDock{10};   // "Users (N)" dock title
    int topicBar{10};   // topic bar text + button
    int inputNick{10};  // your nick label
    int input{10};      // message typing area
};

struct UiConfig {
    QString   theme{"default"};
    bool      showNickPrefix{true};
    bool      showTopic{true};
    bool      showEmojiButton{false};
    bool      coloredNicks{true};
    QString   fontFamily{"IBM Plex Mono"};
    FontSizes fontSizes;
    bool      typingIndicator{true};
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
