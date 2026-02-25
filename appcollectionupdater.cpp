#include "appcollectionupdater.h"
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

AppCollectionUpdater::AppCollectionUpdater(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    periodicTimer = new QTimer(this);
    retryTimer = new QTimer(this);
    
    m_collectionUrl = "https://haiezan.github.io/page/collections/";
    m_updateInProgress = false;
    m_updateAvailable = false;
    m_retryCount = 0;
    m_maxRetries = 3;
    m_retryDelay = 5000;
    m_checkInterval = 3600000;
    
    connect(periodicTimer, &QTimer::timeout, this, &AppCollectionUpdater::onPeriodicCheck);
    connect(retryTimer, &QTimer::timeout, this, &AppCollectionUpdater::retryUpdateCheck);
}

void AppCollectionUpdater::checkForUpdates()
{
    if (m_updateInProgress) {
        emit logMessage("Update check already in progress");
        return;
    }
    
    m_updateInProgress = true;
    emit updateCheckStarted();
    emit logMessage("开始检查应用集合更新...");
    
    QNetworkRequest request;
    request.setUrl(QUrl(m_collectionUrl));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() { onCollectionInfoReceived(reply); });
    connect(reply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError error) { onNetworkError(error); });
}

void AppCollectionUpdater::startPeriodicChecks()
{
    periodicTimer->start(m_checkInterval);
    emit logMessage("已启动定期更新检查");
}

void AppCollectionUpdater::stopPeriodicChecks()
{
    periodicTimer->stop();
    emit logMessage("已停止定期更新检查");
}

QVector<CategoryInfo> AppCollectionUpdater::categories() const
{
    return m_categories;
}

QVector<RecommendedAppInfo> AppCollectionUpdater::allApps() const
{
    return m_allApps;
}

bool AppCollectionUpdater::isUpdateAvailable() const
{
    return m_updateAvailable;
}

bool AppCollectionUpdater::isUpdateInProgress() const
{
    return m_updateInProgress;
}

void AppCollectionUpdater::downloadUpdate()
{
    emit updateFinished();
    emit logMessage("应用集合更新完成");
}

