#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>

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
    int remoteDesktopId;
};

Q_DECLARE_METATYPE(AppInfo)

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

struct ShortcutStat {
    QString shortcut;
    int useCount;
    QDateTime lastUsed;
};

enum TaskPriority {
    TaskPriority_Low,
    TaskPriority_Medium,
    TaskPriority_High
};

enum TaskStatus {
    TaskStatus_Todo,
    TaskStatus_InProgress,
    TaskStatus_Paused,
    TaskStatus_Completed
};

struct Task {
    QString id;
    QString title;
    QString description;
    int categoryId;
    TaskPriority priority;
    TaskStatus status;
    double workDuration;
    QDateTime completionTime;
    QStringList tags;
    QDateTime updatedAt;
};

struct Category {
    int id;
    QString name;
    QString description;
    int parentId;
    QString color;
    int sortOrder;
};

class Database : public QObject
{
    Q_OBJECT
signals:
    void appsChanged();
    void tasksChanged();
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool init();
    void setCurrentUser(int userId);
    int getCurrentUserId() const { return currentUserId; }
    
    bool addApp(const AppInfo &app);
    bool updateApp(const AppInfo &app);
    bool deleteApp(int id);
    QList<AppInfo> getAllApps();
    QList<AppInfo> getFavoriteApps();
    AppInfo getAppById(int id);
    int getMaxSortOrder();
    
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
    
    bool setShowBottomAppBar(bool show);
    bool getShowBottomAppBar();
    
    bool setAutoCheckUpdate(bool enabled);
    bool getAutoCheckUpdate();
    
    bool setShortcutKey(const QString &key);
    QString getShortcutKey();
    
    bool recordShortcutUsage(const QString &shortcut);
    QList<ShortcutStat> getShortcutStats();
    bool clearShortcutStats();
    
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

    bool addTask(const Task &task);
    bool addTaskWithId(const Task &task);  // 保留原有ID添加任务，用于同步
    bool updateTask(const Task &task);
    bool deleteTask(const QString &id);
    QList<Task> getAllTasks();
    QList<Task> getTasksForDate(const QDate &date);
    QList<Task> getTasksByStatus(TaskStatus status);
    QList<Task> getTasksByCategory(int categoryId);
    QList<Task> getTasksByDateRange(const QDateTime &startDate, const QDateTime &endDate);
    QList<Task> searchTasks(const QString &keyword);
    Task getTaskById(const QString &id);
    bool updateTaskStatus(const QString &id, TaskStatus status);
    bool updateTaskDuration(const QString &id, double duration);

    static QList<Category> getBuiltinCategories();

    QHash<QString, double> getCategoryWorkHours(const QDateTime &startDate, const QDateTime &endDate);
    QHash<QString, int> getCategoryTaskCount(const QDateTime &startDate, const QDateTime &endDate);
    double getTotalWorkHours(const QDateTime &startDate, const QDateTime &endDate);
    int getTotalTaskCount(const QDateTime &startDate, const QDateTime &endDate);

    // 公开方法供外部调用保存和转换任务数据
    bool saveTaskData();
    QJsonObject taskToJson(const Task &task);

private:
    QString dataFilePath;
    QString taskFilePath;
    int currentUserId;
    QJsonObject rootObject;
    QJsonObject taskRootObject;
    int nextAppId;
    int nextCollectionId;
    int nextRemoteDesktopId;
    int nextSnapshotId;

    bool loadData();
    bool saveData();
    bool loadTaskData();
    bool migrateTaskData();
    QJsonObject appToJson(const AppInfo &app);
    AppInfo jsonToApp(const QJsonObject &obj);
    QJsonObject collectionToJson(const AppCollection &collection);
    AppCollection jsonToCollection(const QJsonObject &obj);
    QJsonObject remoteDesktopToJson(const RemoteDesktopConnection &connection);
    RemoteDesktopConnection jsonToRemoteDesktop(const QJsonObject &obj);
    QJsonObject snapshotToJson(const SnapshotInfo &snapshot);
    SnapshotInfo jsonToSnapshot(const QJsonObject &obj);
    Task jsonToTask(const QJsonObject &obj);
    QString encryptPassword(const QString &password);
    QString decryptPassword(const QString &encrypted);
    QString generateTaskId();
    int getDailyTaskCount(const QString &dateStr);
    bool taskExists(const QString &taskId);
};

Q_DECLARE_METATYPE(RemoteDesktopConnection)
Q_DECLARE_METATYPE(SnapshotInfo)
Q_DECLARE_METATYPE(AppCollection)
Q_DECLARE_METATYPE(Task)
Q_DECLARE_METATYPE(Category)

#endif
