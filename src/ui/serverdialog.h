#pragma once
#include "config/config.h"
#include <QDialog>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;

class ServerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ServerDialog(QWidget *parent = nullptr);
    explicit ServerDialog(const ServerConfig &existing, QWidget *parent = nullptr);
    ServerConfig serverConfig() const;

private:
    QLineEdit *m_name;
    QLineEdit *m_host;
    QSpinBox  *m_port;
    QCheckBox *m_ssl;
    QLineEdit *m_nick;
    QLineEdit *m_user;
    QLineEdit *m_realname;
    QLineEdit *m_password;
    QLineEdit *m_saslUser;
    QLineEdit *m_saslPassword;
    QLineEdit *m_nickservPassword;
    QComboBox *m_bouncerType;
    QLineEdit *m_bouncerNetwork;
    QLineEdit *m_autoJoin;
};
