#include "mainwindow.h"
#include "irc/ircclient.h"
#include "ui/trayicon.h"
#include "ui/aboutdialog.h"
#include "ui/docsdialog.h"
#include "ui/fontdialog.h"
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
#include <QSysInfo>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include "version.h"

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
    applyFontSizes();

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

    // Font config
    auto *fontAct = menu->addAction("Font Config...");
    connect(fontAct, &QAction::triggered, this, [this]{
        FontDialog dlg(m_config.ui.fontFamily, m_config.ui.fontSizes, this);
        if (dlg.exec() == QDialog::Accepted) {
            m_config.ui.fontFamily = dlg.selectedFamily();
            m_config.ui.fontSizes  = dlg.selectedSizes();
            Config::save(m_config, Config::defaultPath());
            QFont appFont(m_config.ui.fontFamily);
            appFont.setStyleHint(QFont::Monospace);
            QApplication::setFont(appFont);
            applyFontSizes();
        }
    });

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

    // Typing indicator toggle
    auto *typingAct = menu->addAction("Typing Indicator");
    typingAct->setCheckable(true);
    typingAct->setChecked(m_config.ui.typingIndicator);
    connect(typingAct, &QAction::toggled, this, [this](bool on){
        m_config.ui.typingIndicator = on;
        Config::save(m_config, Config::defaultPath());
        if (!on) {
            m_typingOutTimer->stop();
            m_typingNicks.clear();
            for (auto *t : std::as_const(m_typingNickTimers)) { t->stop(); t->deleteLater(); }
            m_typingNickTimers.clear();
            m_typingLabel->setVisible(false);
        }
    });

    // Colored nicks toggle
    auto *colorNicksAct = menu->addAction("Colored Nicks");
    colorNicksAct->setCheckable(true);
    colorNicksAct->setChecked(m_config.ui.coloredNicks);
    connect(colorNicksAct, &QAction::toggled, this, [this](bool on){
        m_config.ui.coloredNicks = on;
        Config::save(m_config, Config::defaultPath());
        // Refresh nick list so color change takes effect immediately
        if (!m_model->activeHost().isEmpty() && !m_model->activeChannel().isEmpty())
            refreshNickList(m_model->activeHost(), m_model->activeChannel());
    });

    m_hamburger->setMenu(menu);
    tb->addWidget(m_hamburger);

    m_appLabel = new QLabel("  Uplink");
    QFont f = m_appLabel->font();
    f.setBold(true);
    m_appLabel->setFont(f);
    tb->addWidget(m_appLabel);

    tb->addSeparator();
}

