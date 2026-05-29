#include "fontdialog.h"

#include <QFontComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QFrame>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

FontDialog::FontDialog(const QString &family, const FontSizes &sizes, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Font Configuration");
    setMinimumWidth(360);

    m_familyBox = new QFontComboBox;
    m_familyBox->setCurrentFont(QFont(family));

    m_spToolbar      = makeSpinBox(sizes.toolbar);
    m_spServerHeader = makeSpinBox(sizes.serverHeader);
    m_spSidebar      = makeSpinBox(sizes.sidebar);
    m_spChat         = makeSpinBox(sizes.chat);
    m_spNickList     = makeSpinBox(sizes.nickList);
    m_spNickDock     = makeSpinBox(sizes.nickDock);
    m_spTopicBar     = makeSpinBox(sizes.topicBar);
    m_spInputNick    = makeSpinBox(sizes.inputNick);
    m_spInput        = makeSpinBox(sizes.input);
    m_spTyping       = makeSpinBox(sizes.typing);
    m_spStatusBar    = makeSpinBox(sizes.statusBar);

    m_preview = new QLabel("The quick brown fox — AaBbCc 0123");
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_preview->setContentsMargins(8, 8, 8, 8);

    auto updatePreview = [this]{
        m_preview->setFont(QFont(m_familyBox->currentFont().family(), m_spChat->value()));
    };
    connect(m_familyBox, &QFontComboBox::currentFontChanged, this, updatePreview);
    connect(m_spChat, QOverload<int>::of(&QSpinBox::valueChanged), this, updatePreview);
    updatePreview();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *form = new QFormLayout;
    form->addRow("Family:",         m_familyBox);
    form->addRow("Toolbar:",        m_spToolbar);
    form->addRow("Network Name:",   m_spServerHeader);
    form->addRow("Channels:",       m_spSidebar);
    form->addRow("Chat:",           m_spChat);
    form->addRow("User List:",      m_spNickList);
    form->addRow("Users Title:",    m_spNickDock);
    form->addRow("Topic Bar:",      m_spTopicBar);
    form->addRow("Nick Label:",     m_spInputNick);
    form->addRow("Input:",          m_spInput);
    form->addRow("Typing Indicator:", m_spTyping);
    form->addRow("Status Bar:",       m_spStatusBar);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_preview);
    layout->addWidget(buttons);
}

QSpinBox *FontDialog::makeSpinBox(int value)
{
    auto *sb = new QSpinBox;
    sb->setRange(6, 32);
    sb->setSuffix(" pt");
    sb->setValue(value);
    return sb;
}

QString   FontDialog::selectedFamily() const { return m_familyBox->currentFont().family(); }
FontSizes FontDialog::selectedSizes()  const
{
    return { m_spToolbar->value(), m_spServerHeader->value(), m_spSidebar->value(),
             m_spChat->value(), m_spNickList->value(), m_spNickDock->value(),
             m_spTopicBar->value(), m_spInputNick->value(), m_spInput->value(),
             m_spTyping->value(), m_spStatusBar->value() };
}
