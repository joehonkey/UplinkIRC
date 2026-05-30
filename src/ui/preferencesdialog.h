#pragma once
#include <QDialog>
#include "config/config.h"

class QListWidget;
class QCheckBox;
class QComboBox;

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(const Config &config, QWidget *parent = nullptr);

signals:
    void themeChanged(const QString &name);
    void fontConfigRequested();
    void appIconChanged(const QString &key);
    void topicBarToggled(bool on);
    void nickPrefixToggled(bool on);
    void emojiBtnToggled(bool on);
    void typingIndicatorToggled(bool on);
    void notificationsToggled(bool on);
    void coloredNicksToggled(bool on);
    void hangingIndentToggled(bool on);
    void nickBracketsChanged(const QString &brackets);
    void manageServersRequested();
    void aboutRequested();
    void docsRequested();

private:
    QListWidget *m_themeList{nullptr};
    QListWidget *m_iconList{nullptr};
    QCheckBox   *m_topicCheck{nullptr};
    QCheckBox   *m_nickPrefixCheck{nullptr};
    QCheckBox   *m_emojiCheck{nullptr};
    QCheckBox   *m_typingCheck{nullptr};
    QCheckBox   *m_notificationsCheck{nullptr};
    QCheckBox   *m_coloredNicksCheck{nullptr};
    QCheckBox   *m_hangingIndentCheck{nullptr};
    QComboBox   *m_bracketsCombo{nullptr};

    static const QList<QPair<QString,QString>> s_iconChoices;
    static const QList<QPair<QString,QString>> s_bracketChoices;
};
