#pragma once

#include <QString>
#include <QStringList>
#include <QList>

#if defined(Q_OS_WIN)
inline const char *kDefaultFontFamily = "Consolas";
#else
inline const char *kDefaultFontFamily = "IBM Plex Mono";
#endif

enum class BouncerType { None, ZNC, Soju };

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
    QString            password;         // PASS / bouncer
    QString            saslUser;
    QString            saslPassword;
    QString            nickservPassword;
    BouncerType        bouncerType{BouncerType::None};
    QString            bouncerNetwork;   // soju network name, or empty
    QList<ChannelConfig> channels;

    bool operator==(const ServerConfig &o) const {
        return host == o.host && port == o.port && ssl == o.ssl
            && nick == o.nick && user == o.user && realname == o.realname
            && name == o.name && password == o.password
            && saslUser == o.saslUser && saslPassword == o.saslPassword
            && nickservPassword == o.nickservPassword;
    }
    bool operator!=(const ServerConfig &o) const { return !(*this == o); }
};

struct FontSizes {
    int toolbar{10};      // hamburger button
    int serverHeader{9};  // network name section label
    int sidebar{10};      // channel list
    int chat{10};         // message area
    int nickList{10};     // user list panel
    int nickDock{10};     // "Users (N)" dock title
    int topicBar{10};     // topic bar text
    int inputNick{10};    // your nick label
    int input{10};        // message typing area
    int typing{9};        // "nick is typing..." indicator
};

struct UiConfig {
    QString   theme{"default"};
    bool      showNickPrefix{true};
    bool      showTopic{true};
    bool      showEmojiButton{false};
    bool      coloredNicks{true};
    QString   fontFamily{kDefaultFontFamily};
    FontSizes fontSizes;
    bool      typingIndicator{true};
    bool      showConnStatus{true};
    bool      notifications{true};
    QString   appIcon{"dark"};
    QString   nickBrackets{"<>"};
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
