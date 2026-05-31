#include <QApplication>
#include <QFont>
#include <QInputDialog>
#include "config/config.h"
#include "model/sessionmodel.h"
#include "ui/mainwindow.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("UplinkIRC");
    app.setApplicationVersion(UPLINKIRC_VERSION);

#if defined(Q_OS_WIN)
    // Use native Windows rendering as the base style.
    // Custom themes layer QSS on top; the default theme leaves it pure native.
    if (!QApplication::setStyle("windows11"))
        if (!QApplication::setStyle("windowsvista"))
            QApplication::setStyle("windows");
#endif

    const QString cfgPath = Config::defaultPath();
    Config cfg = Config::load(cfgPath);

    QFont font(cfg.ui.fontFamily);
    font.setStyleHint(QFont::Monospace);
    app.setFont(font);

    // First-run nick setup
    if (cfg.needsNickSetup()) {
        bool ok = false;
        const QString nick = QInputDialog::getText(
            nullptr, "UplinkIRC — Set Your Nick",
            "Choose a nickname:", QLineEdit::Normal, "", &ok);
        if (ok && !nick.trimmed().isEmpty()) {
            for (auto &s : cfg.servers)
                if (s.nick == "yournick" || s.nick.isEmpty())
                    s.nick = nick.trimmed();
            Config::save(cfg, cfgPath);
        }
    }

    auto *model = new SessionModel;

    MainWindow window(model, cfg);
    window.show();

    // Load config after MainWindow has connected its signals
    model->loadConfig(cfg);

    return app.exec();
}
