#include "mainwindow.h"
#include "irc/ircclient.h"
#include "ui/trayicon.h"
#include "ui/aboutdialog.h"
#include "ui/docsdialog.h"
#include "ui/fontdialog.h"
#include "ui/preferencesdialog.h"
#include "ui/serverdialog.h"
#include "ui/manageserversdialog.h"
#include "ui/appicons.h"
#include "ui/themeloader.h"
#include "ui/linkpreview.h"
#include "ui/emojipicker.h"
#include "ui/emojidata.h"
#include "ui/menuicons.h"
#include "ui/signalbars.h"
#include "config/config.h"

#include <QApplication>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QTreeWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextBrowser>
#include <QDesktopServices>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QSplitter>
#include <QTextCharFormat>
#include <QScrollBar>
#include <QInputDialog>
#include <QSysInfo>
#include <QTimer>
#include <QSettings>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QPainter>
#include <QPixmap>
#include "version.h"
#include <QRegularExpression>
#include <QToolTip>
#include <QCursor>
#include <QMouseEvent>
#include <QTextDocument>
#include <QTextCursor>
#include <QBuffer>
#include <QRandomGenerator>
#if defined(Q_OS_WIN)
#  include <windows.h>
#endif

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
    setWindowIcon(AppIcons::appIcon(m_config.ui.appIcon));
    resize(1100, 700);

    ThemeLoader::apply(m_config.ui.theme);
    m_theme = ThemeLoader::load(m_config.ui.theme);
    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    setupToolbar();
    setupSidebar();
    setupNickPanel();
    setupChatArea();
    setupInputBar();
    connectModel();
    applyFontSizes();

    if (QSystemTrayIcon::isSystemTrayAvailable())
        m_tray = new TrayIcon(model, this);

    statusBar()->hide();

    QSettings settings("LinuxDojo", "UplinkIRC");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    if (settings.contains("nickSplitter"))
        m_chatSplitter->restoreState(settings.value("nickSplitter").toByteArray());
    if (settings.contains("sidebarWidth"))
        m_sidebarExpandedWidth = settings.value("sidebarWidth").toInt();

    QTimer::singleShot(0, this, [this]{
        const int total = m_mainSplitter->width();
        if (total > 0)
            m_mainSplitter->setSizes({m_sidebarExpandedWidth, total - m_sidebarExpandedWidth});
        if (m_topicLeft) m_topicLeft->setFixedWidth(m_sidebarExpandedWidth);
    });

    connect(m_mainSplitter, &QSplitter::splitterMoved, this, [this](int, int){
        const int w = m_mainSplitter->sizes().value(0);
        if (m_sidebarExpanded && w > 0)
            m_sidebarExpandedWidth = w;
        if (m_topicLeft) m_topicLeft->setFixedWidth(w);
    });

    connect(qApp, &QApplication::aboutToQuit, this, [this]{
        QSettings s("LinuxDojo", "UplinkIRC");
        s.setValue("geometry", saveGeometry());
        s.setValue("windowState", saveState());
        s.setValue("nickSplitter", m_chatSplitter->saveState());
        s.setValue("sidebarWidth", m_sidebarExpandedWidth);
    });
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
    tb->setContentsMargins(0, 0, 0, 0);
    if (tb->layout())
        tb->layout()->setContentsMargins(0, 0, 0, 0);
    tb->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tb, &QWidget::customContextMenuRequested, this, [](const QPoint&){});

    m_hamburger = new QToolButton;
    m_hamburger->setText("☰");
    m_hamburger->setFixedHeight(22);
    m_hamburger->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_hamburger, &QWidget::customContextMenuRequested, this, [](const QPoint&){});
    m_hamburger->setObjectName("hamburger");
    tb->hide();

    connect(m_hamburger, &QToolButton::clicked, this, [this]{
        auto *menu = new QMenu(m_hamburger);
        menu->setAttribute(Qt::WA_DeleteOnClose);

        menu->addAction(MenuIcons::about(), "About UplinkIRC", this, [this]{
            AboutDialog dlg(this);
            dlg.exec();
        });

        menu->addAction(MenuIcons::documentation(), "Documentation", this, [this]{
            if (!m_docsDialog)
                m_docsDialog = new DocsDialog(this);
            m_docsDialog->show();
            m_docsDialog->raise();
            m_docsDialog->activateWindow();
        });

        menu->addAction(MenuIcons::fontConfig(), "Preferences", this, [this]{
            if (!m_prefsDialog) {
                m_prefsDialog = new PreferencesDialog(m_config, this);
                connectPreferences();
            }
            m_prefsDialog->show();
            m_prefsDialog->raise();
            m_prefsDialog->activateWindow();
        });

        menu->addSeparator();

        menu->addAction(MenuIcons::servers(), "Open Config", this, [this]{
            QDesktopServices::openUrl(QUrl::fromLocalFile(Config::defaultPath()));
        });

        menu->addAction(MenuIcons::connStatus(), "Reload Config", this, [this]{
            m_config = Config::load(Config::defaultPath());
            m_model->syncServers(m_config.servers);

            m_showNickPrefix = m_config.ui.showNickPrefix;
            m_showTopic      = m_config.ui.showTopic;
            m_showEmojiBtn   = m_config.ui.showEmojiButton;

            ThemeLoader::apply(m_config.ui.theme);
            m_theme = ThemeLoader::load(m_config.ui.theme);
            if (m_chatView && m_theme.valid)
                m_chatView->document()->setDefaultStyleSheet(
                    QString("a { color: %1; text-decoration: underline; }").arg(m_theme.accent));

            applyAppIcon(m_config.ui.appIcon);

            QApplication::setFont(QFont(m_config.ui.fontFamily));
            applyFontSizes();

            if (m_topicDisplay)     m_topicDisplay->setVisible(m_showTopic);
            if (m_nickPrefix)       m_nickPrefix->setVisible(m_showNickPrefix);
            if (m_emojiBtn)         m_emojiBtn->setVisible(m_showEmojiBtn);
            if (m_typingLabel)      m_typingLabel->setVisible(m_config.ui.typingIndicator);
        });

        QPoint pos = m_hamburger->mapToGlobal(QPoint(0, m_hamburger->height()));
        menu->exec(pos);
    });
}

