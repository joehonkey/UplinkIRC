#include "serverdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

ServerDialog::ServerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Add Server");
    setMinimumWidth(380);

    m_name     = new QLineEdit;
    m_host     = new QLineEdit;
    m_host->setPlaceholderText("irc.example.org");
    m_port     = new QSpinBox;
    m_port->setRange(1, 65535);
    m_port->setValue(6697);
    m_ssl      = new QCheckBox("Use SSL/TLS");
    m_ssl->setChecked(true);
    m_nick     = new QLineEdit;
    m_user     = new QLineEdit;
    m_realname = new QLineEdit;
    m_password = new QLineEdit;
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setPlaceholderText("optional");
    m_saslUser         = new QLineEdit;
    m_saslPassword     = new QLineEdit;
    m_saslPassword->setEchoMode(QLineEdit::Password);
    m_nickservPassword = new QLineEdit;
    m_nickservPassword->setEchoMode(QLineEdit::Password);
    m_nickservPassword->setPlaceholderText("optional");

    auto makeHeader = [](const QString &text) {
        auto *lbl = new QLabel(text);
        lbl->setStyleSheet("font-weight: bold; margin-top: 6px;");
        return lbl;
    };

    auto *form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    form->addRow(makeHeader("Connection"));
    form->addRow("Name:",    m_name);
    form->addRow("Host:",    m_host);
    form->addRow("Port:",    m_port);
    form->addRow("",         m_ssl);

    form->addRow(makeHeader("Identity"));
    form->addRow("Nick:",      m_nick);
    form->addRow("Username:",  m_user);
    form->addRow("Real Name:", m_realname);

    form->addRow(makeHeader("Authentication"));
    form->addRow("Server Password:", m_password);
    form->addRow("SASL User:",       m_saslUser);
    form->addRow("SASL Password:",   m_saslPassword);
    form->addRow("NickServ:",        m_nickservPassword);

    m_bouncerType = new QComboBox;
    m_bouncerType->addItem("None",  static_cast<int>(BouncerType::None));
    m_bouncerType->addItem("ZNC",   static_cast<int>(BouncerType::ZNC));
    m_bouncerType->addItem("Soju",  static_cast<int>(BouncerType::Soju));
    m_bouncerNetwork = new QLineEdit;
    m_bouncerNetwork->setPlaceholderText("network name (soju only)");

    form->addRow(makeHeader("Bouncer"));
    form->addRow("Type:",    m_bouncerType);
    form->addRow("Network:", m_bouncerNetwork);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

ServerDialog::ServerDialog(const ServerConfig &existing, QWidget *parent)
    : ServerDialog(parent)
{
    setWindowTitle("Edit Server");
    m_name->setText(existing.name);
    m_host->setText(existing.host);
    m_port->setValue(existing.port);
    m_ssl->setChecked(existing.ssl);
    m_nick->setText(existing.nick);
    m_user->setText(existing.user);
    m_realname->setText(existing.realname);
    m_password->setText(existing.password);
    m_saslUser->setText(existing.saslUser);
    m_saslPassword->setText(existing.saslPassword);
    m_nickservPassword->setText(existing.nickservPassword);
    m_bouncerType->setCurrentIndex(static_cast<int>(existing.bouncerType));
    m_bouncerNetwork->setText(existing.bouncerNetwork);
}

ServerConfig ServerDialog::serverConfig() const
{
    ServerConfig sc;
    sc.name             = m_name->text().trimmed();
    sc.host             = m_host->text().trimmed();
    sc.port             = static_cast<quint16>(m_port->value());
    sc.ssl              = m_ssl->isChecked();
    sc.nick             = m_nick->text().trimmed();
    sc.user             = m_user->text().trimmed();
    sc.realname         = m_realname->text().trimmed();
    sc.password         = m_password->text();
    sc.saslUser         = m_saslUser->text().trimmed();
    sc.saslPassword     = m_saslPassword->text();
    sc.nickservPassword = m_nickservPassword->text();
    sc.bouncerType      = static_cast<BouncerType>(m_bouncerType->currentData().toInt());
    sc.bouncerNetwork   = m_bouncerNetwork->text().trimmed();
    return sc;
}
