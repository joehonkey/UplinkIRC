#pragma once

#include <QSystemTrayIcon>
#include <QMenu>

class MainWindow;
class SessionModel;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit TrayIcon(SessionModel *model, MainWindow *window);

    void setUnread(bool hasUnread);

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onServerConnected(const QString &host);
    void onServerDisconnected(const QString &host);
    void onUnreadChanged(const QString &host, const QString &channel, int count);

private:
    void buildMenu();
    void updateTooltip();

    void updateShowAction();

    MainWindow   *m_window;
    SessionModel *m_model;
    QMenu        *m_menu;
    QAction      *m_showAction;
    QIcon         m_baseIcon;
    int           m_totalUnread{0};
};