void MainWindow::connectPreferences()
{
    connect(m_prefsDialog, &PreferencesDialog::themeChanged, this, [this](const QString &name){
        m_config.ui.theme = name;
        ThemeLoader::apply(name);
        m_theme = ThemeLoader::load(name);
        if (m_chatView && m_theme.valid)
            m_chatView->document()->setDefaultStyleSheet(
                QString("a { color: %1; text-decoration: underline; }").arg(m_theme.accent));
        Config::save(m_config, Config::defaultPath());
    });

    connect(m_prefsDialog, &PreferencesDialog::fontConfigRequested, this, [this]{
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

    connect(m_prefsDialog, &PreferencesDialog::appIconChanged, this, [this](const QString &key){
        m_config.ui.appIcon = key;
        Config::save(m_config, Config::defaultPath());
        applyAppIcon(key);
    });

    connect(m_prefsDialog, &PreferencesDialog::topicBarToggled, this, [this](bool on){
        m_showTopic = on;
        m_config.ui.showTopic = on;
        m_topicDisplay->setVisible(on);
        Config::save(m_config, Config::defaultPath());
    });

    connect(m_prefsDialog, &PreferencesDialog::nickPrefixToggled, this, [this](bool on){
        m_showNickPrefix = on;
        m_config.ui.showNickPrefix = on;
        m_nickPrefix->setVisible(on);
        Config::save(m_config, Config::defaultPath());
    });

    connect(m_prefsDialog, &PreferencesDialog::emojiBtnToggled, this, [this](bool on){
        m_showEmojiBtn = on;
        m_config.ui.showEmojiButton = on;
        m_emojiBtn->setVisible(on);
        Config::save(m_config, Config::defaultPath());
    });

    connect(m_prefsDialog, &PreferencesDialog::typingIndicatorToggled, this, [this](bool on){
        m_config.ui.typingIndicator = on;
        Config::save(m_config, Config::defaultPath());
        if (!on) {
            m_typingOutTimer->stop();
            m_typingActive = false;
            m_typingNicks.clear();
            for (auto *t : std::as_const(m_typingNickTimers)) { t->stop(); t->deleteLater(); }
            m_typingNickTimers.clear();
            m_typingLabel->setVisible(false);
        } else {
            m_typingLabel->setText("");
            m_typingLabel->setVisible(true);
        }
    });


    connect(m_prefsDialog, &PreferencesDialog::notificationsToggled, this, [this](bool on){
        m_config.ui.notifications = on;
        Config::save(m_config, Config::defaultPath());
    });

    connect(m_prefsDialog, &PreferencesDialog::coloredNicksToggled, this, [this](bool on){
        m_config.ui.coloredNicks = on;
        Config::save(m_config, Config::defaultPath());
        if (!m_model->activeHost().isEmpty() && !m_model->activeChannel().isEmpty())
            refreshNickList(m_model->activeHost(), m_model->activeChannel());
    });

    connect(m_prefsDialog, &PreferencesDialog::nickBracketsChanged, this, [this](const QString &br){
        m_config.ui.nickBrackets = br;
        Config::save(m_config, Config::defaultPath());
    });

    connect(m_prefsDialog, &PreferencesDialog::manageServersRequested, this, [this]{
        ManageServersDialog dlg(m_config.servers, this);
        if (dlg.exec() != QDialog::Accepted) return;
        const QList<ServerConfig> updated = dlg.servers();
        for (const ServerConfig &old : m_config.servers) {
            const bool stillPresent = std::any_of(updated.begin(), updated.end(),
                [&](const ServerConfig &s){ return s.host == old.host; });
            if (!stillPresent)
                m_model->removeServer(old.host);
        }
        for (const ServerConfig &sc : updated) {
            const ServerConfig *existing = nullptr;
            for (const ServerConfig &old : m_config.servers)
                if (old.host == sc.host) { existing = &old; break; }
            if (!existing) {
                m_model->addServer(sc);
            } else if (*existing != sc) {
                m_model->updateServer(existing->host, sc);
            }
        }
        m_config.servers = updated;
        Config::save(m_config, Config::defaultPath());
    });

    connect(m_prefsDialog, &PreferencesDialog::aboutRequested, this, [this]{
        AboutDialog dlg(this);
        dlg.exec();
    });

    connect(m_prefsDialog, &PreferencesDialog::docsRequested, this, [this]{
        if (!m_docsDialog)
            m_docsDialog = new DocsDialog(this);
        m_docsDialog->show();
        m_docsDialog->raise();
        m_docsDialog->activateWindow();
    });
}

void MainWindow::applyAppIcon(const QString &choice)
{
    const QIcon icon = AppIcons::appIcon(choice);
    setWindowIcon(icon);
    if (m_tray) m_tray->setBaseIcon(icon);
}

static QIcon makeGearIcon(int angleDeg, const QColor &color)
{
    const int sz = 16;
    QPixmap pix(sz, sz);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.translate(sz / 2.0, sz / 2.0);
    p.rotate(angleDeg);
    p.translate(-sz / 2.0, -sz / 2.0);
    QFont f;
    f.setPixelSize(sz - 1);
    p.setFont(f);
    p.setPen(color);
    p.drawText(QRect(0, 0, sz, sz), Qt::AlignCenter, QStringLiteral("⚙"));
    return QIcon(pix);
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
        QFont f = makeFont(fs.toolbar * 2);
        f.setBold(false);
        m_hamburger->setFont(f);
    }
    if (m_appLabel) {
        QFont f = makeFont(fs.toolbar);
        f.setBold(true);
        m_appLabel->setFont(f);
    }
    if (m_sidebar) {
        m_sidebar->setFont(makeFont(fs.sidebar));
        // Update server header items with their dedicated font size
        for (int i = 0; i < m_sidebar->topLevelItemCount(); ++i) {
            auto *srv = m_sidebar->topLevelItem(i);
            QFont f = makeFont(fs.serverHeader);
            f.setBold(true);
            srv->setFont(0, f);
        }
    }
    if (m_chatView)       m_chatView->setFont(makeFont(fs.chat));
    if (m_nickList)       m_nickList->setFont(makeFont(fs.nickList));
    if (m_nickPanel)      m_nickPanel->setFont(makeFont(fs.nickDock));
    if (m_nickToggleBtn)
        m_nickToggleBtn->setIcon(
            makeGearIcon(0, m_nickToggleBtn->palette().color(QPalette::WindowText)));
    if (m_sidebarToggleBtn)
        m_sidebarToggleBtn->setIcon(
            makeGearIcon(0, m_sidebarToggleBtn->palette().color(QPalette::WindowText)));
    if (m_topicLabel)    m_topicLabel->setFont(makeFont(fs.topicBar));
    if (m_modesLabel)    m_modesLabel->setFont(makeFont(fs.topicBar));
    if (m_userInfoLabel) m_userInfoLabel->setFont(makeFont(fs.topicBar));
    if (m_nickPrefix)   m_nickPrefix->setFont(makeFont(fs.inputNick));
    if (m_input)        m_input->setFont(makeFont(fs.input));
    if (m_typingLabel) {
        QFont f = makeFont(fs.typing);
        f.setItalic(true);
        m_typingLabel->setFont(f);
    }
}

static QIcon makeConnectedIcon()
{
    QPixmap pm(10, 10);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor("#a6adc8"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(1, 1, 8, 8);
    return QIcon(pm);
}

void MainWindow::setupSidebar()
{
    m_sidebar = new QTreeWidget;
    m_sidebar->setHeaderHidden(true);
    m_sidebar->setRootIsDecorated(false);
    m_sidebar->setItemsExpandable(false);
    m_sidebar->setIndentation(8);
    m_sidebar->setMinimumWidth(112);
    m_sidebar->setObjectName("sidebar");

    connect(m_sidebar, &QTreeWidget::currentItemChanged,
            this, [this](QTreeWidgetItem *, QTreeWidgetItem *){ onSidebarSelectionChanged(); });
    m_sidebar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_sidebar, &QTreeWidget::customContextMenuRequested,
            this, &MainWindow::onSidebarContextMenu);

    m_sidebarToggleBtn = new QToolButton;
    m_sidebarToggleBtn->setFixedSize(22, 22);
    m_sidebarToggleBtn->setAutoRaise(true);
    m_sidebarToggleBtn->setObjectName("sidebarToggleBtn");
    m_sidebarToggleBtn->setToolTip(tr("Toggle channel list"));
    m_sidebarToggleBtn->setIcon(makeGearIcon(0, palette().color(QPalette::WindowText)));
    connect(m_sidebarToggleBtn, &QToolButton::clicked, this, [this]{
        m_sidebarExpanded = !m_sidebarExpanded;
        m_sidebar->setVisible(m_sidebarExpanded);
        const QList<int> sizes = m_mainSplitter->sizes();
        const int total = sizes[0] + sizes[1];
        if (m_sidebarExpanded) {
            m_mainSplitter->setSizes({m_sidebarExpandedWidth, total - m_sidebarExpandedWidth});
            if (m_topicLeft) m_topicLeft->setFixedWidth(m_sidebarExpandedWidth);
        } else {
            m_mainSplitter->setSizes({0, total});
            if (m_topicLeft) m_topicLeft->setFixedWidth(0);
        }
    });

    m_sidebarPanel = new QWidget;
    m_sidebarPanel->setObjectName("sidebarPanel");
    auto *vbox = new QVBoxLayout(m_sidebarPanel);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    vbox->addWidget(m_sidebar, 1);
}

void MainWindow::setupNickPanel()
{
    m_nickList = new QListWidget;
    m_nickList->setSpacing(0);
    m_nickList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_nickList, &QListWidget::customContextMenuRequested,
            this, &MainWindow::onNickListContextMenu);

    m_nickCountLabel = new QLabel(QStringLiteral("0 users"));
    m_nickCountLabel->setObjectName("nickCountLabel");
    m_nickCountLabel->setContentsMargins(4, 0, 4, 0);

    m_nickToggleBtn = new QToolButton;
    m_nickToggleBtn->setFixedSize(22, 22);
    m_nickToggleBtn->setAutoRaise(true);
    m_nickToggleBtn->setToolTip(tr("Toggle user list"));
    m_nickToggleBtn->setIcon(makeGearIcon(0, palette().color(QPalette::WindowText)));

    m_gearTimer = new QTimer(this);
    m_gearTimer->setInterval(16);
    connect(m_gearTimer, &QTimer::timeout, this, [this]{
        m_gearAngle += 12;
        m_nickToggleBtn->setIcon(
            makeGearIcon(m_gearAngle, m_nickToggleBtn->palette().color(QPalette::WindowText)));
        if (m_gearAngle < 360) return;

        m_gearTimer->stop();
        m_gearAngle = 0;
        m_nickExpanded = !m_nickExpanded;
        m_nickList->setVisible(m_nickExpanded);

        m_nickToggleBtn->setIcon(
            makeGearIcon(0, m_nickToggleBtn->palette().color(QPalette::WindowText)));
    });

    connect(m_nickToggleBtn, &QToolButton::clicked, this, [this]{
        if (m_gearTimer->isActive()) return;
        m_gearAngle = 0;
        m_gearTimer->start();
    });

    auto *header = new QWidget;
    header->setObjectName("nickPanelHeader");
    auto *hbox = new QHBoxLayout(header);
    hbox->setContentsMargins(2, 2, 2, 2);
    hbox->setSpacing(2);
    hbox->addWidget(m_nickToggleBtn);
    hbox->addWidget(m_nickCountLabel, 1);

    m_nickPanel = new QWidget;
    m_nickPanel->setObjectName("nickPanel");
    m_nickPanel->setMinimumWidth(24);
    auto *vbox = new QVBoxLayout(m_nickPanel);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(header);
    vbox->addWidget(m_nickList, 100);
    vbox->addStretch(1);
}

