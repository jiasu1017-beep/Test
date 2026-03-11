#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QIcon>
#include <QFileInfo>
#include <QWidget>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <windows.h>
#include "database.h"

class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationManager(Database *db, QObject *parent = nullptr);
    ~ApplicationManager();

    enum LaunchResult {
        Success,
        InvalidApp,
        PathNotFound,
        LaunchFailed
    };
    Q_ENUM(LaunchResult)

    struct LaunchOptions {
        bool updateUseCount = true;
        bool refreshUI = true;
        bool silentMode = false;
    };

    static QIcon getAppIcon(const AppInfo &app);
    static QIcon getFileIcon(const QString &filePath);

    LaunchResult launchApp(const AppInfo &app, const LaunchOptions &options);
    LaunchResult launchApp(const AppInfo &app);
    LaunchResult launchAppById(int appId, const LaunchOptions &options);
    LaunchResult launchAppById(int appId);

    static void launchRemoteDesktop(const RemoteDesktopConnection &conn, Database *db = nullptr);

    static bool launchWebsite(const QString &url);
    static bool launchFolder(const QString &path);
    static bool launchDocument(const QString &path);
    static bool launchExecutable(const QString &path, const QString &arguments = QString());

    static bool isExecutable(const QString &path);
    static bool isWebsite(const QString &path);
    static bool isFolder(const QString &path);

    static QList<AppInfo> getAppsFromRegistry();
    static QList<AppInfo> getBookmarksFromBrowsers();
    static QList<AppInfo> getRunningApps();

signals:
    void appLaunched(const AppInfo &app);
    void launchFailed(const AppInfo &app, const QString &error);
    void useCountUpdated(const AppInfo &app);

private:
    bool launchWebsiteApp(const AppInfo &app, const LaunchOptions &options);
    bool launchFolderApp(const AppInfo &app, const LaunchOptions &options);
    bool launchDocumentApp(const AppInfo &app, const LaunchOptions &options);
    bool launchRemoteDesktopApp(const AppInfo &app, const LaunchOptions &options);
    bool launchExecutableApp(const AppInfo &app, const LaunchOptions &options);

    void updateAppUsage(const AppInfo &app);

    static void parseChromeBookmarks(const QJsonObject &root, QList<AppInfo> &bookmarks, const QString &source);
    static void parseChromeBookmarkFolder(const QJsonObject &folder, QList<AppInfo> &bookmarks, const QString &source);
    static void parseFirefoxBookmarks(const QString &dbPath, QList<AppInfo> &bookmarks);
    static QString getProcessPath(DWORD processID);

    Database *m_db;
};

#endif // APPLICATIONMANAGER_H