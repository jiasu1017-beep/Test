#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVersionNumber>
#include <QTimer>
#include <QUrl>

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
    
signals:
    void updateAvailable(const UpdateInfo &info);
    void noUpdateAvailable();
    void updateCheckFailed(const QString &error);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString &filePath);
    void downloadFailed(const QString &error);
    
private slots:
    void onReleaseInfoReceived(QNetworkReply *reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished(QNetworkReply *reply);
    void onPeriodicCheck();
    
private:
    void parseReleaseInfo(const QByteArray &data);
    bool compareVersions(const QString &local, const QString &remote) const;
    QString formatFileSize(qint64 bytes) const;
    
    QNetworkAccessManager *networkManager;
    QTimer *periodicTimer;
    UpdateInfo m_latestUpdate;
    QString m_ignoredVersion;
    QString m_currentVersion;
    QString m_repoOwner;
    QString m_repoName;
};

#endif