void MainWindow::setupChatArea()
{
    // Single full-width bar spanning above sidebar AND chat, with border below
    m_topicBar = new QWidget;
    m_topicBar->setObjectName("topicBar");
    auto *tHbox = new QHBoxLayout(m_topicBar);
    tHbox->setContentsMargins(0, 0, 0, 0);
    tHbox->setSpacing(0);

    // Left zone — mirrors sidebar width: hamburger left, gear right
    m_topicLeft = new QWidget;
    m_topicLeft->setObjectName("topicLeftZone");
    m_topicLeft->setFixedWidth(m_sidebarExpandedWidth);
    auto *tlHbox = new QHBoxLayout(m_topicLeft);
    tlHbox->setContentsMargins(0, 4, 4, 4);
    tlHbox->setSpacing(0);
    tlHbox->addWidget(m_hamburger);
    tlHbox->addStretch(1);
    tlHbox->addWidget(m_sidebarToggleBtn);

    // Right zone — mirrors chat area: signal bars + channel info
    auto *topicRight = new QWidget;
    topicRight->setObjectName("topicRightZone");
    auto *trHbox = new QHBoxLayout(topicRight);
    trHbox->setContentsMargins(8, 4, 8, 4);
    trHbox->setSpacing(6);

    m_topicLabel = new QLabel;
    m_topicLabel->setObjectName("channelLabel");
    m_modesLabel = new QLabel;
    m_modesLabel->setObjectName("modesLabel");
    m_userInfoLabel = new QLabel;
    m_userInfoLabel->setObjectName("userInfoLabel");
    m_signalBars = new SignalBars(topicRight);

    trHbox->addWidget(m_signalBars);
    trHbox->addWidget(m_topicLabel);
    trHbox->addWidget(m_userInfoLabel);
    trHbox->addWidget(m_modesLabel, 1);

    tHbox->addWidget(m_topicLeft);
    tHbox->addWidget(topicRight, 1);

    // Right content — topic display + chat + input (no info bar here)
    m_rightContent = new QWidget;
    auto *vbox     = new QVBoxLayout(m_rightContent);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    // Topic display — shown below info bar only when Show Topic is on
    m_topicDisplay = new QWidget;
    auto *tdHbox   = new QHBoxLayout(m_topicDisplay);
    tdHbox->setContentsMargins(8, 3, 8, 3);
    m_topicText = new QLabel;
    m_topicText->setObjectName("topicText");
    m_topicText->setWordWrap(true);
    m_topicText->setTextFormat(Qt::RichText);
    m_topicText->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_topicText->setOpenExternalLinks(true);
    tdHbox->addWidget(m_topicText);
    m_topicDisplay->setObjectName("topicDisplay");
    m_topicDisplay->setVisible(m_showTopic);
    vbox->addWidget(m_topicDisplay);

    // Chat view
    m_chatView = new QTextBrowser;
    m_chatView->setReadOnly(true);
    m_chatView->setLineWrapMode(QTextEdit::WidgetWidth);
    m_chatView->setOpenLinks(false);
    if (m_theme.valid)
        m_chatView->document()->setDefaultStyleSheet(
            QString("a { color: %1; text-decoration: underline; }").arg(m_theme.accent));
    connect(m_chatView, &QTextBrowser::anchorClicked,
            this, [](const QUrl &url){ QDesktopServices::openUrl(url); });

    m_linkPreview = new LinkPreview(this);

    m_chatView->viewport()->setMouseTracking(true);
    m_chatView->viewport()->installEventFilter(this);
    m_chatView->installEventFilter(this);

    connect(m_linkPreview, &LinkPreview::titleReady, this, [this](const QUrl &url, const QString &title){
        if (url.toString() != m_hoveredUrl) return;
        const QString display = title.length() > 80 ? title.left(79) + QChar(0x2026) : title;
        statusBar()->showMessage(display);
        QToolTip::showText(m_hoverGlobalPos, display, m_chatView->viewport());
    });

    connect(m_linkPreview, &LinkPreview::cardReady, this,
            [this](const QUrl &pageUrl, const QString &title, const QPixmap &thumbnail){
        const QString urlStr = pageUrl.toString();
        auto it = m_previewChannels.find(urlStr);
        if (it == m_previewChannels.end()) return;
        const QString host    = it->first;
        const QString channel = it->second;
        m_previewChannels.erase(it);

        auto *ch = m_model->channel(host, channel);
        if (!ch) return;

        QString imgHtml;
        if (!thumbnail.isNull()) {
            QByteArray imgData;
            QBuffer imgBuf(&imgData);
            imgBuf.open(QIODevice::WriteOnly);
            thumbnail.save(&imgBuf, "PNG");
            const QString dataUri = "data:image/png;base64,"
                                  + QString::fromLatin1(imgData.toBase64());
            imgHtml = QString("<td><img src=\"%1\" width=\"%2\" height=\"%3\"/></td>")
                .arg(dataUri).arg(thumbnail.width()).arg(thumbnail.height());
        }

        const QColor bg     = m_chatView->palette().color(QPalette::AlternateBase);
        const QColor border = m_chatView->palette().color(QPalette::Mid);
        const QColor fg     = m_chatView->palette().color(QPalette::Text);
        const QColor sub    = m_chatView->palette().color(QPalette::PlaceholderText);

        const QString titleEsc  = title.toHtmlEscaped().left(120);
        const QString domainEsc = pageUrl.host().toHtmlEscaped();

        const QString cardHtml = QString(
            "<table cellpadding=\"5\" cellspacing=\"0\" "
            "style=\"margin:1px 0 3px 20px;"
            "border-left:3px solid %1;"
            "background-color:%2\">"
            "<tr>%3"
            "<td valign=\"top\"%4>"
            "<span style=\"color:%5;font-weight:bold\">%6</span><br/>"
            "<span style=\"color:%7;font-size:8pt\">%8</span>"
            "</td></tr></table>")
            .arg(border.name(), bg.name(), imgHtml,
                 imgHtml.isEmpty() ? "" : " style=\"padding-left:6px\"",
                 fg.name(), titleEsc, sub.name(), domainEsc);

        ch->previews.insert(urlStr, cardHtml);

        if (host != m_model->activeHost() ||
            channel.toLower() != m_model->activeChannel().toLower())
            return;

        QScrollBar *sb = m_chatView->verticalScrollBar();
        const bool atBottom = sb->value() >= sb->maximum() - 4;

        m_chatView->append(cardHtml);

        if (atBottom)
            sb->setValue(sb->maximum());
    });

    m_chatSplitter = new QSplitter(Qt::Horizontal);
    m_chatSplitter->setHandleWidth(0);
    m_chatSplitter->addWidget(m_chatView);
    m_chatSplitter->addWidget(m_nickPanel);
    m_chatSplitter->setStretchFactor(0, 1);
    m_chatSplitter->setStretchFactor(1, 0);
    vbox->addWidget(m_chatSplitter, 1);

    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainSplitter->setHandleWidth(0);
    m_mainSplitter->addWidget(m_sidebarPanel);
    m_mainSplitter->addWidget(m_rightContent);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    auto *outer      = new QWidget;
    auto *outerVbox  = new QVBoxLayout(outer);
    outerVbox->setContentsMargins(0, 0, 0, 0);
    outerVbox->setSpacing(0);
    outerVbox->addWidget(m_topicBar);
    outerVbox->addWidget(m_mainSplitter, 1);

    setCentralWidget(outer);
}

