#include "trayicon.h"
#include "mainwindow.h"
#include "model/sessionmodel.h"

#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

// Build a simple placeholder icon — a filled circle with "U" until real icons land
static QIcon makePlaceholderIcon(bool unread = false)
{
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(unread ? QColor("#e05050") : QColor("#5080e0"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(1, 1, 30, 30);
    p.setPen(Qt::white);
    QFont f = p.font();
    f.setBold(true);
    f.setPointSize(14);
    p.setFont(f);
    p.drawText(pm.rect(), Qt::AlignCenter, "U");
    return QIcon(pm);
}

TrayIcon::TrayIcon(SessionModel *model, MainWindow *window)
    : QSystemTrayIcon(window)
    , m_window(window)
    , m_model(model)
{
    setIcon(makePlaceholderIcon());
    setToolTip("UplinkIRC");

    buildMenu();
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

    m_toggleAction = m_menu->addAction("Hide", this, [this]{
        if (m_window->isVisible()) {
            m_window->hide();
            m_toggleAction->setText("Show");
        } else {
            m_window->show();
            m_window->raise();
            m_window->activateWindow();
            m_toggleAction->setText("Hide");
        }
    });

    m_menu->addSeparator();
    m_menu->addAction("Quit", qApp, &QApplication::quit);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (m_window->isVisible()) {
            m_window->hide();
            m_toggleAction->setText("Show");
        } else {
            m_window->show();
            m_window->raise();
            m_window->activateWindow();
            m_toggleAction->setText("Hide");
        }
    }
}

void TrayIcon::onServerConnected(const QString &host)
{
    setToolTip("UplinkIRC — " + host);
    updateTooltip();
}

void TrayIcon::onServerDisconnected(const QString &host)
{
    Q_UNUSED(host)
    updateTooltip();
}

void TrayIcon::onUnreadChanged(const QString &, const QString &, int)
{
    // Recount total unread across all sessions/channels
    m_totalUnread = 0;
    for (const auto &sess : m_model->sessions())
        for (const auto &ch : sess.channels)
            m_totalUnread += ch.unread;

    setUnread(m_totalUnread > 0);
}

void TrayIcon::setUnread(bool hasUnread)
{
    setIcon(makePlaceholderIcon(hasUnread));
    if (hasUnread && !m_window->isVisible())
        showMessage("UplinkIRC", QString("You have %1 unread message(s)").arg(m_totalUnread),
                    QSystemTrayIcon::Information, 3000);
}

void TrayIcon::updateTooltip()
{
    QStringList connected;
    for (const auto &sess : m_model->sessions())
        if (sess.connected) connected << sess.host;

    if (connected.isEmpty())
        setToolTip("UplinkIRC — not connected");
    else
        setToolTip("UplinkIRC — " + connected.join(", "));
}
