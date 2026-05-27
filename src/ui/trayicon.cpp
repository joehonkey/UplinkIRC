#include "trayicon.h"
#include "mainwindow.h"
#include "model/sessionmodel.h"
#include "ui/appicons.h"

#include <QApplication>
#include <QPixmap>
#include <QPainter>

// Overlay a red dot on the icon to signal unread messages
static QIcon withUnreadBadge(const QIcon &base)
{
    QPixmap pm = base.pixmap(32, 32);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor("#e05050"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(20, 0, 11, 11);
    return QIcon(pm);
}

TrayIcon::TrayIcon(SessionModel *model, MainWindow *window)
    : QSystemTrayIcon(window)
    , m_window(window)
    , m_model(model)
{
    m_baseIcon = AppIcons::appIcon();
    setIcon(m_baseIcon);
    setToolTip("UplinkIRC");

    buildMenu();
    // Right-click menu only — left click is handled via activated signal
    setContextMenu(m_menu);

    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::onActivated);
    connect(m_model, &SessionModel::serverConnected,    this, &TrayIcon::onServerConnected);
    connect(m_model, &SessionModel::serverDisconnected, this, &TrayIcon::onServerDisconnected);
    connect(m_model, &SessionModel::unreadChanged,      this, &TrayIcon::onUnreadChanged);

    show();
}

void TrayIcon::buildMenu()
{
    m_menu = new QMenu;

    m_showAction = m_menu->addAction("Show", this, [this]{
        m_window->show();
        m_window->raise();
        m_window->activateWindow();
        updateShowAction();
    });

    m_menu->addSeparator();
    m_menu->addAction("Quit", qApp, &QApplication::quit);

    updateShowAction();
}

void TrayIcon::setBaseIcon(const QIcon &icon)
{
    m_baseIcon = icon;
    setIcon(m_totalUnread > 0 ? withUnreadBadge(m_baseIcon) : m_baseIcon);
}

void TrayIcon::updateShowAction()
{
    m_showAction->setText(m_window->isVisible() ? "Show" : "Show");
    // Always labelled "Show" — clicking it always brings the window up
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        // Left click — always show and raise
        m_window->show();
        m_window->raise();
        m_window->activateWindow();
    }
    // Right click is handled automatically by the context menu
}

void TrayIcon::onServerConnected(const QString &host)
{
    Q_UNUSED(host)
    updateTooltip();
}

void TrayIcon::onServerDisconnected(const QString &host)
{
    Q_UNUSED(host)
    updateTooltip();
}

void TrayIcon::onUnreadChanged(const QString &, const QString &, int)
{
    m_totalUnread = 0;
    for (const auto &sess : m_model->sessions())
        for (const auto &ch : sess.channels)
            m_totalUnread += ch.unread;

    setUnread(m_totalUnread > 0);
}

void TrayIcon::setUnread(bool hasUnread)
{
    setIcon(hasUnread ? withUnreadBadge(m_baseIcon) : m_baseIcon);
    if (hasUnread && !m_window->isVisible())
        showMessage("UplinkIRC",
                    QString("%1 unread message(s)").arg(m_totalUnread),
                    QSystemTrayIcon::Information, 3000);
}

void TrayIcon::updateTooltip()
{
    QStringList connected;
    for (const auto &sess : m_model->sessions())
        if (sess.connected) connected << sess.host;

    setToolTip(connected.isEmpty()
               ? "UplinkIRC — not connected"
               : "UplinkIRC — " + connected.join(", "));
}
