#include "themeloader.h"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>

#include <toml++/toml.hpp>

// ---------------------------------------------------------------------------
// Theme search path: user config dir → exe dir → cwd
// ---------------------------------------------------------------------------

QString ThemeLoader::themesDir()
{
    const QStringList candidates = {
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/themes",
        QCoreApplication::applicationDirPath() + "/themes",
        "themes"
    };
    for (const QString &p : candidates)
        if (QDir(p).exists()) return p;
    return "themes";
}

QStringList ThemeLoader::availableThemes()
{
    QStringList names;
    const QDir dir(themesDir());
    for (const QString &f : dir.entryList({"*.toml"}, QDir::Files, QDir::Name))
        names << QFileInfo(f).baseName();
    return names;
}

// ---------------------------------------------------------------------------
// Parse
// ---------------------------------------------------------------------------

static QString str(const toml::table &tbl, std::string_view key, const char *def = "#ffffff")
{
    return QString::fromStdString(tbl[key].value_or<std::string>(def));
}

Theme ThemeLoader::load(const QString &name)
{
    const QString path = themesDir() + "/" + name + ".toml";
    Theme t;

    try {
        auto tbl = toml::parse_file(path.toStdString());

        auto get = [&](std::string_view section) -> const toml::table * {
            auto n = tbl[section];
            return n.is_table() ? n.as_table() : nullptr;
        };

        if (auto *g = get("general")) {
            t.background = str(*g, "background", "#1e1e2e");
            t.text       = str(*g, "text",       "#cdd6f4");
            t.border     = str(*g, "border",     "#313244");
            t.accent     = str(*g, "accent",     "#89b4fa");
        }
        if (auto *s = get("sidebar")) {
            t.sidebarBg      = str(*s, "background", "#181825");
            t.sidebarText    = str(*s, "text",       "#6c7086");
            t.sidebarActive  = str(*s, "active",     "#cdd6f4");
            t.sidebarUnread  = str(*s, "unread",     "#89b4fa");
            t.sidebarMention = str(*s, "mention",    "#f38ba8");
            t.sidebarServer  = str(*s, "server",     "#585b70");
        }
        if (auto *b = get("buffer")) {
            t.bufferBg   = str(*b, "background",  "#1e1e2e");
            t.timestamp  = str(*b, "timestamp",   "#6c7086");
            t.serverLine = str(*b, "server_line", "#585b70");
            t.action     = str(*b, "action",      "#cba6f7");
            t.nickSelf   = str(*b, "nick_self",   "#a6e3a1");
        }
        if (auto *h = get("highlights")) {
            t.mentionBg   = str(*h, "mention_bg",   "#2a1a2e");
            t.mentionText = str(*h, "mention_text",  "#f38ba8");
            t.keyword     = str(*h, "keyword",       "#fab387");
        }
        if (auto *n = get("nicklist")) {
            t.nicklistBg   = str(*n, "background", "#181825");
            t.nicklistText = str(*n, "text",       "#6c7086");
            t.op           = str(*n, "op",         "#89b4fa");
            t.halfop       = str(*n, "halfop",     "#cba6f7");
            t.voice        = str(*n, "voice",      "#a6e3a1");
            t.away         = str(*n, "away",       "#45475a");
        }
        if (auto *i = get("input")) {
            t.inputBg    = str(*i, "background", "#313244");
            t.inputText  = str(*i, "text",       "#cdd6f4");
            t.placeholder= str(*i, "placeholder","#6c7086");
            t.inputNick  = str(*i, "nick_color", "#a6e3a1");
        }

        t.valid = true;

    } catch (const toml::parse_error &e) {
        qWarning() << "ThemeLoader: failed to parse" << path << ":" << e.what();
    }

    return t;
}

// ---------------------------------------------------------------------------
// QSS generation — uses named {{key}} substitution to avoid Qt arg() mangling
// ---------------------------------------------------------------------------

