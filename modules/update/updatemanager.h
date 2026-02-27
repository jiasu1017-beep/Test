#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVersionNumber>
#include <QTimer>
#include <QUrl>
#include <QFile>
#include <QCryptographicHash>

struct UpdateInfo {
    QString version;
    QString releaseDate;
    QString changelog;
    QString downloadUrl;
    qint64 fileSize;
    bool isValid;
};

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);
    
    void checkForUpdates();
    void startPeriodicChecks();
    void stopPeriodicChecks();
    
    QString currentVersion() const;
    UpdateInfo latestUpdate() const;
    bool isUpdateAvailable() const;
    QString ignoredVersion() const;
    void setIgnoredVersion(const QString &version);
    
public slots:
    void downloadUpdate();
    void remindLater();
    void skipThisVersion();
    void installUpdate(const QString &filePath);
    void restartApplication();
    
signals:
    void checkForUpdatesStarted();
    void updateAvailable(const UpdateInfo &info);
    void noUpdateAvailable();
    void updateCheckFailed(const QString &error);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString &filePath);
    void downloadFailed(const QString &error);
    void extractProgress(int percent);
    void extractFinished(const QString &extractPath);
    void extractFailed(const QString &error);
    void installProgress(int percent);
    void installFinished();
    void installFailed(const QString &error);
    void logMessage(const QString &message);
    
private slots:
    void onReleaseInfoReceived(QNetworkReply *reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished(QNetworkReply *reply);
    void onDownloadError(QNetworkReply::NetworkError error);
    void onPeriodicCheck();
    void retryDownload();
    
private:
    void parseReleaseInfo(const QByteArray &data);
    bool compareVersions(const QString &local, const QString &remote) const;
    QString formatFileSize(qint64 bytes) const;
    bool verifyDownloadedFile(const QString &filePath, qint64 expectedSize);
    bool extractZipFile(const QString &zipPath, const QString &extractPath);
    bool backupCurrentVersion();
    bool replaceFiles(const QString &sourcePath, const QString &targetPath, const QStringList &filters = QStringList());
    void rollbackUpdate();
    void log(const QString &message);
    QString calculateFileHash(const QString &filePath, QCryptographicHash::Algorithm algorithm);
    
    QNetworkAccessManager *networkManager;
    QTimer *periodicTimer;
    QTimer *retryTimer;
    UpdateInfo m_latestUpdate;
    QString m_ignoredVersion;
    QString m_currentVersion;
    QString m_repoOwner;
    QString m_repoName;
    QString m_downloadedFilePath;
    int m_retryCount;
    int m_maxRetries;
    int m_retryDelay;
};

#endif
