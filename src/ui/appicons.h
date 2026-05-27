#pragma once

#include <QIcon>
#include <QSettings>

namespace AppIcons {

// Which icon the user has chosen — persisted via QSettings
inline QString activeIconName()
{
    QSettings s("LinuxDojo", "UplinkIRC");
    return s.value("appearance/icon", "maindefault").toString();
}

inline void setActiveIconName(const QString &name)
{
    QSettings s("LinuxDojo", "UplinkIRC");
    s.setValue("appearance/icon", name);
}

inline QIcon appIcon()
{
    const QString name = activeIconName();
    return name == "mainalt" ? QIcon(":/icons/mainalt.png")
                             : QIcon(":/icons/maindefault.png");
}

inline QIcon defaultIcon()  { return QIcon(":/icons/maindefault.png"); }
inline QIcon altIcon()      { return QIcon(":/icons/mainalt.png");     }
inline QIcon aboutIcon()    { return QIcon(":/icons/about.png");       }

} // namespace AppIcons
