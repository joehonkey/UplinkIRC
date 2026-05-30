#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QColor>
#include <QHash>
#include <QSet>
#include <QPair>
#include "model/sessionmodel.h"
#include "config/config.h"
#include "ui/themeloader.h"

class TrayIcon;
class SignalBars;
class AboutDialog;
class DocsDialog;
class PreferencesDialog;
class LinkPreview;
class EmojiPicker;

class QTimer;
class QTreeWidget;
class QTreeWidgetItem;
class QTextBrowser;
class QLineEdit;
class QLabel;
class QPushButton;
class QListWidget;
class QToolButton;
class QSplitter;
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
    void changeEvent(QEvent *event) override;

private slots:
    // Model → UI
    void onServerAdded      (const QString &host);
    void onServerConnected  (const QString &host);
    void onServerDisconnected(const QString &host);
    void onChannelAdded     (const QString &host, const QString &channel);
    void onChannelRemoved   (const QString &host, const QString &channel);
    void onMessageAdded     (const QString &host, const QString &channel, const Message &msg);
    void onTopicChanged     (const QString &host, const QString &channel, const QString &topic);
    void onNickListChanged     (const QString &host, const QString &channel);
    void onNickListContextMenu  (const QPoint &pos);
    void onSidebarContextMenu   (const QPoint &pos);
    void onUnreadChanged    (const QString &host, const QString &channel, int count);
    void onSelfNickChanged  (const QString &host, const QString &nick);

    // UI → Model
    void onSidebarSelectionChanged();
    void onInputSubmit();

    // Typing
    void onTypingReceived(const QString &host, const QString &channel,
                          const QString &nick, const QString &state);

private:
    void setupToolbar();
    void setupSidebar();
    void setupChatArea();
    void setupNickPanel();
    void setupInputBar();
    void connectModel();
    void connectPreferences();

    void switchToChannel(const QString &host, const QString &channel);
    void refreshChatView(const QString &host, const QString &channel);
    void refreshNickList(const QString &host, const QString &channel);
    void refreshTopicBar(const QString &host, const QString &channel);
    void appendMessage  (const Message &msg, bool autoPreview = false);
    void applyFontSizes();
    void updateTypingLabel();
    void applyAppIcon(const QString &choice);

    QString    formatMessage(const Message &msg) const;
    static QColor nickColor(const QString &nick);

    // Tab completion
    void handleTabComplete();
    QStringList m_tabCandidates;
    int         m_tabCandidateIndex{0};
    QString     m_tabPrefix;
    bool        m_tabActive{false};

    // Emoji inline autocomplete
    void checkEmojiAutocomplete(const QString &text);
    void commitEmojiAutocomplete(int row);
    void hideEmojiAutocomplete();
    void repositionTypingLabel();
    QListWidget *m_emojiCompleter{nullptr};
    int          m_emojiTriggerPos{-1};

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
    QTextBrowser *m_chatView;
    QLineEdit    *m_input;
    QLabel       *m_nickPrefix;
    QPushButton  *m_emojiBtn;
    QWidget      *m_sidebarPanel{nullptr};
    QWidget      *m_sidebarHeader{nullptr};
    QWidget      *m_topicLeft{nullptr};
    QToolButton  *m_sidebarToggleBtn{nullptr};
    bool          m_sidebarExpanded{true};
    int           m_sidebarExpandedWidth{180};
    QSplitter    *m_mainSplitter{nullptr};
    QWidget      *m_rightContent{nullptr};
    QListWidget  *m_nickList;
    QWidget      *m_nickPanel{nullptr};
    QLabel       *m_nickCountLabel{nullptr};
    QToolButton  *m_nickToggleBtn{nullptr};
    QSplitter    *m_chatSplitter{nullptr};
    QTimer       *m_gearTimer{nullptr};
    int           m_gearAngle{0};
    bool          m_nickExpanded{true};
    QWidget      *m_topicBar;               // info bar — always visible
    QLabel       *m_topicLabel{nullptr};    // #channel (modes)
    QLabel       *m_modesLabel{nullptr};    // stretch spacer
    QLabel       *m_userInfoLabel{nullptr}; // * network — N users
    QWidget      *m_topicDisplay{nullptr};  // topic text — shown when showTopic
    QLabel       *m_topicText{nullptr};
    QToolButton  *m_hamburger;
    QLabel       *m_appLabel{nullptr};
    QLabel       *m_typingLabel{nullptr};
    AboutDialog       *m_aboutDialog{nullptr};
    DocsDialog        *m_docsDialog{nullptr};
    PreferencesDialog *m_prefsDialog{nullptr};
    EmojiPicker       *m_emojiPicker{nullptr};

    // Typing indicator state
    QTimer                      *m_typingOutTimer{nullptr};
    bool                         m_typingActive{false};
    QHash<QString, QSet<QString>> m_typingNicks;       // "host|channel" → nicks
    QHash<QString, QTimer*>       m_typingNickTimers;  // "host|channel|nick" → timeout
    QHash<QString, QString>       m_botIcons;          // lowercased nick → cached bot icon

    SessionModel *m_model;
    TrayIcon     *m_tray{nullptr};
    SignalBars   *m_signalBars{nullptr};
    LinkPreview  *m_linkPreview{nullptr};
    QString       m_hoveredUrl;
    QPoint        m_hoverGlobalPos;
    QHash<QString, QPair<QString,QString>> m_previewChannels; // url → {host, channel}
    Config        m_config;
    Theme         m_theme;

    bool m_showNickPrefix{true};
    bool m_showEmojiBtn{false};
    bool m_showTopic{true};

};
