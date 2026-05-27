#pragma once

#include <QIcon>

namespace AppIcons {

inline QIcon platformIcon()
{
#if defined(Q_OS_LINUX)
    return QIcon(":/icons/linux.png");
#elif defined(Q_OS_FREEBSD)
    return QIcon(":/icons/freebsd.png");
#elif defined(Q_OS_MACOS)
    return QIcon(":/icons/macos.png");
#elif defined(Q_OS_WIN)
    return QIcon(":/icons/windows.png");
#else
    return QIcon(":/icons/linux.png");
#endif
}

inline QIcon aboutIcon()  { return QIcon(":/icons/about.png"); }
inline QIcon logoIcon()   { return QIcon(":/icons/logo.png");  }

} // namespace AppIcons