void MainWindow::applyFontSizes()
{
    const QString &fam = m_config.ui.fontFamily;
    const FontSizes &fs = m_config.ui.fontSizes;

    auto makeFont = [&](int pt) {
        QFont f(fam, pt);
        f.setStyleHint(QFont::Monospace);
        return f;
    };

    if (m_hamburger) {
        QFont f = makeFont(fs.toolbar);
        f.setBold(false);
        m_hamburger->setFont(f);
    }
    if (m_appLabel) {
        QFont f = makeFont(fs.toolbar);
        f.setBold(true);
        m_appLabel->setFont(f);
    }
    if (m_sidebar)        m_sidebar->setFont(makeFont(fs.sidebar));
    if (m_chatView)       m_chatView->setFont(makeFont(fs.chat));
    if (m_nickList)       m_nickList->setFont(makeFont(fs.nickList));
    if (m_nickDock)       m_nickDock->setFont(makeFont(fs.nickDock));
    if (m_topicLabel)    m_topicLabel->setFont(makeFont(fs.topicBar));
    if (m_modesLabel)    m_modesLabel->setFont(makeFont(fs.topicBar));
    if (m_userInfoLabel) m_userInfoLabel->setFont(makeFont(fs.topicBar));
    if (m_nickPrefix)   m_nickPrefix->setFont(makeFont(fs.inputNick));
    if (m_input)        m_input->setFont(makeFont(fs.input));
    if (m_typingLabel) {
        QFont f = makeFont(qMax(fs.input - 1, 7));
        f.setItalic(true);
        m_typingLabel->setFont(f);
    }
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

    // Topic bar — shows: #channel (modes)  ·  Server — N users
    m_topicBar  = new QWidget;
    auto *tHbox = new QHBoxLayout(m_topicBar);
    tHbox->setContentsMargins(6, 3, 6, 3);
    tHbox->setSpacing(6);

    m_topicLabel = new QLabel;
    m_topicLabel->setObjectName("channelLabel");

    m_modesLabel = new QLabel;
    m_modesLabel->setObjectName("modesLabel");

    m_userInfoLabel = new QLabel;
    m_userInfoLabel->setObjectName("userInfoLabel");
    tHbox->addWidget(m_topicLabel);
    tHbox->addWidget(m_modesLabel);
    tHbox->addWidget(m_userInfoLabel);
    tHbox->addStretch(1);
    m_topicBar->setObjectName("topicBar");
    m_topicBar->setVisible(m_showTopic);
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

    bar->setObjectName("inputBar");

    auto *layout = qobject_cast<QVBoxLayout *>(centralWidget()->layout());

    // Typing indicator label — sits just above the input bar
    m_typingLabel = new QLabel;
    m_typingLabel->setObjectName("typingLabel");
    m_typingLabel->setContentsMargins(6, 1, 6, 1);
    m_typingLabel->setVisible(false);
    layout->addWidget(m_typingLabel);

    layout->addWidget(bar);

    // Debounce timer: sends typing=active 1s after the user starts typing
    m_typingOutTimer = new QTimer(this);
    m_typingOutTimer->setSingleShot(true);
    m_typingOutTimer->setInterval(1000);
    connect(m_typingOutTimer, &QTimer::timeout, this, [this]{
        if (!m_config.ui.typingIndicator) return;
        const QString host = m_model->activeHost();
        const QString ch   = m_model->activeChannel();
        if (ch.isEmpty() || ch == "(server)") return;
        m_model->sendTyping(host, ch, "active");
    });

    connect(m_input, &QLineEdit::textChanged, this, [this](const QString &text){
        if (!m_config.ui.typingIndicator) return;
        const QString host = m_model->activeHost();
        const QString ch   = m_model->activeChannel();
        if (ch.isEmpty() || ch == "(server)") return;
        if (!text.isEmpty()) {
            m_typingOutTimer->start();
        } else {
            m_typingOutTimer->stop();
            m_model->sendTyping(host, ch, "done");
        }
    });

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
    connect(m_model, &SessionModel::modesChanged, this, [this](const QString &host, const QString &channel){
        if (host == m_model->activeHost() && channel.toLower() == m_model->activeChannel().toLower())
            refreshTopicBar(host, channel);
    });
    connect(m_model, &SessionModel::nickListChanged,   this, &MainWindow::onNickListChanged);
    connect(m_model, &SessionModel::unreadChanged,     this, &MainWindow::onUnreadChanged);
    connect(m_model, &SessionModel::selfNickChanged,   this, &MainWindow::onSelfNickChanged);
    connect(m_model, &SessionModel::typingReceived,    this, &MainWindow::onTypingReceived);
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
    if (channel == "(server)") return srv;
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
    QString label = host;
    for (const auto &sc : std::as_const(m_config.servers))
        if (sc.host == host && !sc.name.isEmpty()) { label = sc.name; break; }
    auto *item = new QTreeWidgetItem(m_sidebar);
    item->setText(0, label);
    item->setData(0, Qt::UserRole,     host);
    item->setData(0, Qt::UserRole + 1, QString("(server)"));
    item->setExpanded(true);
}

void MainWindow::onServerConnected(const QString &host)
{
    auto *item = findServerItem(host);
    if (item) {
        QString label = host;
        for (const auto &sc : std::as_const(m_config.servers))
            if (sc.host == host && !sc.name.isEmpty()) { label = sc.name; break; }
        item->setText(0, label);
    }
    statusBar()->showMessage("Connected to " + host);
}

