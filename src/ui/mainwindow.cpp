#include "mainwindow.h"
#include "irc/ircclient.h"
#include "ui/trayicon.h"
#include "ui/aboutdialog.h"
#include "ui/docsdialog.h"
#include "ui/appicons.h"
#include "ui/themeloader.h"
#include "config/config.h"

#include <QApplication>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QDockWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QSplitter>
#include <QTextCharFormat>
#include <QScrollBar>
#include <QInputDialog>

// ---------------------------------------------------------------------------
// Nick color — consistent hash-based color per nick
// ---------------------------------------------------------------------------

QColor MainWindow::nickColor(const QString &nick)
{
    static const char *palette[] = {
        "#e06c75", "#98c379", "#e5c07b", "#61afef",
        "#c678dd", "#56b6c2", "#d19a66", "#7ec8a0",
        "#ff7b72", "#79c0ff", "#ffa657", "#85e89d",
        "#f78166", "#58a6ff", "#d4a0f5", "#4db5bd",
    };
    static constexpr int N = int(sizeof(palette) / sizeof(palette[0]));
    return QColor(palette[qHash(nick.toLower()) % N]);
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

MainWindow::MainWindow(SessionModel *model, const Config &cfg, QWidget *parent)
    : QMainWindow(parent)
    , m_model(model)
    , m_config(cfg)
{
    // Init UI toggles from config
    m_showNickPrefix = cfg.ui.showNickPrefix;
    m_showTopic      = cfg.ui.showTopic;
    m_showEmojiBtn   = cfg.ui.showEmojiButton;

    // Sync icon preference from config (config takes precedence over QSettings)
    AppIcons::setActiveIconName(m_config.ui.icon);

    setWindowTitle("UplinkIRC");
    setWindowIcon(AppIcons::appIcon());
    resize(1100, 700);

    ThemeLoader::apply(m_config.ui.theme);
    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    setupToolbar();
    setupSidebar();
    setupNickDock();
    setupChatArea();
    setupInputBar();
    connectModel();

    if (QSystemTrayIcon::isSystemTrayAvailable())
        m_tray = new TrayIcon(model, this);

    statusBar()->showMessage("UplinkIRC ready");
}

MainWindow::~MainWindow() = default;

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void MainWindow::setupToolbar()
{
    auto *tb = addToolBar("Main");
    tb->setMovable(false);
    tb->setFloatable(false);

    m_hamburger = new QToolButton;
    m_hamburger->setText("☰");
    m_hamburger->setPopupMode(QToolButton::InstantPopup);

    auto *menu = new QMenu(m_hamburger);

    // About
    auto *aboutAct = menu->addAction("About UplinkIRC");
    connect(aboutAct, &QAction::triggered, this, [this]{
        AboutDialog dlg(this);
        dlg.exec();
    });

    // Documentation
    auto *docsAct = menu->addAction("Documentation");
    connect(docsAct, &QAction::triggered, this, [this]{
        if (!m_docsDialog)
            m_docsDialog = new DocsDialog(this);
        m_docsDialog->show();
        m_docsDialog->raise();
        m_docsDialog->activateWindow();
    });

    // App icon picker
    auto *iconMenu    = menu->addMenu("App Icon");
    auto *iconDefault = iconMenu->addAction("Default");
    auto *iconAlt     = iconMenu->addAction("Alternative");
    iconDefault->setCheckable(true);
    iconAlt->setCheckable(true);
    iconDefault->setChecked(m_config.ui.icon == "maindefault");
    iconAlt->setChecked(m_config.ui.icon == "mainalt");

    auto applyIcon = [this, iconDefault, iconAlt](const QString &name) {
        AppIcons::setActiveIconName(name);
        m_config.ui.icon = name;
        const QIcon icon = AppIcons::appIcon();
        setWindowIcon(icon);
        if (m_tray) m_tray->setBaseIcon(icon);
        iconDefault->setChecked(name == "maindefault");
        iconAlt->setChecked(name == "mainalt");
        Config::save(m_config, Config::defaultPath());
    };
    connect(iconDefault, &QAction::triggered, this, [applyIcon]{ applyIcon("maindefault"); });
    connect(iconAlt,     &QAction::triggered, this, [applyIcon]{ applyIcon("mainalt"); });

    // Theme picker
    auto *themeMenu = menu->addMenu("Theme");
    for (const QString &name : ThemeLoader::availableThemes()) {
        themeMenu->addAction(name, this, [this, name]{
            m_config.ui.theme = name;
            ThemeLoader::apply(name);
            Config::save(m_config, Config::defaultPath());
        });
    }

    menu->addSeparator();

    // Topic toggle
    auto *topicAct = menu->addAction("Show Topic Bar");
    topicAct->setCheckable(true);
    topicAct->setChecked(m_showTopic);
    m_toggleTopicAction = topicAct;
    connect(topicAct, &QAction::toggled, this, [this](bool on){
        m_showTopic = on;
        m_config.ui.showTopic = on;
        m_topicBar->setVisible(on);
        Config::save(m_config, Config::defaultPath());
    });

    // Nick prefix toggle
    auto *nickPrefixAct = menu->addAction("Show Nick in Input");
    nickPrefixAct->setCheckable(true);
    nickPrefixAct->setChecked(m_showNickPrefix);
    connect(nickPrefixAct, &QAction::toggled, this, [this](bool on){
        m_showNickPrefix = on;
        m_config.ui.showNickPrefix = on;
        m_nickPrefix->setVisible(on);
        Config::save(m_config, Config::defaultPath());
    });

    // Emoji button toggle
    auto *emojiAct = menu->addAction("Show Emoji Button");
    emojiAct->setCheckable(true);
    emojiAct->setChecked(m_showEmojiBtn);
    connect(emojiAct, &QAction::toggled, this, [this](bool on){
        m_showEmojiBtn = on;
        m_config.ui.showEmojiButton = on;
        m_emojiBtn->setVisible(on);
        Config::save(m_config, Config::defaultPath());
    });

    m_hamburger->setMenu(menu);
    tb->addWidget(m_hamburger);

    auto *appLabel = new QLabel("  Uplink");
    QFont f = appLabel->font();
    f.setBold(true);
    f.setPointSize(f.pointSize() + 1);
    appLabel->setFont(f);
    tb->addWidget(appLabel);

    tb->addSeparator();
}

void MainWindow::setupSidebar()
{
    m_sidebar = new QTreeWidget;
    m_sidebar->setHeaderHidden(true);
    m_sidebar->setRootIsDecorated(true);
    m_sidebar->setIndentation(12);
    m_sidebar->setMinimumWidth(140);
    m_sidebar->setMaximumWidth(220);

    auto *dock = new QDockWidget("Servers", this);
    dock->setWidget(m_sidebar);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(m_sidebar, &QTreeWidget::currentItemChanged,
            this, [this](QTreeWidgetItem *, QTreeWidgetItem *){ onSidebarSelectionChanged(); });
}

void MainWindow::setupNickDock()
{
    m_nickList = new QListWidget;
    m_nickList->setMinimumWidth(120);
    m_nickList->setMaximumWidth(180);

    m_nickDock = new QDockWidget("Users", this);
    m_nickDock->setWidget(m_nickList);
    m_nickDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_nickDock);
}

