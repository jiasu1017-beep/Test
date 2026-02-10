#include "updatemanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMessageBox>
#include <QProcess>
#include <QSslSocket>
#include <QSslConfiguration>

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    periodicTimer = new QTimer(this);
    
    m_currentVersion = QCoreApplication::applicationVersion();
    m_repoOwner = "jiasu1017-beep";
    m_repoName = "Test";
    m_latestUpdate.isValid = false;
    
    connect(periodicTimer, &QTimer::timeout, this, &UpdateManager::onPeriodicCheck);
}

void UpdateManager::checkForUpdates()
{
    QString apiUrl = QString("https://api.github.com/repos/%1/%2/releases/latest")
                        .arg(m_repoOwner, m_repoName);
    
    QUrl url(apiUrl);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "PonyWork-Updater");
    
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReleaseInfoReceived(reply);
    });
}

void UpdateManager::startPeriodicChecks()
{
    periodicTimer->start(24 * 60 * 60 * 1000);
    checkForUpdates();
}

void UpdateManager::stopPeriodicChecks()
{
    periodicTimer->stop();
}

QString UpdateManager::currentVersion() const
{
    return m_currentVersion;
}

UpdateInfo UpdateManager::latestUpdate() const
{
    return m_latestUpdate;
}

bool UpdateManager::isUpdateAvailable() const
{
    if (!m_latestUpdate.isValid) {
        return false;
    }
    
    if (m_latestUpdate.version == m_ignoredVersion) {
        return false;
    }
    
    return compareVersions(m_currentVersion, m_latestUpdate.version);
}

QString UpdateManager::ignoredVersion() const
{
    return m_ignoredVersion;
}

void UpdateManager::setIgnoredVersion(const QString &version)
{
    m_ignoredVersion = version;
}

void UpdateManager::downloadUpdate()
{
    if (!m_latestUpdate.isValid || m_latestUpdate.downloadUrl.isEmpty()) {
        emit downloadFailed("没有有效的下载链接");
        return;
    }
    
    QUrl url(m_latestUpdate.downloadUrl);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "PonyWork-Updater");
    
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, &UpdateManager::onDownloadProgress);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onDownloadFinished(reply);
    });
}

void UpdateManager::remindLater()
{
}

void UpdateManager::skipThisVersion()
{
    if (m_latestUpdate.isValid) {
        m_ignoredVersion = m_latestUpdate.version;
    }
}

void UpdateManager::onReleaseInfoReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = reply->errorString();
        
        if (errorMsg.contains("TLS") || errorMsg.contains("SSL")) {
            errorMsg = "SSL/TLS 初始化失败\n\n"
                       "可能原因：\n"
                       "1. 缺少 OpenSSL 库文件\n"
                       "2. 系统不支持 HTTPS 连接\n\n"
                       "建议：请确保发布包包含 libssl-1_1-x64.dll 和 libcrypto-1_1-x64.dll";
        }
        
        emit updateCheckFailed(errorMsg);
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    parseReleaseInfo(data);
    reply->deleteLater();
    
    if (isUpdateAvailable()) {
        emit updateAvailable(m_latestUpdate);
    } else {
        emit noUpdateAvailable();
    }
}

void UpdateManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void UpdateManager::onDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit downloadFailed(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileName = QFileInfo(reply->url().path()).fileName();
    if (fileName.isEmpty()) {
        fileName = "PonyWork-update.zip";
    }
    
    QString filePath = tempDir + "/" + fileName;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit downloadFailed("无法保存文件: " + file.errorString());
        reply->deleteLater();
        return;
    }
    
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    
    emit downloadFinished(filePath);
}

void UpdateManager::onPeriodicCheck()
{
    checkForUpdates();
}

void UpdateManager::parseReleaseInfo(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        m_latestUpdate.isValid = false;
        return;
    }
    
    QJsonObject obj = doc.object();
    
    m_latestUpdate.version = obj["tag_name"].toString().remove('v');
    m_latestUpdate.releaseDate = obj["published_at"].toString();
    m_latestUpdate.changelog = obj["body"].toString();
    
    QJsonArray assets = obj["assets"].toArray();
    if (!assets.isEmpty()) {
        QJsonObject asset = assets[0].toObject();
        m_latestUpdate.downloadUrl = asset["browser_download_url"].toString();
        m_latestUpdate.fileSize = asset["size"].toVariant().toLongLong();
    }
    
    m_latestUpdate.isValid = !m_latestUpdate.version.isEmpty();
}

bool UpdateManager::compareVersions(const QString &local, const QString &remote) const
{
    QVersionNumber localVer = QVersionNumber::fromString(local);
    QVersionNumber remoteVer = QVersionNumber::fromString(remote);
    
    return remoteVer > localVer;
}

QString UpdateManager::formatFileSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = 1024 * KB;
    const qint64 GB = 1024 * MB;
    
    if (bytes >= GB) {
        return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
}
