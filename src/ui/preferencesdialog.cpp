#include "preferencesdialog.h"
#include "ui/themeloader.h"
#include "ui/menuicons.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>

const QList<QPair<QString,QString>> PreferencesDialog::s_iconChoices = {
    { "dark",          "Dark"           },
    { "light-default", "Light (default)" },
    { "light",         "Light"          },
    { "avatar",        "Avatar"         },
};

const QList<QPair<QString,QString>> PreferencesDialog::s_bracketChoices = {
    { "<>",   "<nick>  (angle)"   },
    { "[]",   "[nick]  (square)"  },
    { "::::", "::nick::  (colon)" },
    { "",     "nick  (none)"      },
};

static QLabel *sectionLabel(const QString &text)
{
    auto *l = new QLabel("<b>" + text + "</b>");
    l->setContentsMargins(0, 6, 0, 2);
    return l;
}

static QFrame *makeSep()
{
    auto *f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Sunken);
    return f;
}

PreferencesDialog::PreferencesDialog(const Config &cfg, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Preferences");
    setMinimumWidth(330);

    auto *inner = new QWidget;
    auto *vbox  = new QVBoxLayout(inner);
    vbox->setContentsMargins(12, 8, 12, 12);
    vbox->setSpacing(4);

    // ── Appearance ──────────────────────────────────────────────────────────
    vbox->addWidget(sectionLabel("Appearance"));

    vbox->addWidget(new QLabel("Theme:"));
    m_themeList = new QListWidget;
    m_themeList->setFrameShape(QFrame::StyledPanel);
    m_themeList->setFixedHeight(200);
    for (const QString &name : ThemeLoader::availableThemes())
        m_themeList->addItem(name);
    {
        const auto matches = m_themeList->findItems(cfg.ui.theme, Qt::MatchExactly);
        if (!matches.isEmpty()) {
            m_themeList->setCurrentItem(matches.first());
            m_themeList->scrollToItem(matches.first());
        }
    }
    connect(m_themeList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        emit themeChanged(item->text());
    });
    vbox->addWidget(m_themeList);

    vbox->addSpacing(4);
    vbox->addWidget(new QLabel("App Icon:"));
    m_iconList = new QListWidget;
    m_iconList->setFrameShape(QFrame::StyledPanel);
    m_iconList->setFixedHeight(static_cast<int>(s_iconChoices.size()) * 26 + 4);
    for (const auto &[key, label] : s_iconChoices)
        m_iconList->addItem(label);
    for (int i = 0; i < s_iconChoices.size(); ++i) {
        if (s_iconChoices[i].first == cfg.ui.appIcon) {
            m_iconList->setCurrentRow(i);
            break;
        }
    }
    connect(m_iconList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        const int idx = m_iconList->row(item);
        if (idx >= 0 && idx < s_iconChoices.size())
            emit appIconChanged(s_iconChoices[idx].first);
    });
    vbox->addWidget(m_iconList);

    vbox->addSpacing(4);
    auto *fontBtn = new QPushButton("Font Config...");
    fontBtn->setIcon(MenuIcons::fontConfig());
    connect(fontBtn, &QPushButton::clicked, this, [this]{ emit fontConfigRequested(); });
    vbox->addWidget(fontBtn);

    // ── Interface ────────────────────────────────────────────────────────────
    vbox->addSpacing(4);
    vbox->addWidget(makeSep());
    vbox->addWidget(sectionLabel("Interface"));

    m_topicCheck = new QCheckBox("Show Topic Bar");
    m_topicCheck->setChecked(cfg.ui.showTopic);
    connect(m_topicCheck, &QCheckBox::toggled, this, [this](bool on){ emit topicBarToggled(on); });
    vbox->addWidget(m_topicCheck);

    m_nickPrefixCheck = new QCheckBox("Show Nick in Input");
    m_nickPrefixCheck->setChecked(cfg.ui.showNickPrefix);
    connect(m_nickPrefixCheck, &QCheckBox::toggled, this, [this](bool on){ emit nickPrefixToggled(on); });
    vbox->addWidget(m_nickPrefixCheck);

    m_emojiCheck = new QCheckBox("Show Emoji Button");
    m_emojiCheck->setChecked(cfg.ui.showEmojiButton);
    connect(m_emojiCheck, &QCheckBox::toggled, this, [this](bool on){ emit emojiBtnToggled(on); });
    vbox->addWidget(m_emojiCheck);

    m_typingCheck = new QCheckBox("Typing Indicator");
    m_typingCheck->setChecked(cfg.ui.typingIndicator);
    connect(m_typingCheck, &QCheckBox::toggled, this, [this](bool on){ emit typingIndicatorToggled(on); });
    vbox->addWidget(m_typingCheck);

    m_notificationsCheck = new QCheckBox("Desktop Notifications");
    m_notificationsCheck->setChecked(cfg.ui.notifications);
    connect(m_notificationsCheck, &QCheckBox::toggled, this, [this](bool on){ emit notificationsToggled(on); });
    vbox->addWidget(m_notificationsCheck);

    m_coloredNicksCheck = new QCheckBox("Colored Nicks");
    m_coloredNicksCheck->setChecked(cfg.ui.coloredNicks);
    connect(m_coloredNicksCheck, &QCheckBox::toggled, this, [this](bool on){ emit coloredNicksToggled(on); });
    vbox->addWidget(m_coloredNicksCheck);

    m_hangingIndentCheck = new QCheckBox("Hanging Indent (wrap under message)");
    m_hangingIndentCheck->setChecked(cfg.ui.hangingIndent);
    connect(m_hangingIndentCheck, &QCheckBox::toggled, this, [this](bool on){ emit hangingIndentToggled(on); });
    vbox->addWidget(m_hangingIndentCheck);

    vbox->addSpacing(4);
    {
        auto *row = new QHBoxLayout;
        row->addWidget(new QLabel("Nick Brackets:"));
        m_bracketsCombo = new QComboBox;
        for (const auto &[key, label] : s_bracketChoices)
            m_bracketsCombo->addItem(label);
        for (int i = 0; i < s_bracketChoices.size(); ++i) {
            if (s_bracketChoices[i].first == cfg.ui.nickBrackets) {
                m_bracketsCombo->setCurrentIndex(i);
                break;
            }
        }
        connect(m_bracketsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int idx){
            if (idx >= 0 && idx < s_bracketChoices.size())
                emit nickBracketsChanged(s_bracketChoices[idx].first);
        });
        row->addWidget(m_bracketsCombo, 1);
        vbox->addLayout(row);
    }

    // ── Bottom actions ───────────────────────────────────────────────────────
    vbox->addSpacing(4);
    vbox->addWidget(makeSep());
    vbox->addSpacing(2);
    {
        auto *row = new QHBoxLayout;
        auto *manageBtn = new QPushButton("Manage Servers...");
        manageBtn->setIcon(MenuIcons::servers());
        connect(manageBtn, &QPushButton::clicked, this, [this]{ emit manageServersRequested(); });
        row->addWidget(manageBtn);
        row->addStretch(1);
        auto *docsBtn = new QPushButton("Docs");
        docsBtn->setIcon(MenuIcons::documentation());
        connect(docsBtn, &QPushButton::clicked, this, [this]{ emit docsRequested(); });
        auto *aboutBtn = new QPushButton("About");
        aboutBtn->setIcon(MenuIcons::about());
        connect(aboutBtn, &QPushButton::clicked, this, [this]{ emit aboutRequested(); });
        row->addWidget(docsBtn);
        row->addWidget(aboutBtn);
        vbox->addLayout(row);
    }

    vbox->addStretch(1);

    auto *scroll = new QScrollArea;
    scroll->setWidget(inner);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scroll);
}
