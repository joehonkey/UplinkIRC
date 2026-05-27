#include "config.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>

#include <toml++/toml.hpp>

static const char *kDefaultConfig = R"(
[ui]
theme           = "default"
show_nick_prefix = true
show_topic      = true
show_emoji_button = false
colored_nicks   = true

[[server]]
name     = "LinuxDojo"
host     = "irc.linuxdojo.org"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"

[[server.channels]]
name = "#uplink"
)";

// ---------------------------------------------------------------------------

QString Config::defaultPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
           + "/config.toml";
}

void Config::ensureExists(const QString &path)
{
    if (QFile::exists(path)) return;

    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        f.write(kDefaultConfig);
    else
        qWarning() << "UplinkIRC: could not create config at" << path;
}

Config Config::load(const QString &path)
{
    ensureExists(path);

    Config cfg;

    try {
        auto tbl = toml::parse_file(path.toStdString());

        // [ui]
        if (auto ui = tbl["ui"].as_table()) {
            cfg.ui.theme           = QString::fromStdString((*ui)["theme"].value_or<std::string>("default"));
            cfg.ui.icon            = QString::fromStdString((*ui)["icon"].value_or<std::string>("maindefault"));
            cfg.ui.showNickPrefix  = (*ui)["show_nick_prefix"].value_or(true);
            cfg.ui.showTopic       = (*ui)["show_topic"].value_or(true);
            cfg.ui.showEmojiButton = (*ui)["show_emoji_button"].value_or(false);
            cfg.ui.coloredNicks    = (*ui)["colored_nicks"].value_or(true);
        }

        // [[server]]
        if (auto servers = tbl["server"].as_array()) {
            for (auto &node : *servers) {
                auto *s = node.as_table();
                if (!s) continue;

                ServerConfig sc;
                sc.name     = QString::fromStdString((*s)["name"].value_or<std::string>(""));
                sc.host     = QString::fromStdString((*s)["host"].value_or<std::string>(""));
                sc.port     = static_cast<quint16>((*s)["port"].value_or(6697));
                sc.ssl      = (*s)["ssl"].value_or(true);
                sc.nick     = QString::fromStdString((*s)["nick"].value_or<std::string>(""));
                sc.user     = QString::fromStdString((*s)["user"].value_or<std::string>("uplink"));
                sc.realname = QString::fromStdString((*s)["realname"].value_or<std::string>("UplinkIRC User"));
                sc.password = QString::fromStdString((*s)["password"].value_or<std::string>(""));

                if (auto chans = (*s)["channels"].as_array()) {
                    for (auto &cn : *chans) {
                        auto *ct = cn.as_table();
                        if (!ct) continue;
                        ChannelConfig cc;
                        cc.name     = QString::fromStdString((*ct)["name"].value_or<std::string>(""));
                        cc.password = QString::fromStdString((*ct)["password"].value_or<std::string>(""));
                        if (!cc.name.isEmpty())
                            sc.channels.append(cc);
                    }
                }

                if (!sc.host.isEmpty())
                    cfg.servers.append(sc);
            }
        }

    } catch (const toml::parse_error &e) {
        qWarning() << "UplinkIRC: config parse error:" << e.what();
    }

    return cfg;
}

void Config::save(const Config &cfg, const QString &path)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "UplinkIRC: could not save config to" << path;
        return;
    }

    QTextStream out(&f);

    out << "[ui]\n";
    out << "theme             = \"" << cfg.ui.theme << "\"\n";
    out << "icon              = \"" << cfg.ui.icon  << "\"\n";
    out << "show_nick_prefix  = " << (cfg.ui.showNickPrefix  ? "true" : "false") << "\n";
    out << "show_topic        = " << (cfg.ui.showTopic       ? "true" : "false") << "\n";
    out << "show_emoji_button = " << (cfg.ui.showEmojiButton ? "true" : "false") << "\n";
    out << "colored_nicks     = " << (cfg.ui.coloredNicks    ? "true" : "false") << "\n\n";

    for (const auto &s : cfg.servers) {
        out << "[[server]]\n";
        out << "name     = \"" << s.name     << "\"\n";
        out << "host     = \"" << s.host     << "\"\n";
        out << "port     = "   << s.port     << "\n";
        out << "ssl      = "   << (s.ssl ? "true" : "false") << "\n";
        out << "nick     = \"" << s.nick     << "\"\n";
        out << "user     = \"" << s.user     << "\"\n";
        out << "realname = \"" << s.realname << "\"\n";
        if (!s.password.isEmpty())
            out << "password = \"" << s.password << "\"\n";
        for (const auto &ch : s.channels) {
            out << "\n[[server.channels]]\n";
            out << "name = \"" << ch.name << "\"\n";
            if (!ch.password.isEmpty())
                out << "password = \"" << ch.password << "\"\n";
        }
        out << "\n";
    }
}

bool Config::needsNickSetup() const
{
    for (const auto &s : servers)
        if (s.nick == "yournick" || s.nick.isEmpty())
            return true;
    return false;
}
