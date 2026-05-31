#pragma once

#include <QFile>
#include <QHostAddress>
#include <QObject>

class QTcpServer;
class QTcpSocket;

class DccSend : public QObject
{
    Q_OBJECT
public:
    explicit DccSend(const QString &filepath, QObject *parent = nullptr);

    bool    listen(QHostAddress bindAddr = QHostAddress::Any);
    void    cancel();
    quint16 port()     const;
    QString filename() const;
    qint64  filesize() const;

signals:
    void progress(qint64 sent, qint64 total);
    void finished();
    void error(const QString &msg);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onBytesWritten(qint64);

private:
    void sendNextChunk();

    QTcpServer *m_server{nullptr};
    QTcpSocket *m_socket{nullptr};
    QFile       m_file;
    qint64      m_sent{0};

    static constexpr qint32 kChunk = 65536;
};
