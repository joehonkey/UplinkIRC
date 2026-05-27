#include "docsdialog.h"
#include "appicons.h"

#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFile>
#include <QTextStream>

DocsDialog::DocsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("UplinkIRC Documentation");
    setWindowIcon(AppIcons::appIcon());
    resize(800, 580);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 8);
    layout->setSpacing(4);

    auto *tabs = new QTabWidget;
    addTab(tabs, "Configuration",  ":/docs/configuration.md");
    addTab(tabs, "Commands",       ":/docs/commands.md");
    addTab(tabs, "IRCv3",          ":/docs/ircv3.md");
    addTab(tabs, "FAQ",            ":/docs/faq.md");
    addTab(tabs, "Shortcuts",      ":/docs/keyboard-shortcuts.md");
    layout->addWidget(tabs, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
    layout->addWidget(buttons);
    layout->setContentsMargins(8, 0, 8, 8);
}

void DocsDialog::addTab(QTabWidget *tabs, const QString &title, const QString &resource)
{
    QFile f(resource);
    QString content;
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&f);
        content = ts.readAll();
    } else {
        content = "# " + title + "\n\nDocumentation not available.";
    }

    auto *browser = new QTextBrowser;
    browser->setMarkdown(content);
    browser->setOpenExternalLinks(false);
    browser->setStyleSheet("QTextBrowser { padding: 10px; }");
    tabs->addTab(browser, title);
}