void MainWindow::setupChatArea()
{
    auto *central = new QWidget;
    auto *vbox    = new QVBoxLayout(central);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    // Topic bar
    m_topicBar  = new QWidget;
    auto *tHbox = new QHBoxLayout(m_topicBar);
    tHbox->setContentsMargins(6, 3, 6, 3);

    m_modesLabel = new QLabel;
    m_modesLabel->setStyleSheet("font-family: monospace; color: gray;");
    m_topicLabel = new QLabel;
    m_topicLabel->setWordWrap(false);
    m_topicLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_topicLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    tHbox->addWidget(m_modesLabel);
    tHbox->addWidget(m_topicLabel, 1);
    m_topicBar->setVisible(m_showTopic);
    m_topicBar->setStyleSheet("background: palette(mid);");
    vbox->addWidget(m_topicBar);

    // Chat view
    m_chatView = new QTextEdit;
    m_chatView->setReadOnly(true);
    m_chatView->setLineWrapMode(QTextEdit::WidgetWidth);
    vbox->addWidget(m_chatView, 1);

    setCentralWidget(central);
}

void MainWindow::setupInputBar()
{
    auto *bar  = new QWidget;
    auto *hbox = new QHBoxLayout(bar);
    hbox->setContentsMargins(4, 3, 4, 3);
    hbox->setSpacing(4);

    m_nickPrefix = new QLabel;
    m_nickPrefix->setStyleSheet("font-weight: bold; padding-right: 4px;");
    m_nickPrefix->setVisible(m_showNickPrefix);

    m_input = new QLineEdit;
    m_input->setPlaceholderText("Type a message...");
    m_input->installEventFilter(this);

    m_emojiBtn = new QPushButton("😊");
    m_emojiBtn->setFixedWidth(32);
    m_emojiBtn->setVisible(m_showEmojiBtn);

    hbox->addWidget(m_nickPrefix);
    hbox->addWidget(m_input, 1);
    hbox->addWidget(m_emojiBtn);

    bar->setStyleSheet("background: palette(base); border-top: 1px solid palette(mid);");

    auto *layout = qobject_cast<QVBoxLayout *>(centralWidget()->layout());
    layout->addWidget(bar);

    connect(m_input, &QLineEdit::returnPressed, this, &MainWindow::onInputSubmit);
}

