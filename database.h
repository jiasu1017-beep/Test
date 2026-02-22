#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

enum AppType {
    AppType_Executable,
    AppType_Website,
    AppType_Folder,
    AppType_Document,
    AppType_RemoteDesktop
};

enum SnapshotType {
    SnapshotType_Folder,
    SnapshotType_Website,
    SnapshotType_Document
};

struct AppInfo {
    int id;
    QString name;
    QString path;
    QString arguments;
    QString iconPath;
    QString category;
    int useCount;
    bool isFavorite;
    int sortOrder;
    AppType type;
    bool isRemoteDesktop;
    int remoteDesktopId;
};

struct SnapshotInfo {
    int id;
    QString name;
    QString path;
    QString description;
    SnapshotType type;
    QString thumbnailPath;
    QDateTime createdTime;
    QDateTime lastAccessedTime;
    QString folderStructure;
    QString fileTypeDistribution;
    QString websiteTitle;
    QString websiteUrl;
    QString documentTitle;
    QString documentAuthor;
    QString documentModifiedDate;
    int fileCount;
    qint64 totalSize;
    QString tags;
    bool isFavorite;
    int sortOrder;
};

struct AppCollection {
    int id;
    QString name;
    QString description;
    QList<int> appIds;
    QString tag;
    int sortPriority;
};

struct RemoteDesktopConnection {
    int id;
    QString name;
    QString hostAddress;
    int port;
    QString username;
    QString password;
    QString domain;
    QString displayName;
    int screenWidth;
    int screenHeight;
    bool fullScreen;
    bool useAllMonitors;
    bool enableAudio;
    bool enableClipboard;
    bool enablePrinter;
    bool enableDrive;
    QString notes;
    QString category;
    int sortOrder;
    bool isFavorite;
    QDateTime createdTime;
    QDateTime lastUsedTime;
};

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool init();
    
    bool addApp(const AppInfo &app);
    bool updateApp(const AppInfo &app);
    bool deleteApp(int id);
    QList<AppInfo> getAllApps();
    QList<AppInfo> getFavoriteApps();
    AppInfo getAppById(int id);
    
    bool addCollection(const AppCollection &collection);
    bool updateCollection(const AppCollection &collection);
    bool deleteCollection(int id);
    QList<AppCollection> getAllCollections();
    AppCollection getCollectionById(int id);
    
    bool setAutoStart(bool enabled);
    bool getAutoStart();
    
    bool setMinimizeToTray(bool enabled);
    bool getMinimizeToTray();
    
    bool setShowClosePrompt(bool show);
    bool getShowClosePrompt();
    
    bool setAutoCheckUpdate(bool enabled);
    bool getAutoCheckUpdate();
    
    bool setIgnoredVersion(const QString &version);
    QString getIgnoredVersion();

    bool addRemoteDesktop(const RemoteDesktopConnection &connection);
    bool updateRemoteDesktop(const RemoteDesktopConnection &connection);
    bool deleteRemoteDesktop(int id);
    QList<RemoteDesktopConnection> getAllRemoteDesktops();
    QList<RemoteDesktopConnection> getFavoriteRemoteDesktops();
    RemoteDesktopConnection getRemoteDesktopById(int id);
    QList<RemoteDesktopConnection> searchRemoteDesktops(const QString &keyword);

    bool addSnapshot(const SnapshotInfo &snapshot);
    bool updateSnapshot(const SnapshotInfo &snapshot);
    bool deleteSnapshot(int id);
    QList<SnapshotInfo> getAllSnapshots();
    QList<SnapshotInfo> getSnapshotsByType(SnapshotType type);
    QList<SnapshotInfo> getFavoriteSnapshots();
    SnapshotInfo getSnapshotById(int id);
    QList<SnapshotInfo> searchSnapshots(const QString &keyword);

private:
    QString dataFilePath;
    QJsonObject rootObject;
    int nextAppId;
    int nextCollectionId;
    int nextRemoteDesktopId;
    int nextSnapshotId;
    
    bool loadData();
    bool saveData();
    QJsonObject appToJson(const AppInfo &app);
    AppInfo jsonToApp(const QJsonObject &obj);
    QJsonObject collectionToJson(const AppCollection &collection);
    AppCollection jsonToCollection(const QJsonObject &obj);
    QJsonObject remoteDesktopToJson(const RemoteDesktopConnection &connection);
    RemoteDesktopConnection jsonToRemoteDesktop(const QJsonObject &obj);
    QJsonObject snapshotToJson(const SnapshotInfo &snapshot);
    SnapshotInfo jsonToSnapshot(const QJsonObject &obj);
    QString encryptPassword(const QString &password);
    QString decryptPassword(const QString &encrypted);
};

#endif