void MainWindow::onServerDisconnected(const QString &host)
{
    auto *item = findServerItem(host);
    if (item) {
        QString label = host;
        for (const auto &sc : std::as_const(m_config.servers))
            if (sc.host == host && !sc.name.isEmpty()) { label = sc.name; break; }
        item->setText(0, label + " (disconnected)");
    }
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

    if (m_model->activeChannel().isEmpty()) {
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
    Q_UNUSED(topic)
    if (host == m_model->activeHost() &&
        channel.toLower() == m_model->activeChannel().toLower())
        refreshTopicBar(host, channel);
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
    QString label = channel;
    if (channel == "(server)") {
        label = host;
        for (const auto &sc : std::as_const(m_config.servers))
            if (sc.host == host && !sc.name.isEmpty()) { label = sc.name; break; }
    }
    item->setText(0, count > 0 ? label + " (" + QString::number(count) + ")" : label);
}

void MainWindow::onSelfNickChanged(const QString &host, const QString &nick)
{
    if (host == m_model->activeHost())
        m_nickPrefix->setText(nick);
}

void MainWindow::onTypingReceived(const QString &host, const QString &channel,
                                   const QString &nick, const QString &state)
{
    if (!m_config.ui.typingIndicator) return;

    const QString key      = host + "|" + channel;
    const QString timerKey = host + "|" + channel + "|" + nick;

    if (state == "active" || state == "paused") {
        m_typingNicks[key].insert(nick);

        if (m_typingNickTimers.contains(timerKey)) {
            m_typingNickTimers[timerKey]->start(6000);
        } else {
            auto *t = new QTimer(this);
            t->setSingleShot(true);
            connect(t, &QTimer::timeout, this, [this, key, timerKey, nick]{
                m_typingNicks[key].remove(nick);
                if (auto *timer = m_typingNickTimers.value(timerKey)) {
                    m_typingNickTimers.remove(timerKey);
                    timer->deleteLater();
                }
                updateTypingLabel();
            });
            m_typingNickTimers.insert(timerKey, t);
            t->start(6000);
        }
    } else {
        m_typingNicks[key].remove(nick);
        if (auto *t = m_typingNickTimers.value(timerKey)) {
            t->stop();
            t->deleteLater();
            m_typingNickTimers.remove(timerKey);
        }
    }

    updateTypingLabel();
}

void MainWindow::updateTypingLabel()
{
    const QString key = m_model->activeHost() + "|" + m_model->activeChannel();
    const QSet<QString> &typers = m_typingNicks.value(key);

    if (typers.isEmpty() || !m_config.ui.typingIndicator) {
        m_typingLabel->setVisible(false);
        return;
    }

    QStringList names(typers.begin(), typers.end());
    QString text;
    if (names.size() == 1)
        text = names[0] + " is typing...";
    else if (names.size() == 2)
        text = names[0] + " and " + names[1] + " are typing...";
    else
        text = QString::number(names.size()) + " people are typing...";

    m_typingLabel->setText(text);
    m_typingLabel->setVisible(true);
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

// ---------------------------------------------------------------------------
// /sysinfo helpers
// ---------------------------------------------------------------------------

static QString sysinfoOS()
{
#if defined(Q_OS_LINUX)
    QFile f("/etc/os-release");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("PRETTY_NAME=")) {
                QString val = line.mid(12);
                if (val.startsWith('"') && val.endsWith('"'))
                    val = val.mid(1, val.length() - 2);
                return val;
            }
        }
    }
    return "Linux";
#elif defined(Q_OS_FREEBSD)
    QProcess p;
    p.start("uname", {"-s"});
    p.waitForFinished(2000);
    const QString name = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    QProcess r;
    r.start("uname", {"-r"});
    r.waitForFinished(2000);
    const QString rel = QString::fromLocal8Bit(r.readAllStandardOutput()).trimmed();
    return name + " " + rel;
#else
    return QSysInfo::prettyProductName();
#endif
}

static QString sysinfoKernel()
{
    QProcess p;
    p.start("uname", {"-r"});
    p.waitForFinished(2000);
    const QString out = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    return out.isEmpty() ? "Unknown" : out;
}

