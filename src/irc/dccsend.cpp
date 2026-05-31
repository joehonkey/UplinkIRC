#include "dccsend.h"

#include <QFileInfo>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QtEndian>

DccSend::DccSend(const QString &filepath, QObject *parent)
    : QObject(parent)
    , m_file(filepath)
{}

bool DccSend::listen(QHostAddress bindAddr)
{
    if (!m_file.open(QIODevice::ReadOnly)) {
        emit error("Cannot open: " + m_file.fileName());
        return false;
    }

    const qint64 size = filesize();
    if (size <= 0 || size > static_cast<qint64>(UINT32_MAX)) {
        emit error(size <= 0 ? "File is empty" : "File too large for DCC (max 4 GiB)");
        m_file.close();
        return false;
    }

    m_server = new QTcpServer(this);
    if (!m_server->listen(bindAddr, 0)) {
        emit error("Cannot bind listen port");
        m_file.close();
        return false;
    }
    connect(m_server, &QTcpServer::newConnection, this, &DccSend::onNewConnection);

    QTimer::singleShot(60000, this, [this]{
        if (!m_socket)
            emit error("No connection received (timeout)");
    });
    return true;
}

void DccSend::cancel()
{
    if (m_server) m_server->close();
    if (m_socket) m_socket->abort();
    m_file.close();
}

quint16 DccSend::port() const
{
    return m_server ? m_server->serverPort() : 0;
}

QString DccSend::filename() const
{
    return QFileInfo(m_file).fileName().replace(' ', '_');
}

qint64 DccSend::filesize() const
{
    return QFileInfo(m_file.fileName()).size();
}

void DccSend::onNewConnection()
{
    m_socket = m_server->nextPendingConnection();
    m_server->close();
    connect(m_socket, &QTcpSocket::readyRead,     this, &DccSend::onReadyRead);
    connect(m_socket, &QTcpSocket::bytesWritten,  this, &DccSend::onBytesWritten);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, [this]{
        emit error(m_socket->errorString());
    });
    sendNextChunk();
}

void DccSend::onReadyRead()
{
    // ACKs are 4-byte big-endian cumulative byte counts
    while (m_socket->bytesAvailable() >= 4) {
        quint32 raw;
        m_socket->read(reinterpret_cast<char *>(&raw), 4);
        if (static_cast<qint64>(qFromBigEndian(raw)) >= filesize()) {
            emit finished();
            return;
        }
    }
}

void DccSend::onBytesWritten(qint64)
{
    sendNextChunk();
}

void DccSend::sendNextChunk()
{
    if (!m_socket || m_socket->bytesToWrite() > kChunk * 4) return;
    const QByteArray chunk = m_file.read(kChunk);
    if (chunk.isEmpty()) return;  // EOF — wait for final ACK
    m_socket->write(chunk);
    m_sent += chunk.size();
    emit progress(m_sent, filesize());
}