void MainWindow::setupInputBar()
{
    auto *bar  = new QWidget;
    auto *hbox = new QHBoxLayout(bar);
    hbox->setContentsMargins(4, 3, 4, 8);
    hbox->setSpacing(4);

    m_nickPrefix = new QLabel;
    m_nickPrefix->setStyleSheet("font-weight: bold; padding-right: 4px;");
    m_nickPrefix->setVisible(m_showNickPrefix);

    m_input = new QLineEdit;
    m_input->setPlaceholderText("Type a message...");
    m_input->installEventFilter(this);

    m_emojiBtn = new QPushButton("😊");
    m_emojiBtn->setFixedSize(30, 30);
    m_emojiBtn->setStyleSheet("QPushButton { padding: 0; font-size: 16px; }");
    m_emojiBtn->setVisible(m_showEmojiBtn);
    m_emojiBtn->setToolTip("Emoji picker");

    hbox->addWidget(m_nickPrefix);
    hbox->addWidget(m_input, 1);
    hbox->addWidget(m_emojiBtn);

    bar->setObjectName("inputBar");

    m_typingLabel = new QLabel;
    m_typingLabel->setObjectName("typingLabel");
    m_typingLabel->setContentsMargins(8, 2, 8, 2);
    m_typingLabel->setVisible(m_config.ui.typingIndicator);

    auto *layout = qobject_cast<QVBoxLayout *>(m_rightContent->layout());
    layout->addWidget(m_typingLabel);
    layout->addWidget(bar);

    // Emoji picker popup
    m_emojiPicker = new EmojiPicker(this);
    connect(m_emojiPicker, &EmojiPicker::emojiSelected, this, [this](const QString &emoji){
        const int pos = m_input->cursorPosition();
        const QString text = m_input->text();
        m_input->setText(text.left(pos) + emoji + text.mid(pos));
        m_input->setCursorPosition(pos + emoji.length());
        m_input->setFocus();
    });
    connect(m_emojiBtn, &QPushButton::clicked, this, [this]{
        const QPoint anchor = m_emojiBtn->mapToGlobal(
            QPoint(m_emojiBtn->width(), m_emojiBtn->height()));
        m_emojiPicker->showAt(anchor);
    });

    // Emoji inline autocomplete list (child widget, no focus steal)
    m_emojiCompleter = new QListWidget(this);
    m_emojiCompleter->setObjectName("emojiCompleter");
    m_emojiCompleter->setFocusPolicy(Qt::NoFocus);
    m_emojiCompleter->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_emojiCompleter->setFrameShape(QFrame::StyledPanel);
    m_emojiCompleter->hide();
    connect(m_emojiCompleter, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        commitEmojiAutocomplete(m_emojiCompleter->row(item));
    });

    // Inactivity timer: sends typing=paused after 5s with no keypresses
    m_typingOutTimer = new QTimer(this);
    m_typingOutTimer->setSingleShot(true);
    m_typingOutTimer->setInterval(5000);
    connect(m_typingOutTimer, &QTimer::timeout, this, [this]{
        if (!m_config.ui.typingIndicator) return;
        const QString host = m_model->activeHost();
        const QString ch   = m_model->activeChannel();
        if (ch.isEmpty() || ch == "(server)") return;
        m_typingActive = false;
        m_model->sendTyping(host, ch, "paused");
    });

    connect(m_input, &QLineEdit::textChanged, this, [this](const QString &text){
        checkEmojiAutocomplete(text);
    });

    connect(m_input, &QLineEdit::textChanged, this, [this](const QString &text){
        if (!m_config.ui.typingIndicator) return;
        const QString host = m_model->activeHost();
        const QString ch   = m_model->activeChannel();
        if (ch.isEmpty() || ch == "(server)") return;
        if (!text.isEmpty()) {
            if (!m_typingActive) {
                m_typingActive = true;
                m_model->sendTyping(host, ch, "active");
            }
            m_typingOutTimer->start();
        } else {
            m_typingOutTimer->stop();
            if (m_typingActive) {
                m_typingActive = false;
                m_model->sendTyping(host, ch, "done");
            }
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

    connect(m_model, &SessionModel::pingRtt, this, [this](const QString &host, int ms){
        if (m_signalBars && host == m_model->activeHost())
            m_signalBars->setLatency(ms);
    });
    connect(m_model, &SessionModel::serverReconnecting, this, [this](const QString &host){
        if (m_signalBars && host == m_model->activeHost())
            m_signalBars->setState(SignalBars::State::Reconnecting);
    });
}

// ---------------------------------------------------------------------------
// Event filter — Tab completion + input history
// ---------------------------------------------------------------------------

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_chatView->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(event);
            const QString anchor = m_chatView->anchorAt(me->position().toPoint());
            if (anchor != m_hoveredUrl) {
                m_hoveredUrl = anchor;
                if (anchor.isEmpty()) {
                    QToolTip::hideText();
                    statusBar()->clearMessage();
                } else {
                    m_hoverGlobalPos = m_chatView->viewport()->mapToGlobal(
                        me->position().toPoint());
                    const QUrl url(anchor);
                    statusBar()->showMessage(url.host());
                    QToolTip::showText(m_hoverGlobalPos, url.host(),
                                       m_chatView->viewport());
                    m_linkPreview->fetch(url);
                }
            }
        } else if (event->type() == QEvent::Leave) {
            m_hoveredUrl.clear();
            QToolTip::hideText();
            statusBar()->clearMessage();
        }
        return false;
    }

    if (obj == m_chatView && event->type() == QEvent::Resize) {
        repositionTypingLabel();
        return false;
    }

    if (obj != m_input || event->type() != QEvent::KeyPress)
        return QMainWindow::eventFilter(obj, event);

    auto *ke = static_cast<QKeyEvent *>(event);

    // Emoji autocomplete navigation takes priority when popup is visible
    if (m_emojiCompleter->isVisible()) {
        if (ke->key() == Qt::Key_Escape) {
            hideEmojiAutocomplete();
            return true;
        }
        if (ke->key() == Qt::Key_Up) {
            const int cur = m_emojiCompleter->currentRow();
            m_emojiCompleter->setCurrentRow(qMax(0, cur - 1));
            return true;
        }
        if (ke->key() == Qt::Key_Down) {
            const int cur = m_emojiCompleter->currentRow();
            m_emojiCompleter->setCurrentRow(
                qMin(m_emojiCompleter->count() - 1, cur + 1));
            return true;
        }
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter ||
            ke->key() == Qt::Key_Tab) {
            const int row = m_emojiCompleter->currentRow();
            if (row >= 0) {
                commitEmojiAutocomplete(row);
                return true;
            }
        }
    }

    if (ke->key() == Qt::Key_Tab) {
        handleTabComplete();
        return true;
    }

    // Any non-Tab key resets nick completion cycle
    m_tabActive = false;
    m_tabCandidates.clear();

    if (ke->key() == Qt::Key_Up && !m_emojiCompleter->isVisible()) {
        handleHistoryUp();
        return true;
    }
    if (ke->key() == Qt::Key_Down && !m_emojiCompleter->isVisible()) {
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

        if (prefix.startsWith('/')) {
            static const QStringList commands = {
                "/away", "/back", "/ban", "/clear", "/ctcp",
                "/deop", "/devoice", "/invite", "/j", "/join",
                "/kick", "/me", "/mode", "/motd", "/msg",
                "/nick", "/notice", "/op", "/part", "/ping", "/time",
                "/quit", "/quote", "/raw", "/sysinfo", "/topic",
                "/unban", "/version", "/voice", "/whois",
            };
            for (const QString &cmd : commands)
                if (cmd.startsWith(prefix, Qt::CaseInsensitive))
                    m_tabCandidates << cmd;
        } else {
            auto *ch = m_model->channel(m_model->activeHost(), m_model->activeChannel());
            if (ch) {
                for (const auto &e : std::as_const(ch->nicks))
                    if (e.nick.startsWith(prefix, Qt::CaseInsensitive))
                        m_tabCandidates << e.nick;
            }
        }
    }

    if (m_tabCandidates.isEmpty()) return;

    const QString completed = m_tabCandidates[m_tabCandidateIndex];
    m_tabCandidateIndex = (m_tabCandidateIndex + 1) % m_tabCandidates.size();

    // Commands always get a space suffix; nicks get ": " at line start, " " otherwise
    QString suffix;
    if (pos == text.length())
        suffix = (wordStart == 0 && !completed.startsWith('/'))
            ? QStringLiteral(": ") : QStringLiteral(" ");

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
// Emoji inline autocomplete
// ---------------------------------------------------------------------------