static QString sysinfoCPU()
{
#if defined(Q_OS_LINUX)
    QFile f("/proc/cpuinfo");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        QString model;
        int threads = 0;
        while (!in.atEnd()) {
            const QString line = in.readLine();
            if (line.startsWith("model name") && model.isEmpty()) {
                const int colon = line.indexOf(':');
                if (colon != -1)
                    model = line.mid(colon + 1).trimmed();
            }
            if (line.startsWith("processor"))
                ++threads;
        }
        if (!model.isEmpty())
            return QString("%1 (%2 threads)").arg(model).arg(threads);
    }
    return QSysInfo::currentCpuArchitecture();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_DARWIN)
    QProcess pm;
    pm.start("sysctl", {"-n", "hw.model"});
    pm.waitForFinished(2000);
    const QString model = QString::fromLocal8Bit(pm.readAllStandardOutput()).trimmed();
    QProcess pc;
    pc.start("sysctl", {"-n", "hw.ncpu"});
    pc.waitForFinished(2000);
    const QString ncpu = QString::fromLocal8Bit(pc.readAllStandardOutput()).trimmed();
    if (!model.isEmpty() && !ncpu.isEmpty())
        return QString("%1 (%2 threads)").arg(model, ncpu);
    return model.isEmpty() ? "Unknown" : model;
#else
    return QSysInfo::currentCpuArchitecture();
#endif
}

static QString sysinfoRAM()
{
#if defined(Q_OS_LINUX)
    QFile f("/proc/meminfo");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        quint64 total = 0, available = 0;
        while (!in.atEnd()) {
            const QStringList parts = in.readLine().split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                if (parts[0] == "MemTotal:")     total     = parts[1].toULongLong();
                if (parts[0] == "MemAvailable:") available = parts[1].toULongLong();
            }
        }
        if (total > 0) {
            const quint64 used = total - available;
            return QString("%1/%2GB")
                .arg(double(used)  / 1024.0 / 1024.0, 0, 'f', 1)
                .arg(double(total) / 1024.0 / 1024.0, 0, 'f', 0);
        }
    }
    return "Unknown";
#elif defined(Q_OS_FREEBSD)
    QProcess pp;
    pp.start("sysctl", {"-n", "hw.physmem"});
    pp.waitForFinished(2000);
    const quint64 total = QString::fromLocal8Bit(pp.readAllStandardOutput()).trimmed().toULongLong();
    QProcess pf;
    pf.start("sysctl", {"-n", "vm.stats.vm.v_free_count"});
    pf.waitForFinished(2000);
    const quint64 freePages = QString::fromLocal8Bit(pf.readAllStandardOutput()).trimmed().toULongLong();
    QProcess ps;
    ps.start("sysctl", {"-n", "hw.pagesize"});
    ps.waitForFinished(2000);
    const quint64 pageSize = QString::fromLocal8Bit(ps.readAllStandardOutput()).trimmed().toULongLong();
    if (total > 0 && pageSize > 0) {
        const quint64 used = total - freePages * pageSize;
        return QString("%1/%2GB")
            .arg(double(used)  / 1024.0 / 1024.0 / 1024.0, 0, 'f', 1)
            .arg(double(total) / 1024.0 / 1024.0 / 1024.0, 0, 'f', 0);
    }
    return "Unknown";
#else
    return "Unknown";
