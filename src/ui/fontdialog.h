#pragma once
#include <QDialog>
#include "config/config.h"

class QFontComboBox;
class QSpinBox;
class QLabel;

class FontDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FontDialog(const QString &family, const FontSizes &sizes, QWidget *parent = nullptr);
    QString    selectedFamily() const;
    FontSizes  selectedSizes()  const;

private:
    QFontComboBox *m_familyBox;
    QSpinBox      *m_spToolbar;
    QSpinBox      *m_spServerHeader;
    QSpinBox      *m_spSidebar;
    QSpinBox      *m_spChat;
    QSpinBox      *m_spNickList;
    QSpinBox      *m_spNickDock;
    QSpinBox      *m_spTopicBar;
    QSpinBox      *m_spInputNick;
    QSpinBox      *m_spInput;
    QSpinBox      *m_spTyping;
    QSpinBox      *m_spStatusBar;
    QLabel        *m_preview;

    QSpinBox *makeSpinBox(int value);
};
