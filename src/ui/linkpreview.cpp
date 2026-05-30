#include "linkpreview.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QHostAddress>
#include <QBuffer>
#include <QImageReader>
#include <memory>

static constexpr int kMaxBytes    = 32768;   // 32 KB — more room for late <title> tags
static constexpr int kMaxImgBytes = 204800;  // 200 KB
static constexpr int kMaxCache    = 50;
static constexpr int kTimeoutMs   = 6000;

LinkPreview::LinkPreview(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

static bool isImageUrl(const QUrl &url)
{
    const QString p = url.path().toLower();
    return p.endsWith(".png") || p.endsWith(".jpg") || p.endsWith(".jpeg")
        || p.endsWith(".gif") || p.endsWith(".webp");
}

static bool isPrivateUrl(const QUrl &url)
{
    const QString scheme = url.scheme().toLower();
    if (scheme != "http" && scheme != "https") return true;

    const QString host = url.host().toLower();
    if (host == "localhost" || host.endsWith(".local")) return true;

    QHostAddress addr(host);
    if (!addr.isNull()) {
        if (addr.isLoopback())                                       return true;
        if (addr.isInSubnet(QHostAddress("10.0.0.0"),    8))        return true;
        if (addr.isInSubnet(QHostAddress("172.16.0.0"),  12))       return true;
        if (addr.isInSubnet(QHostAddress("192.168.0.0"), 16))       return true;
        if (addr.isInSubnet(QHostAddress("169.254.0.0"), 16))       return true;
        if (addr.isInSubnet(QHostAddress("127.0.0.0"),   8))        return true;
        if (addr.isInSubnet(QHostAddress("::1"),         128))      return true;
        if (addr.isInSubnet(QHostAddress("fc00::"),      7))        return true;
    }

    return false;
}

void LinkPreview::fetch(const QUrl &url)
{
    if (isPrivateUrl(url)) return;

    const QString key = url.toString();

    if (m_cache.contains(key)) {
        const CachedCard &c = m_cache[key];
        emit titleReady(url, c.title);
        emit cardReady(url, c.title, c.thumbnail);
        return;
    }

    if (m_reply) {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    if (isImageUrl(url)) {
        const QString filename = url.fileName();
        fetchImage(url, filename.isEmpty() ? url.host() : filename, url);
        return;
    }

    m_pendingUrl = url;
    m_buf.clear();

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/124.0 Safari/537.36");
    req.setRawHeader("Accept", "text/html,application/xhtml+xml;q=0.9,*/*;q=0.8");
    req.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setTransferTimeout(kTimeoutMs);

    m_reply = m_nam->get(req);
    QNetworkReply *reply = m_reply;

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]{
        if (m_reply != reply) return;
        const int rem = kMaxBytes - m_buf.size();
        if (rem <= 0) { reply->abort(); return; }
        m_buf.append(reply->read(rem));
        if (m_buf.size() >= kMaxBytes) reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]{
        if (m_reply != reply) return;
        const QUrl    pageUrl = m_pendingUrl;
        const QString title   = extractTitle(m_buf);
        const QUrl    imgUrl  = extractImageUrl(m_buf);
        m_reply = nullptr;
        m_buf.clear();
        reply->deleteLater();

        if (title.isEmpty()) return;

        emit titleReady(pageUrl, title);

        if (imgUrl.isValid())
            fetchImage(pageUrl, title, imgUrl);
        else {
            if (m_cache.size() >= kMaxCache) m_cache.erase(m_cache.begin());
            m_cache.insert(pageUrl.toString(), {title, {}});
            emit cardReady(pageUrl, title, {});
        }
    });
}

void LinkPreview::fetchImage(const QUrl &pageUrl, const QString &title, const QUrl &imageUrl)
{
    QNetworkRequest req(imageUrl);
    req.setRawHeader("User-Agent",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/124.0 Safari/537.36");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setTransferTimeout(kTimeoutMs);

    auto *imgReply = m_nam->get(req);
    auto  imgBuf   = std::make_shared<QByteArray>();

    connect(imgReply, &QNetworkReply::readyRead, this, [imgReply, imgBuf]{
        const int rem = kMaxImgBytes - imgBuf->size();
        if (rem <= 0) { imgReply->abort(); return; }
        imgBuf->append(imgReply->read(rem));
        if (imgBuf->size() >= kMaxImgBytes) imgReply->abort();
    });

    connect(imgReply, &QNetworkReply::finished, this,
            [this, imgReply, imgBuf, pageUrl, title]{
        QPixmap pm;
        if (!imgBuf->isEmpty()) {
            QBuffer buf(imgBuf.get());
            buf.open(QIODevice::ReadOnly);
            QImageReader reader(&buf);
            const QSize srcSize = reader.size();
            if (srcSize.isValid() && srcSize.width() <= 4096 && srcSize.height() <= 4096) {
                reader.setScaledSize(srcSize.scaled(120, 90, Qt::KeepAspectRatio));
                QImage img = reader.read();
                if (!img.isNull())
                    pm = QPixmap::fromImage(std::move(img));
            }
        }
        imgReply->deleteLater();

        if (m_cache.size() >= kMaxCache) m_cache.erase(m_cache.begin());
        m_cache.insert(pageUrl.toString(), {title, pm});
        emit cardReady(pageUrl, title, pm);
    });
}

QString LinkPreview::extractTitle(const QByteArray &data) const
{
    const QString html = QString::fromUtf8(data);

    static const QRegularExpression ogPropFirst(
        R"(<meta[^>]+property\s*=\s*["']og:title["'][^>]+content\s*=\s*["']([^"']{1,200})["'])",
        QRegularExpression::CaseInsensitiveOption);
    auto m = ogPropFirst.match(html);
    if (m.hasMatch()) return m.captured(1).trimmed();

    static const QRegularExpression ogContentFirst(
        R"(<meta[^>]+content\s*=\s*["']([^"']{1,200})["'][^>]+property\s*=\s*["']og:title["'])",
        QRegularExpression::CaseInsensitiveOption);
    m = ogContentFirst.match(html);
    if (m.hasMatch()) return m.captured(1).trimmed();

    static const QRegularExpression titleTag(
        R"(<title[^>]*>([\s\S]{1,300}?)</title>)",
        QRegularExpression::CaseInsensitiveOption);
    m = titleTag.match(html);
    if (m.hasMatch()) {
        QString t = m.captured(1).simplified();
        if (!t.isEmpty()) return t;
    }

    return {};
}

QUrl LinkPreview::extractImageUrl(const QByteArray &data) const
{
    const QString html = QString::fromUtf8(data);

    static const QRegularExpression ogPropFirst(
        R"(<meta[^>]+property\s*=\s*["']og:image["'][^>]+content\s*=\s*["']([^"']{1,500})["'])",
        QRegularExpression::CaseInsensitiveOption);
    auto m = ogPropFirst.match(html);
    if (m.hasMatch()) {
        const QUrl u(m.captured(1).trimmed());
        if (u.isValid() && u.scheme().startsWith("http")) return u;
    }

    static const QRegularExpression ogContentFirst(
        R"(<meta[^>]+content\s*=\s*["']([^"']{1,500})["'][^>]+property\s*=\s*["']og:image["'])",
        QRegularExpression::CaseInsensitiveOption);
    m = ogContentFirst.match(html);
    if (m.hasMatch()) {
        const QUrl u(m.captured(1).trimmed());
        if (u.isValid() && u.scheme().startsWith("http")) return u;
    }

    return {};
}