static QString fill(QString tpl, const QHash<QString, QString> &vars)
{
    for (auto it = vars.constBegin(); it != vars.constEnd(); ++it)
        tpl.replace("{{" + it.key() + "}}", it.value());
    return tpl;
}

QString ThemeLoader::toStyleSheet(const Theme &t)
{
    static const QString tpl = R"(
/* ── Base ── */
QMainWindow, QDialog, QWidget {
    background-color: {{bg}};
    color: {{text}};
}
QToolBar {
    background-color: {{sidebarBg}};
    border: none;
    spacing: 0px;
    margin: 0px;
    padding: 0px;
}
QToolBar::handle {
    width: 0px;
    height: 0px;
    image: none;
}
QStatusBar {
    background-color: {{sidebarBg}};
    color: {{srvText}};
    border: none;
    font-size: 7pt;
}
/* ── Dock title bars ── */
QWidget#dockTitleBar {
    background-color: {{sidebarBg}};
    max-height: 16px;
}
QToolButton#dockFloatBtn {
    background: transparent;
    color: {{placeholder}};
    border: none;
    font-size: 9pt;
    padding: 0px;
}
QToolButton#dockFloatBtn:hover { color: {{text}}; }
QWidget#nickContainer { background-color: {{nicklistBg}}; }
QToolButton#hamburger {
    background-color: {{inputBg}};
    color: {{text}};
    border: none;
    padding: 0px 6px;
}
QToolButton#hamburger:hover { color: {{accent}}; }
QToolButton#sidebarToggleBtn {
    background-color: {{inputBg}};
    color: {{text}};
    border: none;
    padding: 0px 2px;
}
QToolButton#sidebarToggleBtn:hover { color: {{accent}}; }
QWidget#sidebarPanel {
    background-color: {{sidebarBg}};
}

QSizeGrip {
    width: 0px;
    height: 0px;
    image: none;
}
QMenuBar {
    background-color: {{bg}};
    color: {{text}};
}
QMenuBar::item:selected {
    background-color: {{accent}};
    color: {{text}};
}
QMenu {
    background-color: {{bg}};
    color: {{text}};
    border: 1px solid {{border}};
}
QMenu::item {
    padding: 4px 20px 4px 20px;
}
QMenu::item:selected {
    background-color: {{accent}};
}

/* ── Sidebar ── */
QTreeWidget {
    background-color: {{sidebarBg}};
    color: {{sidebarText}};
    border: none;
    outline: none;
}
QTreeWidget::item {
    padding: 0px 4px;
    height: 18px;
}
QTreeWidget::item:selected {
    background-color: {{accent}};
    color: {{sidebarActive}};
}
QTreeWidget::item:hover:selected {
    background-color: {{accent}};
}
QTreeWidget::branch {
    background-color: {{sidebarBg}};
    border: none;
    image: none;
    width: 0px;
}

/* ── Chat view ── */
QTextEdit {
    background-color: {{bufferBg}};
    color: {{text}};
    border: none;
    selection-background-color: {{accent}};
}

/* ── Nick list ── */
QListWidget {
    background-color: {{nicklistBg}};
    color: {{nicklistText}};
    border: none;
    outline: none;
}
QListWidget::item {
    padding: 0px 4px;
    height: 18px;
}
QListWidget::item:selected {
    background-color: {{accent}};
    color: {{sidebarActive}};
}
QListWidget::item:hover {
    background-color: {{border}};
}

/* ── Input bar ── */
QWidget#inputBar {
    background-color: {{inputBg}};
    border-top: 1px solid {{border}};
}
QWidget#inputBar QLabel {
    background-color: {{inputBg}};
    color: {{inputNick}};
    border: none;
}
QLineEdit {
    background-color: {{inputBg}};
    color: {{inputText}};
    border: none;
    padding: 4px 6px;
    selection-background-color: {{accent}};
}
QLineEdit::placeholder-text {
    color: {{placeholder}};
}

