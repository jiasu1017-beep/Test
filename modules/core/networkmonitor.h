#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QDebug>

class NetworkMonitor : public QObject
{
    Q_OBJECT

public:
    explicit NetworkMonitor(QObject *parent = nullptr);
    ~NetworkMonitor();

    static NetworkMonitor* instance();

    bool isOnline() const { return m_isOnline; }
    bool isSyncing() const { return m_isSyncing; }

    // 设置检查间隔（毫秒），默认5000ms
    void setCheckInterval(int intervalMs);
    int getCheckInterval() const { return m_checkInterval; }

signals:
    void networkStatusChanged(bool online);
    void syncRequested();  // 联网后自动触发同步

public slots:
    void startMonitoring();
    void stopMonitoring();
    void setSyncing(bool syncing);

private slots:
    void checkNetworkStatus();
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);

private:
    void updateOnlineStatus(bool online);

    static NetworkMonitor* s_instance;

    QNetworkAccessManager* m_networkManager;
    QTimer* m_checkTimer;
    bool m_isOnline;
    bool m_isSyncing;
    int m_checkInterval;
};

#endif // NETWORKMONITOR_H
