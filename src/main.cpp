#include <QApplication>
#include <QInputDialog>
#include "config/config.h"
#include "model/sessionmodel.h"
#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("UplinkIRC");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("LinuxDojo");

    const QString cfgPath = Config::defaultPath();
    Config cfg = Config::load(cfgPath);

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
    model->loadConfig(cfg);

    MainWindow window(model);
    window.show();

    return app.exec();
}
