#include "emojipicker.h"
#include "emojidata.h"

#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QApplication>
#include <QScreen>

static constexpr int kCols     = 9;
static constexpr int kBtnSize  = 34;
static constexpr int kPickerW  = kCols * (kBtnSize + 2) + 16;
static constexpr int kPickerH  = 340;

EmojiPicker::EmojiPicker(QWidget *parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setFixedSize(kPickerW, kPickerH);
    setObjectName("emojiPicker");
    setFrameShape(QFrame::StyledPanel);

    auto *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(4, 4, 4, 4);
    vbox->setSpacing(4);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("Search emoji...");
    m_search->setClearButtonEnabled(true);
    vbox->addWidget(m_search);

    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbox->addWidget(m_scroll);

    m_gridWidget = new QWidget;
    m_gridLayout = new QGridLayout(m_gridWidget);
    m_gridLayout->setSpacing(2);
    m_gridLayout->setContentsMargins(2, 2, 2, 2);
    m_scroll->setWidget(m_gridWidget);

    buildButtons();
    filterGrid({});

    connect(m_search, &QLineEdit::textChanged, this, &EmojiPicker::onSearch);
}

void EmojiPicker::buildButtons()
{
    QFont f = font();
    f.setPointSize(16);

    for (const auto &entry : emojiTable()) {
        auto *btn = new QToolButton(m_gridWidget);
        btn->setText(entry.ch);
        btn->setFont(f);
        btn->setFixedSize(kBtnSize, kBtnSize);
        btn->setAutoRaise(true);
        btn->setToolTip(entry.shortcode);
        btn->setFocusPolicy(Qt::NoFocus);

        connect(btn, &QToolButton::clicked, this, [this, ch = entry.ch]{
            hide();
            emit emojiSelected(ch);
        });

        m_buttons.append({btn, entry.shortcode});
    }
}

void EmojiPicker::filterGrid(const QString &filter)
{
    // Remove all from layout without deleting widgets
    while (m_gridLayout->count() > 0) {
        auto *item = m_gridLayout->takeAt(0);
        delete item;
    }

    int col = 0, row = 0;
    for (auto &entry : m_buttons) {
        const bool match = filter.isEmpty() ||
                           entry.shortcode.contains(filter, Qt::CaseInsensitive);
        if (match) {
            m_gridLayout->addWidget(entry.btn, row, col);
            entry.btn->show();
            if (++col >= kCols) { col = 0; ++row; }
        } else {
            entry.btn->hide();
        }
    }

    m_gridWidget->adjustSize();
}

void EmojiPicker::onSearch(const QString &text)
{
    filterGrid(text.trimmed());
    m_scroll->verticalScrollBar()->setValue(0);
}

void EmojiPicker::showAt(const QPoint &globalAnchor)
{
    m_search->clear();
    filterGrid({});
    m_scroll->verticalScrollBar()->setValue(0);

    // Position: prefer above-left of anchor; clamp to screen
    QPoint pos(globalAnchor.x() - width(), globalAnchor.y() - height());

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    pos.rx() = qBound(screen.left(), pos.x(), screen.right()  - width());
    pos.ry() = qBound(screen.top(),  pos.y(), screen.bottom() - height());

    move(pos);
    show();
    raise();
    m_search->setFocus();
}
