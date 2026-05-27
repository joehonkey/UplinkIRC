#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QColor>
#include "model/sessionmodel.h"
#include "config/config.h"

class TrayIcon;
class DocsDialog;

class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;
class QLineEdit;
class QLabel;
class QPushButton;
class QDockWidget;
class QListWidget;
class QToolButton;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SessionModel *model, const Config &cfg, QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Model → UI
    void onServerAdded      (const QString &host);
    void onServerConnected  (const QString &host);
    void onServerDisconnected(const QString &host);
    void onChannelAdded     (const QString &host, const QString &channel);
    void onChannelRemoved   (const QString &host, const QString &channel);
    void onMessageAdded     (const QString &host, const QString &channel, const Message &msg);
    void onTopicChanged     (const QString &host, const QString &channel, const QString &topic);
    void onNickListChanged  (const QString &host, const QString &channel);
    void onUnreadChanged    (const QString &host, const QString &channel, int count);
    void onSelfNickChanged  (const QString &host, const QString &nick);

    // UI → Model
    void onSidebarSelectionChanged();
    void onInputSubmit();

private:
    void setupToolbar();
    void setupSidebar();
    void setupChatArea();
    void setupNickDock();
    void setupInputBar();
    void connectModel();

    void switchToChannel(const QString &host, const QString &channel);
    void refreshChatView(const QString &host, const QString &channel);
    void refreshNickList(const QString &host, const QString &channel);
    void refreshTopicBar(const QString &host, const QString &channel);
    void appendMessage  (const Message &msg);

    QString    formatMessage(const Message &msg) const;
    static QColor nickColor(const QString &nick);

    // Tab completion
    void handleTabComplete();
    QStringList m_tabCandidates;
    int         m_tabCandidateIndex{0};
    QString     m_tabPrefix;
    bool        m_tabActive{false};

    // Input history
    void handleHistoryUp();
    void handleHistoryDown();
    QStringList m_inputHistory;
    int         m_historyIndex{-1};
    QString     m_historyDraft;

    QTreeWidgetItem *findServerItem (const QString &host) const;
    QTreeWidgetItem *findChannelItem(const QString &host, const QString &channel) const;

    // Widgets
    QTreeWidget  *m_sidebar;
    QTextEdit    *m_chatView;
    QLineEdit    *m_input;
    QLabel       *m_nickPrefix;
    QPushButton  *m_emojiBtn;
    QDockWidget  *m_nickDock;
    QListWidget  *m_nickList;
    QWidget      *m_topicBar;
    QLabel       *m_topicLabel;
    QLabel       *m_modesLabel;
    QAction      *m_toggleTopicAction;
    QToolButton  *m_hamburger;
    DocsDialog   *m_docsDialog{nullptr};

    SessionModel *m_model;
    TrayIcon     *m_tray{nullptr};
    Config        m_config;

    bool m_showNickPrefix{true};
    bool m_showEmojiBtn{false};
    bool m_showTopic{true};
};