/* ── Dock widgets ── */
QDockWidget {
    color: {{text}};
    titlebar-close-icon: none;
}
QDockWidget::title {
    background-color: {{sidebarBg}};
    padding: 0px 2px;
}

/* ── Scroll bars ── */
QScrollBar:vertical {
    background: {{bg}};
    width: 4px;
    border: none;
}
QScrollBar::handle:vertical {
    background: {{border}};
    border-radius: 2px;
    min-height: 20px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { height: 0; }

/* ── Splitter ── */
QSplitter::handle {
    background: {{border}};
}

/* ── Embedded nick panel ── */
QWidget#nickPanel,
QWidget#nickPanelHeader {
    background-color: {{bufferBg}};
}
QWidget#nickPanel QListWidget {
    background-color: {{bufferBg}};
    color: {{text}};
}
QWidget#nickPanelHeader QToolButton {
    background-color: transparent;
    color: {{text}};
}
QWidget#nickPanelHeader QLabel {
    background-color: transparent;
    color: {{text}};
}

/* ── Tool buttons ── */
QToolButton {
    background-color: {{sidebarBg}};
    color: {{sidebarText}};
    border: none;
    padding: 4px 10px 4px 0px;
}
QToolButton:hover {
    background-color: {{border}};
}

/* ── Labels ── */
QLabel {
    color: {{text}};
    background: transparent;
}

/* ── Push buttons ── */
QPushButton {
    background-color: {{border}};
    color: {{text}};
    border: 1px solid {{border}};
    border-radius: 3px;
    padding: 4px 12px;
}
QPushButton:hover   { background-color: {{accent}}; }
QPushButton:pressed { background-color: {{sidebarBg}}; }

/* ── Dialog buttons ── */
QDialogButtonBox QPushButton {
    min-width: 70px;
}

/* ── Topic / info bar ── */
QWidget#topicBar {
    background-color: {{inputBg}};
    border-bottom: 1px solid {{border}};
}
QWidget#topicLeftZone {
    background-color: {{inputBg}};
}
QWidget#topicRightZone {
    background-color: {{inputBg}};
}
QLabel#channelLabel {
    color: {{accent}};
    font-weight: bold;
}
QLabel#modesLabel {
    color: {{placeholder}};
}
QLabel#userInfoLabel {
    color: {{placeholder}};
}
QWidget#topicDisplay {
    background-color: {{bufferBg}};
    border-bottom: 1px solid {{border}};
}
QLabel#topicText {
    color: {{placeholder}};
    background: transparent;
    padding: 0px;
}

/* ── Typing indicator ── */
QLabel#typingLabel {
    color: {{placeholder}};
    background: transparent;
    padding: 0;
}
)";

    return fill(tpl, {
        {"bg",           t.background},
        {"text",         t.text},
        {"border",       t.border},
        {"accent",       t.accent},
        {"sidebarBg",    t.sidebarBg},
        {"sidebarText",  t.sidebarText},
        {"sidebarActive",t.sidebarActive},
        {"bufferBg",     t.bufferBg},
        {"nicklistBg",   t.nicklistBg},
        {"nicklistText", t.nicklistText},
        {"inputBg",      t.inputBg},
        {"inputText",    t.inputText},
        {"placeholder",  t.placeholder},
        {"inputNick",    t.inputNick},
        {"srvText",      t.sidebarServer},
    });
}

// ---------------------------------------------------------------------------
// Apply to QApplication
// ---------------------------------------------------------------------------

void ThemeLoader::apply(const QString &name)
{
#if defined(Q_OS_WIN)
    // On Windows, let the native style handle the default look.
    // Only apply QSS when the user explicitly picks a custom theme.
    if (name.isEmpty() || name == "default") {
        qApp->setStyleSheet({});
        return;
    }
#endif

    const Theme t = load(name);
    if (!t.valid) {
        qWarning() << "ThemeLoader: could not load theme" << name;
        return;
    }
    qApp->setStyleSheet(toStyleSheet(t));
}
