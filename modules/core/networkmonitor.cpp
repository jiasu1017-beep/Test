#include "networkmonitor.h"
#include <QNetworkReply>
#include <QHostAddress>

NetworkMonitor* NetworkMonitor::s_instance = nullptr;

NetworkMonitor::NetworkMonitor(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_checkTimer(new QTimer(this))
    , m_isOnline(true)
    , m_isSyncing(false)
    , m_checkInterval(5000)
{
    // 连接网络状态变化信号
    connect(m_networkManager, &QNetworkAccessManager::networkAccessibleChanged,
            this, &NetworkMonitor::onNetworkAccessibleChanged);

    // 连接定时器检查
    connect(m_checkTimer, &QTimer::timeout,
            this, &NetworkMonitor::checkNetworkStatus);

    // 初始检查
    QTimer::singleShot(1000, this, &NetworkMonitor::checkNetworkStatus);
}

NetworkMonitor::~NetworkMonitor()
{
    m_checkTimer->stop();
}

NetworkMonitor* NetworkMonitor::instance()
{
    if (!s_instance) {
        s_instance = new NetworkMonitor();
    }
    return s_instance;
}

void NetworkMonitor::setCheckInterval(int intervalMs)
{
    m_checkInterval = intervalMs;
    m_checkTimer->setInterval(intervalMs);
}

void NetworkMonitor::startMonitoring()
{
    if (!m_checkTimer->isActive()) {
        m_checkTimer->start(m_checkInterval);
        qDebug() << "NetworkMonitor: Started monitoring with interval" << m_checkInterval << "ms";
    }
}

void NetworkMonitor::stopMonitoring()
{
    m_checkTimer->stop();
    qDebug() << "NetworkMonitor: Stopped monitoring";
}

void NetworkMonitor::setSyncing(bool syncing)
{
    if (m_isSyncing != syncing) {
        m_isSyncing = syncing;
        qDebug() << "NetworkMonitor: Syncing state changed to" << syncing;
    }
}

void NetworkMonitor::checkNetworkStatus()
{
    // 使用 QNetworkAccessManager 的网络可达性检查
    QNetworkAccessManager::NetworkAccessibility accessibility = m_networkManager->networkAccessible();

    bool online = (accessibility == QNetworkAccessManager::Accessible);
    if (!online) {
        // 尝试通过发送简单的请求来检查网络
        QNetworkRequest request;
        request.setUrl(QUrl("https://www.google.com/generate_204"));
        request.setHeader(QNetworkRequest::UserAgentHeader, "PonyWork/1.0");

        QNetworkReply* reply = m_networkManager->get(request);
        QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
            bool online = (reply->error() == QNetworkReply::NoError);
            reply->deleteLater();
            updateOnlineStatus(online);
        });

        // 设置超时
        QTimer::singleShot(3000, this, [this, reply]() {
            if (reply->isRunning()) {
                reply->abort();
                reply->deleteLater();
            }
        });
    } else {
        updateOnlineStatus(online);
    }
}

void NetworkMonitor::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    bool online = (accessible == QNetworkAccessManager::Accessible);
    updateOnlineStatus(online);
}

void NetworkMonitor::updateOnlineStatus(bool online)
{
    if (m_isOnline != online) {
        m_isOnline = online;
        qDebug() << "NetworkMonitor: Network status changed to" << (online ? "Online" : "Offline");
        emit networkStatusChanged(online);

        // 如果从离线变为在线，触发同步请求
        if (online && !m_isSyncing) {
            emit syncRequested();
        }
    }
}