void MainWindow::checkEmojiAutocomplete(const QString &text)
{
    const int cursorPos = m_input->cursorPosition();
    const QString before = text.left(cursorPos);

    // Auto-substitute a completed :shortcode: when the closing colon is just typed
    if (cursorPos > 0 && text[cursorPos - 1] == ':') {
        const QString beforeColon = before.chopped(1);  // drop the closing ':'
        const int openColon = beforeColon.lastIndexOf(':');
        if (openColon >= 0) {
            const QString code = beforeColon.mid(openColon + 1);
            static const QRegularExpression wordOnly(R"(^\w+$)");
            if (wordOnly.match(code).hasMatch()) {
                const QString emoji = emojiByCode().value(code);
                if (!emoji.isEmpty()) {
                    const QString newText = text.left(openColon) + emoji + text.mid(cursorPos);
                    m_input->setText(newText);
                    m_input->setCursorPosition(openColon + emoji.length());
                    hideEmojiAutocomplete();
                    return;
                }
            }
        }
    }

    // Find a bare :word pattern ending at cursor — minimum 1 char after colon
    const int colon = before.lastIndexOf(':');
    if (colon < 0) { hideEmojiAutocomplete(); return; }

    const QString word = before.mid(colon + 1);
    // Only trigger if word is all word-chars and at least 1 char
    static const QRegularExpression wordRe(R"(^\w+$)");
    if (word.isEmpty() || !wordRe.match(word).hasMatch()) {
        hideEmojiAutocomplete();
        return;
    }

    const auto matches = emojiMatching(word);
    if (matches.isEmpty()) { hideEmojiAutocomplete(); return; }

    m_emojiTriggerPos = colon;

    m_emojiCompleter->clear();
    const int shown = qMin(matches.size(), 8);
    for (int i = 0; i < shown; ++i) {
        const auto &e = matches[i];
        auto *item = new QListWidgetItem(e.ch + "  " + e.shortcode);
        item->setData(Qt::UserRole, e.ch);
        m_emojiCompleter->addItem(item);
    }
    m_emojiCompleter->setCurrentRow(0);

    // Size to fit content
    const int itemH  = m_emojiCompleter->sizeHintForRow(0) + 2;
    const int popupH = itemH * shown + 4;
    const int popupW = 220;

    // Position above the input bar in main-window coords
    const QPoint inputTL = m_input->mapTo(this, QPoint(0, 0));

    // Align left edge with colon position approximation using font metrics
    const int charW  = m_input->fontMetrics().averageCharWidth();
    const int colonX = m_input->contentsMargins().left() + colon * charW;
    const QPoint colonLocal = m_input->mapTo(this, QPoint(colonX, 0));

    int px = qMax(inputTL.x(), colonLocal.x());
    int py = inputTL.y() - popupH - 2;
    if (py < 0) py = inputTL.y() + m_input->height() + 2;

    // Clamp horizontally
    if (px + popupW > width()) px = width() - popupW;

    m_emojiCompleter->setGeometry(px, py, popupW, popupH);
    m_emojiCompleter->show();
    m_emojiCompleter->raise();
}

void MainWindow::commitEmojiAutocomplete(int row)
{
    if (row < 0 || row >= m_emojiCompleter->count()) return;

    const QString emoji = m_emojiCompleter->item(row)->data(Qt::UserRole).toString();
    hideEmojiAutocomplete();

    // Replace :word (from trigger pos to cursor) with the emoji
    QString text    = m_input->text();
    const int start = m_emojiTriggerPos;          // position of ':'
    const int end   = m_input->cursorPosition();  // current cursor
    m_input->setText(text.left(start) + emoji + text.mid(end));
    m_input->setCursorPosition(start + emoji.length());
    m_input->setFocus();
}

void MainWindow::hideEmojiAutocomplete()
{
    m_emojiCompleter->hide();
    m_emojiCompleter->clear();
    m_emojiTriggerPos = -1;
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
    item->setText(0, label.toUpper());
    item->setData(0, Qt::UserRole,     host);
    item->setData(0, Qt::UserRole + 1, QString("(server)"));
    item->setExpanded(true);
    // Section header style — not selectable
    item->setFlags(Qt::ItemIsEnabled);
    QFont f(m_config.ui.fontFamily, m_config.ui.fontSizes.serverHeader);
    f.setBold(true);
    item->setFont(0, f);
    item->setForeground(0, QColor("#6c7086"));

    if (m_signalBars && (m_model->activeHost().isEmpty() || host == m_model->activeHost()))
        m_signalBars->setState(SignalBars::State::Connecting);
}

void MainWindow::onServerConnected(const QString &host)
{
    auto *item = findServerItem(host);
    if (item)
        item->setIcon(0, makeConnectedIcon());
    if (m_signalBars && host == m_model->activeHost())
        m_signalBars->setState(SignalBars::State::Connected);
}

void MainWindow::onServerDisconnected(const QString &host)
{
    auto *item = findServerItem(host);
    if (item)
        item->setIcon(0, QIcon());
    if (m_signalBars && host == m_model->activeHost())
        m_signalBars->setState(SignalBars::State::Disconnected);
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

    m_sidebar->setCurrentItem(item);
    switchToChannel(host, channel);
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
        appendMessage(msg, true);
    }

    if (m_config.ui.notifications && m_tray && !isActiveWindow()
        && (msg.type == MessageType::Privmsg || msg.type == MessageType::Action))
    {
        const QString myNick = m_model->selfNick(host);
        const bool isPM = !channel.startsWith('#') && !channel.startsWith('&');
        const bool isMention = !isPM && !myNick.isEmpty()
                               && msg.text.contains(myNick, Qt::CaseInsensitive);
        if (isPM || isMention)
            m_tray->setNotify(true);
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
    refreshTopicBar(host, channel);
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
    if (channel != "(server)") {
        if (count > 0 && m_model->hasMention(host, channel)) {
            item->setText(0, "💡 " + label);
            item->setForeground(0, QColor("red"));
        } else if (count > 0) {
            item->setText(0, "🔥 " + label);
            item->setData(0, Qt::ForegroundRole, QVariant());
        } else {
            item->setText(0, label);
            item->setData(0, Qt::ForegroundRole, QVariant());
        }
    }
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

void MainWindow::repositionTypingLabel()
{
    // no-op: typing label is now a layout row, not an overlay
}

void MainWindow::updateTypingLabel()
{
    const QString key = m_model->activeHost() + "|" + m_model->activeChannel();
    const QSet<QString> &typers = m_typingNicks.value(key);

    if (!m_config.ui.typingIndicator) {
        m_typingLabel->setVisible(false);
        return;
    }

    if (typers.isEmpty()) {
        m_typingLabel->setText("");
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

static QString sysinfoKernel()
{
    QProcess p;
    p.start("uname", {"-r"});
    p.waitForFinished(2000);
    const QString out = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    return out.isEmpty() ? "Unknown" : out;
}

static QString sysinfoOS()
{
#if defined(Q_OS_LINUX)
    QString name, buildId, versionId;
    QFile f("/etc/os-release");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        const auto strip = [](QString s) {
            if (s.startsWith('"') && s.endsWith('"'))
                s = s.mid(1, s.length() - 2);
            return s;
        };
        while (!in.atEnd()) {
            const QString line = in.readLine();
            if (line.startsWith("NAME=") && name.isEmpty())
                name = strip(line.mid(5));
            else if (line.startsWith("BUILD_ID=") && buildId.isEmpty())
                buildId = strip(line.mid(9));
            else if (line.startsWith("VERSION_ID=") && versionId.isEmpty())
                versionId = strip(line.mid(11));
        }
    }
    if (name.isEmpty()) name = "Linux";
    const QString ver = !buildId.isEmpty() ? buildId : versionId;
    const QString distro = ver.isEmpty() ? name : name + " " + ver;
    return QString("Linux (%1) (%2)").arg(distro, sysinfoKernel());
#elif defined(Q_OS_FREEBSD)
    QProcess p;
    p.start("uname", {"-s"});
    p.waitForFinished(2000);
    return QString("%1 (%2)").arg(QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed(),
                                  sysinfoKernel());
#else
    return QSysInfo::prettyProductName();
#endif
}

static QString sysinfoCPU()
{
#if defined(Q_OS_LINUX)
    QFile f("/proc/cpuinfo");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            const QString line = in.readLine();
            if (line.startsWith("model name")) {
                const int colon = line.indexOf(':');
                if (colon != -1)
                    return line.mid(colon + 1).trimmed();
            }
        }
    }
    return QSysInfo::currentCpuArchitecture();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_DARWIN)
    QProcess p;
    p.start("sysctl", {"-n", "hw.model"});
    p.waitForFinished(2000);
    const QString model = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    return model.isEmpty() ? "Unknown" : model;
