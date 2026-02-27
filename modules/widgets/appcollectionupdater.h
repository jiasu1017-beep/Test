#ifndef APPCOLLECTIONUPDATER_H
#define APPCOLLECTIONUPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QVector>
#include "../core/appcollectiontypes.h"

class AppCollectionUpdater : public QObject
{
    Q_OBJECT
public:
    explicit AppCollectionUpdater(QObject *parent = nullptr);
    
    void checkForUpdates();
    void startPeriodicChecks();
    void stopPeriodicChecks();
    
    QVector<CategoryInfo> categories() const;
    QVector<RecommendedAppInfo> allApps() const;
    bool isUpdateAvailable() const;
    bool isUpdateInProgress() const;
    
public slots:
    void downloadUpdate();
    
signals:
    void updateCheckStarted();
    void updateAvailable(int appCount);
    void noUpdateAvailable();
    void updateCheckFailed(const QString &error);
    void updateProgress(int percent);
    void updateFinished();
    void updateFailed(const QString &error);
    void logMessage(const QString &message);
    
private slots:
    void onCollectionInfoReceived(QNetworkReply *reply);
    void onNetworkError(QNetworkReply::NetworkError error);
    void onPeriodicCheck();
    void retryUpdateCheck();
    
private:
    void parseCollectionInfo(const QByteArray &data);
    void extractAppsFromCategory(const QByteArray &data, const QString &categoryName, const QString &iconEmoji);
    bool parseHtmlContent(const QString &html, QVector<CategoryInfo> &parsedCategories);
    
    QNetworkAccessManager *networkManager;
    QTimer *periodicTimer;
    QTimer *retryTimer;
    QVector<CategoryInfo> m_categories;
    QVector<RecommendedAppInfo> m_allApps;
    QString m_collectionUrl;
    bool m_updateInProgress;
    bool m_updateAvailable;
    int m_retryCount;
    int m_maxRetries;
    int m_retryDelay;
    int m_checkInterval;
};

#endif // APPCOLLECTIONUPDATER_H