void MainWindow::connectModel()
{
    connect(m_model, &SessionModel::serverAdded,       this, &MainWindow::onServerAdded);
    connect(m_model, &SessionModel::serverConnected,   this, &MainWindow::onServerConnected);
    connect(m_model, &SessionModel::serverDisconnected,this, &MainWindow::onServerDisconnected);
    connect(m_model, &SessionModel::channelAdded,      this, &MainWindow::onChannelAdded);
    connect(m_model, &SessionModel::channelRemoved,    this, &MainWindow::onChannelRemoved);
    connect(m_model, &SessionModel::messageAdded,      this, &MainWindow::onMessageAdded);
    connect(m_model, &SessionModel::topicChanged,      this, &MainWindow::onTopicChanged);
    connect(m_model, &SessionModel::nickListChanged,   this, &MainWindow::onNickListChanged);
    connect(m_model, &SessionModel::unreadChanged,     this, &MainWindow::onUnreadChanged);
    connect(m_model, &SessionModel::selfNickChanged,   this, &MainWindow::onSelfNickChanged);
}

// ---------------------------------------------------------------------------
// Event filter — Tab completion + input history
// ---------------------------------------------------------------------------

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_input || event->type() != QEvent::KeyPress)
        return QMainWindow::eventFilter(obj, event);

    auto *ke = static_cast<QKeyEvent *>(event);

    if (ke->key() == Qt::Key_Tab) {
        handleTabComplete();
        return true;
    }

    // Any non-Tab key resets completion cycle
    m_tabActive = false;
    m_tabCandidates.clear();

    if (ke->key() == Qt::Key_Up) {
        handleHistoryUp();
        return true;
    }
    if (ke->key() == Qt::Key_Down) {
        handleHistoryDown();
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::handleTabComplete()
{
    const QString text = m_input->text();
    const int pos = m_input->cursorPosition();

    // Find start of word before cursor
    const int wordStart = text.lastIndexOf(' ', pos - 1) + 1;
    const QString prefix = text.mid(wordStart, pos - wordStart);

    if (prefix.isEmpty()) return;

    // Build candidate list if this is a new cycle or prefix changed
    if (!m_tabActive || prefix != m_tabPrefix) {
        m_tabPrefix = prefix;
        m_tabCandidates.clear();
        m_tabCandidateIndex = 0;
        m_tabActive = true;

        auto *ch = m_model->channel(m_model->activeHost(), m_model->activeChannel());
        if (ch) {
            for (const auto &e : std::as_const(ch->nicks))
                if (e.nick.startsWith(prefix, Qt::CaseInsensitive))
                    m_tabCandidates << e.nick;
        }
    }

    if (m_tabCandidates.isEmpty()) return;

    const QString completed = m_tabCandidates[m_tabCandidateIndex];
    m_tabCandidateIndex = (m_tabCandidateIndex + 1) % m_tabCandidates.size();

    // Suffix: ": " at start of line, " " otherwise (only when at end of input)
    QString suffix;
    if (pos == text.length())
        suffix = (wordStart == 0) ? QStringLiteral(": ") : QStringLiteral(" ");

    m_input->setText(text.left(wordStart) + completed + suffix + text.mid(pos));
    m_input->setCursorPosition(wordStart + completed.length() + suffix.length());
}

void MainWindow::handleHistoryUp()
{
    if (m_inputHistory.isEmpty()) return;
    if (m_historyIndex == -1)
        m_historyDraft = m_input->text();
    m_historyIndex = qMin(m_historyIndex + 1, m_inputHistory.size() - 1);
    m_input->setText(m_inputHistory[m_historyIndex]);
    m_input->end(false);
}

void MainWindow::handleHistoryDown()
{
    if (m_historyIndex == -1) return;
    m_historyIndex--;
    if (m_historyIndex < 0) {
        m_historyIndex = -1;
        m_input->setText(m_historyDraft);
    } else {
        m_input->setText(m_inputHistory[m_historyIndex]);
    }
    m_input->end(false);
}

// ---------------------------------------------------------------------------
// Sidebar helpers
// ---------------------------------------------------------------------------

QTreeWidgetItem *MainWindow::findServerItem(const QString &host) const
{
    for (int i = 0; i < m_sidebar->topLevelItemCount(); ++i) {
        auto *item = m_sidebar->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == host)
            return item;
    }
    return nullptr;
}