#elif defined(Q_OS_WIN)
    QSettings reg(
        "HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        QSettings::NativeFormat);
    const QString name = reg.value("ProcessorNameString").toString().trimmed();
    return name.isEmpty() ? QSysInfo::currentCpuArchitecture() : name;
#else
    return QSysInfo::currentCpuArchitecture();
#endif
}

static QString sysinfoMEM()
{
#if defined(Q_OS_LINUX)
    QFile f("/proc/meminfo");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            const QStringList parts = in.readLine().split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2 && parts[0] == "MemTotal:") {
                const quint64 kb = parts[1].toULongLong();
                return QString("%1 GB").arg((kb + 512 * 1024) / (1024 * 1024));
            }
        }
    }
    return "Unknown";
#elif defined(Q_OS_FREEBSD)
    QProcess p;
    p.start("sysctl", {"-n", "hw.physmem"});
    p.waitForFinished(2000);
    const quint64 total = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed().toULongLong();
    if (total > 0)
        return QString("%1 GB").arg((total + 512ULL*1024*1024) / (1024ULL*1024*1024));
    return "Unknown";
#elif defined(Q_OS_WIN)
    MEMORYSTATUSEX ms{};
    ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms))
        return QString("%1 GB").arg((ms.ullTotalPhys + 512ULL*1024*1024) / (1024ULL*1024*1024));
    return "Unknown";
#else
    return "Unknown";
#endif
}

static QString sysinfoGPU()
{
#if defined(Q_OS_LINUX)
    // Try vulkaninfo first — gives device name + renderer string
    QProcess vk;
    vk.start("vulkaninfo", {"--summary"});
    if (vk.waitForFinished(3000) && vk.exitCode() == 0) {
        const QString out = QString::fromLocal8Bit(vk.readAllStandardOutput());
        QString deviceName, driverInfo;
        for (const QString &line : out.split('\n')) {
            const QString t = line.trimmed();
            if (t.startsWith("deviceName") && deviceName.isEmpty())
                deviceName = t.section('=', 1).trimmed();
            else if (t.startsWith("driverInfo") && driverInfo.isEmpty())
                driverInfo = t.section('=', 1).trimmed();
        }
        if (!deviceName.isEmpty()) {
            // driverInfo e.g. "Mesa 24.3.4 (RADV STRIX1)" → extract last parens
            const int lp = driverInfo.lastIndexOf('(');
            const int rp = driverInfo.lastIndexOf(')');
            if (lp != -1 && rp > lp)
                return QString("%1 (%2) (Vulkan)").arg(deviceName, driverInfo.mid(lp + 1, rp - lp - 1));
            return deviceName + " (Vulkan)";
        }
    }
    // Fallback: lspci
    QProcess lp;
    lp.start("lspci", {});
    if (lp.waitForFinished(2000)) {
        const QString out = QString::fromLocal8Bit(lp.readAllStandardOutput());
        for (const QString &line : out.split('\n')) {
            if (line.contains("VGA", Qt::CaseInsensitive) ||
                line.contains("3D controller", Qt::CaseInsensitive) ||
                line.contains("Display controller", Qt::CaseInsensitive)) {
                // "06:00.0 VGA compatible controller: AMD ... [Radeon 890M] (rev c8)"
                const int c2 = line.indexOf(':', line.indexOf(':') + 1);
                if (c2 != -1)
                    return line.mid(c2 + 1).section('(', 0, 0).trimmed();
            }
        }
    }
    return "Unknown";
#elif defined(Q_OS_WIN)
    QProcess p;
    p.start("powershell", {"-NoProfile", "-Command",
        "(Get-CimInstance Win32_VideoController | Select-Object -First 1).Name"});
    if (p.waitForFinished(5000)) {
        const QString out = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
        if (!out.isEmpty()) return out;
    }
    return "Unknown";
#else
    return "Unknown";
#endif
}

static QString sysinfoUptime()
{
#if defined(Q_OS_LINUX)
    QFile f("/proc/uptime");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const quint64 s = static_cast<quint64>(
            QString::fromLocal8Bit(f.readLine()).section(' ', 0, 0).toDouble());
        const quint64 days    = s / 86400;
        const quint64 hours   = (s % 86400) / 3600;
        const quint64 minutes = (s % 3600) / 60;
        const quint64 seconds = s % 60;
        QStringList parts;
        if (days > 0)    parts << QString("%1 day%2").arg(days).arg(days != 1 ? "s" : "");
        if (hours > 0)   parts << QString("%1 hour%2").arg(hours).arg(hours != 1 ? "s" : "");
        if (minutes > 0) parts << QString("%1 minute%2").arg(minutes).arg(minutes != 1 ? "s" : "");
        parts << QString("%1 second%2").arg(seconds).arg(seconds != 1 ? "s" : "");
        return parts.join(' ') + " ago";
    }
    return "Unknown";
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_DARWIN)
    QProcess p;
    p.start("sysctl", {"-n", "kern.boottime"});
    p.waitForFinished(2000);
    // Parse "{ sec = N, usec = N } date" — extract sec and compute elapsed
    const QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
    const int eq = out.indexOf("sec = ");
    const int cm = out.indexOf(',', eq);
    if (eq != -1 && cm != -1) {
        const quint64 bootSec = out.mid(eq + 6, cm - eq - 6).trimmed().toULongLong();
        const quint64 now = QDateTime::currentSecsSinceEpoch();
        const quint64 s   = now > bootSec ? now - bootSec : 0;
        const quint64 days    = s / 86400;
        const quint64 hours   = (s % 86400) / 3600;
        const quint64 minutes = (s % 3600) / 60;
        const quint64 seconds = s % 60;
        QStringList parts;
        if (days > 0)    parts << QString("%1 day%2").arg(days).arg(days != 1 ? "s" : "");
        if (hours > 0)   parts << QString("%1 hour%2").arg(hours).arg(hours != 1 ? "s" : "");
        if (minutes > 0) parts << QString("%1 minute%2").arg(minutes).arg(minutes != 1 ? "s" : "");
        parts << QString("%1 second%2").arg(seconds).arg(seconds != 1 ? "s" : "");
        return parts.join(' ') + " ago";
    }
    return "Unknown";