void AppCollectionUpdater::onCollectionInfoReceived(QNetworkReply *reply)
{
    m_updateInProgress = false;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit updateCheckFailed(reply->errorString());
        emit logMessage("更新检查失败: " + reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    reply->deleteLater();
    
    parseCollectionInfo(data);
}

void AppCollectionUpdater::onNetworkError(QNetworkReply::NetworkError error)
{
    m_updateInProgress = false;
    
    QString errorString = "网络错误: " + QString::number(error);
    emit updateCheckFailed(errorString);
    emit logMessage(errorString);
    
    if (m_retryCount < m_maxRetries) {
        m_retryCount++;
        emit logMessage(QString("正在重试 (%1/%2)...").arg(m_retryCount).arg(m_maxRetries));
        retryTimer->start(m_retryDelay);
    } else {
        emit logMessage("达到最大重试次数，更新检查失败");
    }
}

void AppCollectionUpdater::onPeriodicCheck()
{
    checkForUpdates();
}

void AppCollectionUpdater::retryUpdateCheck()
{
    retryTimer->stop();
    checkForUpdates();
}

void AppCollectionUpdater::parseCollectionInfo(const QByteArray &data)
{
    QString html = QString(data);
    QVector<CategoryInfo> parsedCategories;
    
    if (parseHtmlContent(html, parsedCategories)) {
        if (parsedCategories != m_categories) {
            m_categories = parsedCategories;
            m_allApps.clear();
            
            for (const auto &category : m_categories) {
                for (const auto &app : category.apps) {
                    m_allApps.append(app);
                }
            }
            
            m_updateAvailable = true;
            emit updateAvailable(m_allApps.size());
            emit logMessage(QString("发现更新: %1 个应用分类，共 %2 个应用").arg(m_categories.size()).arg(m_allApps.size()));
        } else {
            m_updateAvailable = false;
            emit noUpdateAvailable();
            emit logMessage("当前已是最新版本");
        }
    } else {
        emit updateCheckFailed("解析网页内容失败");
        emit logMessage("解析网页内容失败");
    }
}

bool AppCollectionUpdater::parseHtmlContent(const QString &html, QVector<CategoryInfo> &parsedCategories)
{
    QMap<QString, QString> categoryMap = {
        {"结构", "🏗️"},
        {"编程", "💻"},
        {"教程", "📚"},
        {"论坛", "💬"},
        {"Github", "🐙"},
        {"网站搭建", "🌐"},
        {"文字处理", "📄"},
        {"GitBook 及其插件", "📖"},
        {"Chrome 插件", "🧩"},
        {"实用网站", "🔗"},
        {"工具", "🛠️"},
        {"开心一下", "😄"},
        {"Life", "🌈"},
        {"AI", "🤖"}
    };
    
    QRegularExpression categoryRegex("<h2[^>]*>([^<]+)</h2>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression liRegex("<li[^>]*>(.*?)</li>", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpression linkRegex("<a[^>]*href=\"([^\"]+)\"[^>]*>([^<]+)</a>", QRegularExpression::DotMatchesEverythingOption);
    
    int pos = 0;
    while (pos < html.length()) {
        QRegularExpressionMatch categoryMatch = categoryRegex.match(html, pos);
        if (!categoryMatch.hasMatch()) {
            break;
        }
        
        QString categoryName = categoryMatch.captured(1).trimmed();
        if (!categoryMap.contains(categoryName)) {
            pos = categoryMatch.capturedEnd();
            continue;
        }
        
        CategoryInfo category;
        category.name = categoryMap[categoryName] + " " + categoryName;
        category.iconEmoji = categoryMap[categoryName];
        
        pos = categoryMatch.capturedEnd();
        
        int nextCategoryPos = html.indexOf("<h2", pos);
        if (nextCategoryPos == -1) {
            nextCategoryPos = html.length();
        }
        
        QString categoryContent = html.mid(pos, nextCategoryPos - pos);
        
        int liPos = 0;
        while (liPos < categoryContent.length()) {
            QRegularExpressionMatch liMatch = liRegex.match(categoryContent, liPos);
            if (!liMatch.hasMatch()) {
                break;
            }
            
            QString liContent = liMatch.captured(1);
            
            QVector<QPair<QString, QString>> links;
            int linkPos = 0;
            while (linkPos < liContent.length()) {
                QRegularExpressionMatch linkMatch = linkRegex.match(liContent, linkPos);
                if (linkMatch.hasMatch()) {
                    QString url = linkMatch.captured(1);
                    QString linkText = linkMatch.captured(2).trimmed();
                    
                    if (!url.startsWith("http")) {
                        url = QUrl(m_collectionUrl).resolved(QUrl(url)).toString();
                    }
                    
                    links.append(qMakePair(url, linkText));
                    linkPos = linkMatch.capturedEnd();
                } else {
                    break;
                }
            }
            
            QString plainText = liContent;
            plainText.remove(QRegularExpression("<[^>]*>"));
            plainText = plainText.trimmed();
            
            if (!links.isEmpty()) {
                for (const auto &link : links) {
                    RecommendedAppInfo app;
                    app.name = link.second;
                    app.url = link.first;
                    app.description = plainText;
                    app.iconEmoji = "📦";
                    app.category = categoryName;
                    app.isFavorite = false;
                    
                    category.apps.append(app);
                }
            } else if (!plainText.isEmpty()) {
                RecommendedAppInfo app;
                app.name = plainText;
                app.url = "";
                app.description = "";
                app.iconEmoji = "📝";
                app.category = categoryName;
                app.isFavorite = false;
                
                category.apps.append(app);
            }
            
            liPos = liMatch.capturedEnd();
        }
        
        if (!category.apps.isEmpty()) {
            parsedCategories.append(category);
        }
        
        pos = nextCategoryPos;
    }
    
    return !parsedCategories.isEmpty();
}
