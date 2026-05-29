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
colored_nicks      = true
typing_indicator   = true
font_family        = "IBM Plex Mono"
font_toolbar    = 10
font_sidebar    = 10
font_chat       = 10
font_nick_list  = 10
font_nick_dock  = 10
font_topic_bar  = 10
font_input_nick = 10
font_input      = 10

[[server]]
name     = "LinuxDojo"
host     = "irc.linuxdojo.org"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "yournick"
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
    QFileInfo fi(path);
    if (fi.exists() && fi.size() > 0) return;

    QDir().mkpath(fi.absolutePath());
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
            cfg.ui.showNickPrefix  = (*ui)["show_nick_prefix"].value_or(true);
            cfg.ui.showTopic       = (*ui)["show_topic"].value_or(true);
            cfg.ui.showEmojiButton = (*ui)["show_emoji_button"].value_or(false);
            cfg.ui.coloredNicks          = (*ui)["colored_nicks"].value_or(true);
            cfg.ui.typingIndicator       = (*ui)["typing_indicator"].value_or(true);
            cfg.ui.fontFamily            = QString::fromStdString((*ui)["font_family"].value_or<std::string>("IBM Plex Mono"));
            cfg.ui.fontSizes.toolbar      = (*ui)["font_toolbar"].value_or(10);
            cfg.ui.fontSizes.serverHeader = (*ui)["font_server_header"].value_or(9);
            cfg.ui.fontSizes.sidebar      = (*ui)["font_sidebar"].value_or(10);
            cfg.ui.fontSizes.chat         = (*ui)["font_chat"].value_or(10);
            cfg.ui.fontSizes.nickList     = (*ui)["font_nick_list"].value_or(10);
            cfg.ui.fontSizes.nickDock     = (*ui)["font_nick_dock"].value_or(10);
            cfg.ui.fontSizes.topicBar     = (*ui)["font_topic_bar"].value_or(10);
            cfg.ui.fontSizes.inputNick    = (*ui)["font_input_nick"].value_or(10);
            cfg.ui.fontSizes.input        = (*ui)["font_input"].value_or(10);
            cfg.ui.fontSizes.typing       = (*ui)["font_typing"].value_or(9);
            cfg.ui.fontSizes.statusBar    = (*ui)["font_status_bar"].value_or(8);
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
                sc.password     = QString::fromStdString((*s)["password"].value_or<std::string>(""));
                sc.saslUser         = QString::fromStdString((*s)["sasl_user"].value_or<std::string>(""));
                sc.saslPassword     = QString::fromStdString((*s)["sasl_password"].value_or<std::string>(""));
                sc.nickservPassword = QString::fromStdString((*s)["nickserv_password"].value_or<std::string>(""));

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
    out << "show_nick_prefix  = " << (cfg.ui.showNickPrefix  ? "true" : "false") << "\n";
    out << "show_topic        = " << (cfg.ui.showTopic       ? "true" : "false") << "\n";
    out << "show_emoji_button = " << (cfg.ui.showEmojiButton ? "true" : "false") << "\n";
    out << "colored_nicks     = " << (cfg.ui.coloredNicks     ? "true" : "false") << "\n";
    out << "typing_indicator  = " << (cfg.ui.typingIndicator  ? "true" : "false") << "\n";
    out << "font_family       = \"" << cfg.ui.fontFamily << "\"\n";
    out << "font_toolbar       = " << cfg.ui.fontSizes.toolbar      << "\n";
    out << "font_server_header = " << cfg.ui.fontSizes.serverHeader << "\n";
    out << "font_sidebar       = " << cfg.ui.fontSizes.sidebar      << "\n";
    out << "font_chat          = " << cfg.ui.fontSizes.chat         << "\n";
    out << "font_nick_list     = " << cfg.ui.fontSizes.nickList     << "\n";
    out << "font_nick_dock     = " << cfg.ui.fontSizes.nickDock     << "\n";
    out << "font_topic_bar     = " << cfg.ui.fontSizes.topicBar     << "\n";
    out << "font_input_nick    = " << cfg.ui.fontSizes.inputNick    << "\n";
    out << "font_input         = " << cfg.ui.fontSizes.input        << "\n";
    out << "font_typing        = " << cfg.ui.fontSizes.typing       << "\n";
    out << "font_status_bar    = " << cfg.ui.fontSizes.statusBar    << "\n\n";

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
            out << "password     = \"" << s.password << "\"\n";
        if (!s.saslUser.isEmpty())
            out << "sasl_user         = \"" << s.saslUser << "\"\n";
        if (!s.saslPassword.isEmpty())
            out << "sasl_password     = \"" << s.saslPassword << "\"\n";
        if (!s.nickservPassword.isEmpty())
            out << "nickserv_password = \"" << s.nickservPassword << "\"\n";
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
