#include "aboutdialog.h"
#include "appicons.h"
#include "version.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About UplinkIRC");
    setWindowIcon(AppIcons::appIcon());
    setFixedSize(460, 280);

    if (parent) {
        move(parent->width()  / 2 - 230,
             parent->height() / 2 - 140);
    }

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(24, 20, 24, 16);

    auto *logo = new QLabel;
    logo->setPixmap(AppIcons::appIcon("dark").pixmap(96, 96));
    logo->setAlignment(Qt::AlignCenter);
    layout->addWidget(logo);

    // Version + description
    auto *version = new QLabel("UplinkIRC  v" UPLINKIRC_VERSION);
    QFont f = version->font();
    f.setBold(true);
    f.setPointSize(f.pointSize() + 2);
    version->setFont(f);
    version->setAlignment(Qt::AlignCenter);
    layout->addWidget(version);

    auto *desc = new QLabel(
        "A fast, secure, IRCv3-featured IRC client\n"
        "built with Qt6 and C++\n\n"
        "irc.linuxdojo.org  •  #uplink"
    );
    desc->setAlignment(Qt::AlignCenter);
    desc->setStyleSheet("color: palette(text);");
    layout->addWidget(desc);

    layout->addStretch();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
}