QTreeWidgetItem *MainWindow::findChannelItem(const QString &host, const QString &channel) const
{
    auto *srv = findServerItem(host);
    if (!srv) return nullptr;
    for (int i = 0; i < srv->childCount(); ++i) {
        auto *item = srv->child(i);
        if (item->data(0, Qt::UserRole + 1).toString().toLower() == channel.toLower())
            return item;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Model → UI slots
// ---------------------------------------------------------------------------

void MainWindow::onServerAdded(const QString &host)
{
    if (findServerItem(host)) return;
    auto *item = new QTreeWidgetItem(m_sidebar);
    item->setText(0, host);
    item->setData(0, Qt::UserRole, host);
    item->setExpanded(true);
    auto *srvBuf = new QTreeWidgetItem(item);
    srvBuf->setText(0, "(server)");
    srvBuf->setData(0, Qt::UserRole,     host);
    srvBuf->setData(0, Qt::UserRole + 1, "(server)");
}

void MainWindow::onServerConnected(const QString &host)
{
    auto *item = findServerItem(host);
    if (item) item->setText(0, host);
    statusBar()->showMessage("Connected to " + host);
}

void MainWindow::onServerDisconnected(const QString &host)
{
    auto *item = findServerItem(host);
    if (item) item->setText(0, host + " (disconnected)");
    statusBar()->showMessage("Disconnected from " + host);
}

void MainWindow::onChannelAdded(const QString &host, const QString &channel)
{
    if (findChannelItem(host, channel)) return;
    auto *srv = findServerItem(host);
    if (!srv) return;
    auto *item = new QTreeWidgetItem(srv);
    item->setText(0, channel);
    item->setData(0, Qt::UserRole,     host);
    item->setData(0, Qt::UserRole + 1, channel);

    if (m_model->activeChannel().isEmpty() && channel != "(server)") {
        m_sidebar->setCurrentItem(item);
        switchToChannel(host, channel);
    }
}

void MainWindow::onChannelRemoved(const QString &host, const QString &channel)
{
    auto *item = findChannelItem(host, channel);
    if (item) delete item;
}

void MainWindow::onMessageAdded(const QString &host, const QString &channel, const Message &msg)
{
    if (host == m_model->activeHost() &&
        channel.toLower() == m_model->activeChannel().toLower())
    {
        appendMessage(msg);
    }
}

void MainWindow::onTopicChanged(const QString &host, const QString &channel, const QString &topic)
{
    if (host == m_model->activeHost() &&
        channel.toLower() == m_model->activeChannel().toLower())
    {
        m_topicLabel->setText(topic);
    }
}

void MainWindow::onNickListChanged(const QString &host, const QString &channel)
{
    if (host != m_model->activeHost() ||
        channel.toLower() != m_model->activeChannel().toLower()) return;
    refreshNickList(host, channel);
}

void MainWindow::onUnreadChanged(const QString &host, const QString &channel, int count)
{
    auto *item = findChannelItem(host, channel);
    if (!item) return;
    item->setText(0, count > 0 ? channel + " (" + QString::number(count) + ")" : channel);
}

void MainWindow::onSelfNickChanged(const QString &host, const QString &nick)
{
    if (host == m_model->activeHost())
        m_nickPrefix->setText(nick);
}

// ---------------------------------------------------------------------------
// UI → Model
// ---------------------------------------------------------------------------

void MainWindow::onSidebarSelectionChanged()
{
    auto *item = m_sidebar->currentItem();
    if (!item) return;
    const QString host    = item->data(0, Qt::UserRole).toString();
    const QString channel = item->data(0, Qt::UserRole + 1).toString();
    if (host.isEmpty() || channel.isEmpty()) return;
    switchToChannel(host, channel);
}

void MainWindow::onInputSubmit()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;

    // Push to history (newest first, skip consecutive duplicates)
    if (m_inputHistory.isEmpty() || m_inputHistory.first() != text) {
        m_inputHistory.prepend(text);
        if (m_inputHistory.size() > 100)
            m_inputHistory.removeLast();
    }
    m_historyIndex = -1;
    m_tabActive = false;
    m_tabCandidates.clear();

    m_input->clear();

    const QString host    = m_model->activeHost();
    const QString channel = m_model->activeChannel();
    if (host.isEmpty() || channel.isEmpty()) return;

    if (text.startsWith('/')) {
        const QString cmd  = text.section(' ', 0, 0).toLower();
        const QString args = text.section(' ', 1);

        if (cmd == "/join") {
            m_model->sendJoin(host, args.section(' ', 0, 0), args.section(' ', 1, 1));
        } else if (cmd == "/part") {
            m_model->sendPart(host, channel, args);
        } else if (cmd == "/nick") {
            m_model->sendNick(host, args.trimmed());
        } else if (cmd == "/me") {
            m_model->sendAction(host, channel, args);
        } else if (cmd == "/msg") {
            m_model->sendMessage(host, args.section(' ', 0, 0), args.section(' ', 1));
        } else if (cmd == "/quote" || cmd == "/raw") {
            m_model->sendRaw(host, args);
        } else if (cmd == "/quit") {
            if (auto *cl = m_model->clientFor(host)) cl->quit(args.isEmpty() ? "UplinkIRC" : args);
        } else {
            appendMessage(Message::make(MessageType::Error, "", "Unknown command: " + cmd));
        }
        return;
    }

    if (channel == "(server)") return;
    m_model->sendMessage(host, channel, text);
}

// ---------------------------------------------------------------------------
// View helpers
// ---------------------------------------------------------------------------

void MainWindow::switchToChannel(const QString &host, const QString &channel)
{
    m_model->setActive(host, channel);
    refreshChatView(host, channel);
    refreshNickList(host, channel);
    refreshTopicBar(host, channel);

    if (auto *sess = m_model->session(host))
        m_nickPrefix->setText(sess->nick);

    setWindowTitle("UplinkIRC — " + channel + " @ " + host);
}

void MainWindow::refreshChatView(const QString &host, const QString &channel)
{
    m_chatView->clear();
    auto *ch = m_model->channel(host, channel);
    if (!ch) return;
    for (const auto &msg : std::as_const(ch->messages))
        appendMessage(msg);
}

void MainWindow::refreshNickList(const QString &host, const QString &channel)
{
    m_nickList->clear();
    auto *ch = m_model->channel(host, channel);
    if (!ch) return;

    for (const auto &e : std::as_const(ch->nicks)) {
        auto *item = new QListWidgetItem(e.display());
        if (m_config.ui.coloredNicks)
            item->setForeground(nickColor(e.nick));
        m_nickList->addItem(item);
    }

    m_nickDock->setWindowTitle(QString("Users (%1)").arg(ch->nicks.size()));
}

void MainWindow::refreshTopicBar(const QString &host, const QString &channel)
{
    auto *ch = m_model->channel(host, channel);
    m_topicLabel->setText(ch ? ch->topic : QString());
    m_modesLabel->setText(ch ? ch->modes : QString());
}

void MainWindow::appendMessage(const Message &msg)
{
    m_chatView->append(formatMessage(msg));
    auto *sb = m_chatView->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_tray && m_tray->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

QString MainWindow::formatMessage(const Message &msg) const
{
    const QString ts = msg.timestamp.toString("hh:mm");

    auto wrap = [](const QString &color, const QString &text) {
        return QString("<span style='color:%1'>%2</span>").arg(color, text.toHtmlEscaped());
    };

    switch (msg.type) {
    case MessageType::Privmsg: {
        const QString color = m_config.ui.coloredNicks
            ? nickColor(msg.nick).name()
            : QString("palette(text)");
        return QString("<span style='color:gray'>%1</span> "
                       "<b style='color:%2'>&lt;%3&gt;</b> %4")
            .arg(ts, color,
                 msg.nick.toHtmlEscaped(),
                 msg.text.toHtmlEscaped());
    }
    case MessageType::Action:
        return QString("<span style='color:gray'>%1</span> "
                       "<i>* %2 %3</i>")
            .arg(ts, msg.nick.toHtmlEscaped(), msg.text.toHtmlEscaped());

    case MessageType::Notice:
        return QString("<span style='color:gray'>%1</span> "
                       "<span style='color:#cc8800'>-%2- %3</span>")
            .arg(ts, msg.nick.toHtmlEscaped(), msg.text.toHtmlEscaped());

    case MessageType::Join:
        return wrap("seagreen",  ts + " → " + msg.text);
    case MessageType::Part:
        return wrap("firebrick", ts + " ← " + msg.text);
    case MessageType::Quit:
        return wrap("firebrick", ts + " ✕ " + msg.text);
    case MessageType::Nick:
        return wrap("steelblue", ts + " ~ "  + msg.text);
    case MessageType::Kick:
        return wrap("firebrick", ts + " ✕ " + msg.text);
    case MessageType::Topic:
        return wrap("steelblue", ts + " ⦁ Topic: " + msg.text);
    case MessageType::Error:
        return wrap("red",       ts + " !! " + msg.text);
    case MessageType::Server:
    default:
        return wrap("gray",      ts + " * "  + msg.text);
    }
}