#elif defined(Q_OS_WIN)
    const quint64 s = GetTickCount64() / 1000;
    const quint64 days    = s / 86400;
    const quint64 hours   = (s % 86400) / 3600;
    const quint64 minutes = (s % 3600) / 60;
    const quint64 seconds = s % 60;
    QStringList parts;
    if (days > 0)    parts << QString("%1 day%2").arg(days).arg(days != 1 ? "s" : "");
    if (hours > 0)   parts << QString("%1 hour%2").arg(hours).arg(hours != 1 ? "s" : "");
    if (minutes > 0) parts << QString("%1 minute%2").arg(minutes).arg(minutes != 1 ? "s" : "");
    parts << QString("%1 second%2").arg(seconds).arg(seconds != 1 ? "s" : "");
    return parts.join(' ') + " ago";
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
    hideEmojiAutocomplete();

    m_input->clear();

    const QString host    = m_model->activeHost();
    const QString channel = m_model->activeChannel();
    if (host.isEmpty() || channel.isEmpty()) return;

    // Stop typing notification on send
    m_typingOutTimer->stop();
    if (m_typingActive && m_config.ui.typingIndicator && channel != "(server)") {
        m_typingActive = false;
        m_model->sendTyping(host, channel, "done");
    }

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
            const QString target = args.section(' ', 0, 0);
            const QString text   = args.section(' ', 1);
            m_model->sendMessage(host, target, text);
            if (!target.startsWith('#') && !target.startsWith('&'))
                switchToChannel(host, target);
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
            QString topicTarget = channel;
            QString topicText   = args;
            if (args.startsWith('#') || args.startsWith('&')) {
                topicTarget = args.section(' ', 0, 0);
                topicText   = args.section(' ', 1);
            }
            if (topicText.isEmpty())
                m_model->sendRaw(host, "TOPIC " + topicTarget);
            else
                m_model->sendRaw(host, "TOPIC " + topicTarget + " :" + topicText);
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
            const QString info = QString("OS: %1 CPU: %2 MEM: %3 GPU: %4 UP: %5")
                .arg(sysinfoOS(), sysinfoCPU(), sysinfoMEM(), sysinfoGPU(), sysinfoUptime());
            m_model->sendMessage(host, channel, info);
        } else if (cmd == "/j") {
            m_model->sendJoin(host, args.section(' ', 0, 0), args.section(' ', 1, 1));
        } else if (cmd == "/ping") {
            const QString nick = args.trimmed().section(' ', 0, 0);
            if (!nick.isEmpty()) {
                const QString ts = QString::number(QDateTime::currentMSecsSinceEpoch());
                m_model->sendRaw(host, "PRIVMSG " + nick + " :\x01PING " + ts + "\x01");
                m_model->localMessage(host, channel, "Pinged " + nick);
            }
        } else if (cmd == "/time") {
            const QString nick = args.trimmed().section(' ', 0, 0);
            if (!nick.isEmpty()) {
                m_model->sendRaw(host, "PRIVMSG " + nick + " :\x01TIME\x01");
                m_model->localMessage(host, channel, "Querying time for " + nick);
            }
        } else if (cmd == "/invite") {
            const QString nick = args.section(' ', 0, 0);
            const QString chan = args.section(' ', 1, 1);
            if (!nick.isEmpty())
                m_model->sendRaw(host, "INVITE " + nick + " " + (chan.isEmpty() ? channel : chan));
        } else if (cmd == "/mode") {
            if (!args.isEmpty())
                m_model->sendRaw(host, "MODE " + args);
        } else if (cmd == "/op") {
            const QString nick = args.trimmed().section(' ', 0, 0);
            if (!nick.isEmpty())
                m_model->sendRaw(host, "MODE " + channel + " +o " + nick);
        } else if (cmd == "/deop") {
            const QString nick = args.trimmed().section(' ', 0, 0);
            if (!nick.isEmpty())
                m_model->sendRaw(host, "MODE " + channel + " -o " + nick);
        } else if (cmd == "/voice") {
            const QString nick = args.trimmed().section(' ', 0, 0);
            if (!nick.isEmpty())
                m_model->sendRaw(host, "MODE " + channel + " +v " + nick);
        } else if (cmd == "/devoice") {
            const QString nick = args.trimmed().section(' ', 0, 0);
            if (!nick.isEmpty())
                m_model->sendRaw(host, "MODE " + channel + " -v " + nick);
        } else if (cmd == "/ban") {
            const QString mask = args.trimmed().section(' ', 0, 0);
            if (!mask.isEmpty())
                m_model->sendRaw(host, "MODE " + channel + " +b " + mask);
        } else if (cmd == "/unban") {
            const QString mask = args.trimmed().section(' ', 0, 0);
            if (!mask.isEmpty())
                m_model->sendRaw(host, "MODE " + channel + " -b " + mask);
        } else if (cmd == "/clear") {
            if (m_chatView) m_chatView->clear();
        } else if (cmd == "/help") {
            const QStringList lines = {
                "Available commands:",
                "  /join <channel> [key]       — join a channel",
                "  /j <channel> [key]          — alias for /join",
                "  /part [message]             — leave the current channel",
                "  /nick <newnick>             — change your nick",
                "  /me <action>                — send an action (/me waves)",
                "  /msg <target> <message>     — send a private message",
                "  /notice <target> <message>  — send a NOTICE",
                "  /topic [text]               — show or set the channel topic",
                "  /kick <nick> [reason]       — kick a user",
                "  /invite <nick> [#channel]   — invite a user to a channel",
                "  /mode <target> <flags>      — set channel or user modes",
                "  /op <nick>                  — give op (+o)",
                "  /deop <nick>                — remove op (-o)",
                "  /voice <nick>               — give voice (+v)",
                "  /devoice <nick>             — remove voice (-v)",
                "  /ban <mask>                 — ban a mask (+b)",
                "  /unban <mask>               — remove a ban (-b)",
                "  /ping <nick>                — CTCP PING a user",
                "  /time <nick>                — query a user's local time",
                "  /away [message]             — set away status",
                "  /back                       — clear away status",
                "  /whois <nick>               — request WHOIS info",
                "  /motd [server]              — request the MOTD",
                "  /version [nick]             — request VERSION (nick optional)",
                "  /ctcp <target> <cmd> [args] — send a CTCP request",
                "  /sysinfo                    — post client/system info to channel",
                "  /clear                      — clear the chat buffer",
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

    // Substitute any remaining :shortcode: patterns before sending
    static const QRegularExpression shortcodeRe(R"(:(\w+):)");
    QString outText = text;
    int offset = 0;
    auto it = shortcodeRe.globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        const QString emoji = emojiByCode().value(m.captured(1));
        if (!emoji.isEmpty()) {
            outText.replace(m.capturedStart() + offset, m.capturedLength(), emoji);
            offset += emoji.length() - m.capturedLength();
        }
    }

    m_model->sendMessage(host, channel, outText);
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

    if (m_signalBars) {
        auto *sess = m_model->session(host);
        if (!sess)
            m_signalBars->setState(SignalBars::State::None);
        else if (sess->connected)
            m_signalBars->setState(SignalBars::State::Connected);
        else
            m_signalBars->setState(SignalBars::State::Disconnected);
    }

    setWindowTitle("UplinkIRC — " + channel + " @ " + host);
    updateTypingLabel();
}

void MainWindow::onSidebarContextMenu(const QPoint &pos)
{
    auto *item = m_sidebar->itemAt(pos);
    if (!item) return;

    const QString host    = item->data(0, Qt::UserRole).toString();
    const QString channel = item->data(0, Qt::UserRole + 1).toString();

    QMenu menu(this);

    if (channel == "(server)") {
        auto *sess = m_model->session(host);
        if (sess && sess->connected) {
            menu.addAction("Disconnect", this, [this, host]{
                if (auto *cl = m_model->clientFor(host))
                    cl->quit();
            });
        } else {
            menu.addAction("Reconnect", this, [this, host]{
                auto *cl = m_model->clientFor(host);
                if (!cl) return;
                for (const auto &sc : std::as_const(m_config.servers)) {
                    if (sc.host == host) { cl->connectToServer(sc); break; }
                }
            });
        }
    } else if (channel.startsWith('#') || channel.startsWith('&')) {
        menu.addAction("Rejoin", this, [this, host, channel]{
            m_model->sendPart(host, channel);
            QTimer::singleShot(500, this, [this, host, channel]{
                m_model->sendJoin(host, channel);
            });
        });
        menu.addAction("Leave", this, [this, host, channel]{
            m_model->sendPart(host, channel);
        });
        menu.addAction("Close", this, [this, host, channel]{
            m_model->closeBuffer(host, channel);
        });
    } else if (!channel.isEmpty() && channel != "(server)") {
        // PM / user query
        menu.addAction("Close Query", this, [this, host, channel]{
            m_model->closeBuffer(host, channel);
        });
    }

    if (!menu.actions().isEmpty())
        menu.exec(m_sidebar->viewport()->mapToGlobal(pos));
}

void MainWindow::onNickListContextMenu(const QPoint &pos)
{
    auto *item = m_nickList->itemAt(pos);
    if (!item) return;

    const QString nick    = item->data(Qt::UserRole).toString();
    const QString host    = m_model->activeHost();
    const QString channel = m_model->activeChannel();
    if (nick.isEmpty() || host.isEmpty()) return;

    QMenu menu(this);

    QAction *title = menu.addAction(nick);
    title->setEnabled(false);
    QFont tf = title->font();
    tf.setBold(true);
    title->setFont(tf);
    menu.addSeparator();

    connect(menu.addAction("Message"), &QAction::triggered, this, [this, host, nick]{
        m_model->openPM(host, nick);
        switchToChannel(host, nick);
        if (m_input) m_input->setFocus();
    });

    QAction *fileAct = menu.addAction("Send File");
    fileAct->setEnabled(false);  // DCC not yet implemented

    connect(menu.addAction("Whois"), &QAction::triggered, this, [this, host, nick]{
        m_model->sendRaw(host, "WHOIS " + nick);
    });

    connect(menu.addAction("Give Op"), &QAction::triggered, this, [this, host, channel, nick]{
        if (!channel.isEmpty() && channel != "(server)")
            m_model->sendRaw(host, "MODE " + channel + " +o " + nick);
    });

    connect(menu.addAction("Give Voice"), &QAction::triggered, this, [this, host, channel, nick]{
        if (!channel.isEmpty() && channel != "(server)")
            m_model->sendRaw(host, "MODE " + channel + " +v " + nick);
    });

    connect(menu.addAction("Version"), &QAction::triggered, this, [this, host, nick]{
        m_model->sendRaw(host, "PRIVMSG " + nick + " :\x01VERSION\x01");
    });

    menu.exec(m_nickList->mapToGlobal(pos));
}