#endif
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

    // Stop typing notification on send
    m_typingOutTimer->stop();
    if (m_config.ui.typingIndicator && channel != "(server)")
        m_model->sendTyping(host, channel, "done");

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
        } else if (cmd == "/notice") {
            const QString target = args.section(' ', 0, 0);
            const QString msg    = args.section(' ', 1);
            if (!target.isEmpty() && !msg.isEmpty())
                m_model->sendRaw(host, "NOTICE " + target + " :" + msg);
        } else if (cmd == "/quote" || cmd == "/raw") {
            m_model->sendRaw(host, args);
        } else if (cmd == "/quit") {
            if (auto *cl = m_model->clientFor(host)) cl->quit(args.isEmpty() ? "UplinkIRC" : args);
        } else if (cmd == "/away") {
            m_model->sendRaw(host, args.isEmpty() ? "AWAY" : "AWAY :" + args);
        } else if (cmd == "/back") {
            m_model->sendRaw(host, "AWAY");
        } else if (cmd == "/motd") {
            m_model->sendRaw(host, args.isEmpty() ? "MOTD" : "MOTD " + args.trimmed());
        } else if (cmd == "/whois") {
            m_model->sendRaw(host, "WHOIS " + args.trimmed());
        } else if (cmd == "/topic") {
            if (args.isEmpty())
                m_model->sendRaw(host, "TOPIC " + channel);
            else
                m_model->sendRaw(host, "TOPIC " + channel + " :" + args);
        } else if (cmd == "/kick") {
            const QString target = args.section(' ', 0, 0);
            const QString reason = args.section(' ', 1);
            if (!target.isEmpty())
                m_model->sendRaw(host, "KICK " + channel + " " + target
                                 + (reason.isEmpty() ? "" : " :" + reason));
        } else if (cmd == "/version") {
            if (args.isEmpty())
                m_model->sendRaw(host, "VERSION");
            else
                m_model->sendRaw(host, "PRIVMSG " + args.trimmed() + " :\x01VERSION\x01");
        } else if (cmd == "/ctcp") {
            const QString target   = args.section(' ', 0, 0);
            const QString ctcpcmd  = args.section(' ', 1, 1).toUpper();
            const QString ctcpargs = args.section(' ', 2);
            if (!target.isEmpty() && !ctcpcmd.isEmpty()) {
                const QString ctcp = ctcpargs.isEmpty()
                    ? "\x01" + ctcpcmd + "\x01"
                    : "\x01" + ctcpcmd + " " + ctcpargs + "\x01";
                m_model->sendRaw(host, "PRIVMSG " + target + " :" + ctcp);
            }
        } else if (cmd == "/sysinfo") {
            const QString info = QString("OS: %1 | Kernel: %2 | CPU: %3 | RAM: %4")
                .arg(sysinfoOS(), sysinfoKernel(), sysinfoCPU(), sysinfoRAM());
            m_model->sendMessage(host, channel, info);
        } else if (cmd == "/help") {
            const QStringList lines = {
                "Available commands:",
                "  /join <channel> [key]       — join a channel",
                "  /part [message]             — leave the current channel",
                "  /nick <newnick>             — change your nick",
                "  /me <action>                — send an action (/me waves)",
                "  /msg <target> <message>     — send a private message",
                "  /notice <target> <message>  — send a NOTICE",
                "  /topic [text]               — show or set the channel topic",
                "  /kick <nick> [reason]       — kick a user",
                "  /away [message]             — set away status",
                "  /back                       — clear away status",
                "  /whois <nick>               — request WHOIS info",
                "  /motd [server]              — request the MOTD",
                "  /version [nick]             — request VERSION (nick optional)",
                "  /ctcp <target> <cmd> [args] — send a CTCP request",
                "  /sysinfo                    — post client/system info to channel",
                "  /quote <raw>  /raw <raw>    — send a raw IRC line",
                "  /quit [message]             — disconnect from server",
            };
            for (const QString &line : lines)
                appendMessage(Message::make(MessageType::Server, "", line));
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
    updateTypingLabel();
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

    if (channel == "(server)") {
        m_topicLabel->setText(host);
        m_modesLabel->clear();
    } else {
        m_topicLabel->setText(channel);
        const QString modes = ch ? ch->modes : QString();
        m_modesLabel->setText(modes.isEmpty() ? QString() : "(" + modes + ")");
    }

    QString serverName = host;
    for (const auto &sc : std::as_const(m_config.servers))
        if (sc.host == host && !sc.name.isEmpty()) { serverName = sc.name; break; }

    m_userInfoLabel->setText(serverName);
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

static QString ircToHtml(const QString &raw)
{
    static const char* const kColors[] = {
        "#FFFFFF","#000000","#00007F","#009300",
        "#FF0000","#7F0000","#9C009C","#FC7F00",
        "#FFFF00","#00FC00","#009393","#00FFFF",
        "#0000FC","#FF00FF","#7F7F7F","#D2D2D2"
    };

    struct Fmt {
        bool bold{false}, italic{false}, underline{false}, strike{false};
        int  fg{-1}, bg{-1};
        bool operator==(const Fmt &o) const {
            return bold==o.bold && italic==o.italic && underline==o.underline
                && strike==o.strike && fg==o.fg && bg==o.bg;
        }
    };

    Fmt     cur;
    int     openSpans = 0;
    QString out;

    auto closeAll = [&] {
        for (int i = 0; i < openSpans; ++i) out += "</span>";
        openSpans = 0;
    };

    auto applyFmt = [&](const Fmt &next) {
        if (next == cur) return;
        closeAll();
        cur = next;
        QString style;
        if (cur.bold)      style += "font-weight:bold;";
        if (cur.italic)    style += "font-style:italic;";
        QString td;
        if (cur.underline) td += "underline ";
        if (cur.strike)    td += "line-through ";
        if (!td.isEmpty()) style += "text-decoration:" + td.trimmed() + ";";
        if (cur.fg >= 0 && cur.fg < 16) style += QString("color:%1;").arg(kColors[cur.fg]);
        if (cur.bg >= 0 && cur.bg < 16) style += QString("background-color:%1;").arg(kColors[cur.bg]);
        if (!style.isEmpty()) {
            out += "<span style='" + style + "'>";
            openSpans = 1;
        }
    };

    int i = 0;
    const int len = raw.size();
    while (i < len) {
        const ushort c = raw[i].unicode();
        if (c == 0x02) {                          // bold
            Fmt n = cur; n.bold = !cur.bold; applyFmt(n); ++i;
        } else if (c == 0x1D) {                   // italic
            Fmt n = cur; n.italic = !cur.italic; applyFmt(n); ++i;
        } else if (c == 0x1F) {                   // underline
            Fmt n = cur; n.underline = !cur.underline; applyFmt(n); ++i;
        } else if (c == 0x1E) {                   // strikethrough
            Fmt n = cur; n.strike = !cur.strike; applyFmt(n); ++i;
        } else if (c == 0x16) {                   // reverse fg/bg
            Fmt n = cur; std::swap(n.fg, n.bg); applyFmt(n); ++i;
        } else if (c == 0x0F || c == 0x03 && i+1 < len && !raw[i+1].isDigit() && raw[i+1] != ',') {
            // reset all (\x0F) or bare \x03
            applyFmt({}); ++i;
        } else if (c == 0x03) {                   // color \x03[fg[,bg]]
            ++i;
            Fmt n = cur;
            if (i < len && raw[i].isDigit()) {
                int fg = raw[i++].digitValue();
                if (i < len && raw[i].isDigit()) fg = fg * 10 + raw[i++].digitValue();
                n.fg = fg;
                if (i < len && raw[i] == ',' && i+1 < len && raw[i+1].isDigit()) {
                    ++i;
                    int bg = raw[i++].digitValue();
                    if (i < len && raw[i].isDigit()) bg = bg * 10 + raw[i++].digitValue();
                    n.bg = bg;
                }
            } else {
                n.fg = -1; n.bg = -1;
            }
            applyFmt(n);
        } else if (c == 0x11) {                   // monospace — skip control char
            ++i;
        } else {
            if      (raw[i] == '<') out += "&lt;";
            else if (raw[i] == '>') out += "&gt;";
            else if (raw[i] == '&') out += "&amp;";
            else if (raw[i] == '"') out += "&quot;";
            else                    out += raw[i];
            ++i;
        }
    }
    closeAll();
    return out;
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
                 ircToHtml(msg.text));
    }
    case MessageType::Action:
        return QString("<span style='color:gray'>%1</span> "
                       "<i>* %2 %3</i>")
            .arg(ts, msg.nick.toHtmlEscaped(), ircToHtml(msg.text));

    case MessageType::Notice:
        return QString("<span style='color:gray'>%1</span> "
                       "<span style='color:#cc8800'>-%2- %3</span>")
            .arg(ts, msg.nick.toHtmlEscaped(), ircToHtml(msg.text));

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