void MainWindow::refreshChatView(const QString &host, const QString &channel)
{
    m_chatView->clear();
    auto *ch = m_model->channel(host, channel);
    if (!ch) return;

    static const QRegularExpression urlRe(
        R"(https?://[^\s<>"]+)",
        QRegularExpression::CaseInsensitiveOption);

    for (const auto &msg : std::as_const(ch->messages)) {
        appendMessage(msg);
        if (!ch->previews.isEmpty() &&
            (msg.type == MessageType::Privmsg ||
             msg.type == MessageType::Action  ||
             msg.type == MessageType::Notice)) {
            auto it = urlRe.globalMatch(msg.text);
            while (it.hasNext()) {
                const QString urlStr = it.next().captured(0);
                const auto p = ch->previews.constFind(urlStr);
                if (p != ch->previews.constEnd())
                    m_chatView->append(p.value());
            }
        }
    }
}

void MainWindow::refreshNickList(const QString &host, const QString &channel)
{
    m_nickList->clear();
    auto *ch   = m_model->channel(host, channel);
    if (!ch) return;
    auto *sess = m_model->session(host);

    for (const auto &e : std::as_const(ch->nicks)) {
        const bool isBot = (ch->botNicks.contains(e.nick.toLower()))
                        || (sess && sess->botNicks.contains(e.nick.toLower()));
        if (isBot && !m_botIcons.contains(e.nick.toLower()))
            m_botIcons[e.nick.toLower()] = QRandomGenerator::global()->bounded(2)
                                           ? QStringLiteral("🤖") : QStringLiteral("👾");
        const QString label = isBot
            ? e.display() + " " + m_botIcons[e.nick.toLower()]
            : e.display();
        auto *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, e.nick);
        if (m_config.ui.coloredNicks)
            item->setForeground(nickColor(e.nick));
        m_nickList->addItem(item);
    }

    if (m_nickCountLabel)
        m_nickCountLabel->setText(QString::number(ch->nicks.size()) + " users");
}

static QString linkifyTopic(const QString &text)
{
    static const QRegularExpression urlRe(
        R"((https?://[^\s<>"]+))",
        QRegularExpression::CaseInsensitiveOption);
    QString result = text.toHtmlEscaped();
    result.replace(urlRe, R"(<a href="\1">\1</a>)");
    return result;
}

static QString linkifyHtml(const QString &html)
{
    static const QRegularExpression urlRe(
        R"((https?://[^\s<>"]+))",
        QRegularExpression::CaseInsensitiveOption);
    QString result = html;
    result.replace(urlRe, R"(<a href="\1">\1</a>)");
    return result;
}

void MainWindow::refreshTopicBar(const QString &host, const QString &channel)
{
    auto *ch = m_model->channel(host, channel);

    QString serverName = host;
    for (const auto &sc : std::as_const(m_config.servers))
        if (sc.host == host && !sc.name.isEmpty()) { serverName = sc.name; break; }

    m_modesLabel->clear();

    if (channel == "(server)") {
        m_topicLabel->setText(serverName);
        m_userInfoLabel->clear();
        if (m_topicText) m_topicText->clear();
    } else {
        const QString modes   = ch ? ch->modes : QString();
        const QString modeStr = modes.isEmpty() ? QString() : " (" + modes + ")";
        m_topicLabel->setText(channel + modeStr);

        const int    userCount = ch ? ch->nicks.size() : 0;
        m_userInfoLabel->setText(
            QString("* %1 — %2 user%3").arg(serverName).arg(userCount).arg(userCount != 1 ? "s" : ""));

        if (m_topicText)
            m_topicText->setText(linkifyTopic(ch ? ch->topic : QString()));
    }
}

void MainWindow::appendMessage(const Message &msg, bool autoPreview)
{
    m_chatView->append(formatMessage(msg));

    if (autoPreview &&
        (msg.type == MessageType::Privmsg ||
         msg.type == MessageType::Action  ||
         msg.type == MessageType::Notice)) {
        static const QRegularExpression urlRe(
            R"(https?://[^\s<>"]+)",
            QRegularExpression::CaseInsensitiveOption);
        auto it = urlRe.globalMatch(msg.text);
        while (it.hasNext()) {
            const QString urlStr = it.next().captured(0);
            if (!m_previewChannels.contains(urlStr)) {
                m_previewChannels.insert(urlStr,
                    {m_model->activeHost(), m_model->activeChannel()});
                m_linkPreview->fetch(QUrl(urlStr));
            }
        }
    }

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

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange && isActiveWindow() && m_tray)
        m_tray->setNotify(false);
    QMainWindow::changeEvent(event);
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
    const QDateTime local = msg.timestamp.toLocalTime();
    const bool sameDay = local.date() == QDate::currentDate();
    const QString ts = sameDay
        ? local.toString("hh:mm")
        : local.toString("MM/dd hh:mm");

    auto wrap = [](const QString &color, const QString &text) {
        return QString("<span style='color:%1'>%2</span>").arg(color, text.toHtmlEscaped());
    };

    QString html;
    switch (msg.type) {
    case MessageType::Privmsg: {
        const QString color = m_config.ui.coloredNicks
            ? nickColor(msg.nick).name()
            : (m_theme.valid ? m_theme.text : QStringLiteral("#cccccc"));
        const QString &br = m_config.ui.nickBrackets;
        QString nickOpen, nickClose;
        if (!br.isEmpty()) {
            if (br.length() % 2 == 0) {
                nickOpen  = br.left(br.length() / 2).toHtmlEscaped();
                nickClose = br.mid(br.length() / 2).toHtmlEscaped();
            } else {
                nickOpen  = QString(br.front()).toHtmlEscaped();
                nickClose = QString(br.back()).toHtmlEscaped();
            }
        }
        const QString nickDisplay = nickOpen + msg.nick.toHtmlEscaped() + nickClose;
        QString textHtml = linkifyHtml(ircToHtml(msg.text));
        const QString sn = m_model->selfNick(m_model->activeHost());
        if (!sn.isEmpty()) {
            QRegularExpression snRe("(\\b" + QRegularExpression::escape(sn) + "\\b)",
                                    QRegularExpression::CaseInsensitiveOption);
            textHtml.replace(snRe, "<span style='color:red;font-weight:bold'>\\1</span>");
        }
        html = QString("<span style='color:gray'>%1</span> "
                       "<b style='color:%2'>%3</b> %4")
            .arg(ts, color, nickDisplay, textHtml);
        break;
    }
    case MessageType::Action:
        html = QString("<span style='color:gray'>%1</span> "
                       "<i>* %2 %3</i>")
            .arg(ts, msg.nick.toHtmlEscaped(), linkifyHtml(ircToHtml(msg.text)));
        break;

    case MessageType::Notice:
        html = QString("<span style='color:gray'>%1</span> "
                       "<span style='color:#cc8800'>-%2- %3</span>")
            .arg(ts, msg.nick.toHtmlEscaped(), linkifyHtml(ircToHtml(msg.text)));
        break;

    case MessageType::Join:
        html = wrap("seagreen",  ts + " → " + msg.text); break;
    case MessageType::Part:
        html = wrap("firebrick", ts + " ← " + msg.text); break;
    case MessageType::Quit:
        html = wrap("firebrick", ts + " ✕ " + msg.text); break;
    case MessageType::Nick:
        html = wrap("steelblue", ts + " ~ "  + msg.text); break;
    case MessageType::Kick:
        html = wrap("firebrick", ts + " ✕ " + msg.text); break;
    case MessageType::Topic:
        html = wrap("steelblue", ts + " ⦁ Topic: " + msg.text); break;
    case MessageType::Error:
        html = wrap("red",       ts + " !! " + msg.text); break;
    case MessageType::Server:
    default:
        html = wrap("gray",      ts + " * "  + msg.text); break;
    }

    if (msg.isHistory)
        html = "<span style='opacity:0.55'>" + html + "</span>";

    return html;
}
