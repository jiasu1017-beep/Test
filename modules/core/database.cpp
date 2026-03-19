#include "database.h"
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <algorithm>

#ifdef QT_DEBUG
static void runTaskFilterTests();
#endif

Database::Database(QObject *parent) : QObject(parent)
{
    nextAppId = 1;
    nextCollectionId = 1;
    nextRemoteDesktopId = 1;
    currentUserId = 0;
}

Database::~Database()
{
}

bool Database::init()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    dataFilePath = dataPath + "/data.json";
    taskFilePath = dataPath + "/tasks.json";

    if (!loadData()) {
        rootObject = QJsonObject();
        rootObject["apps"] = QJsonArray();
        rootObject["collections"] = QJsonArray();
        rootObject["remoteDesktops"] = QJsonArray();
        rootObject["snapshots"] = QJsonArray();
        rootObject["tasks"] = QJsonArray();
        rootObject["settings"] = QJsonObject();
        rootObject["nextAppId"] = 1;
        rootObject["nextCollectionId"] = 1;
        rootObject["nextRemoteDesktopId"] = 1;
        rootObject["nextSnapshotId"] = 1;
        saveData();
    }

    // 加载任务数据（不自动迁移，让用户自己选择是否保留旧数据）
    loadTaskData();

#ifdef QT_DEBUG
    runTaskFilterTests();
#endif

    return true;
}

bool Database::loadData()
{
    QFile file(dataFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }
    
    rootObject = doc.object();
    nextAppId = rootObject["nextAppId"].toInt(1);
    nextCollectionId = rootObject["nextCollectionId"].toInt(1);
    nextRemoteDesktopId = rootObject["nextRemoteDesktopId"].toInt(1);
    nextSnapshotId = rootObject["nextSnapshotId"].toInt(1);

    if (!rootObject.contains("snapshots")) {
        rootObject["snapshots"] = QJsonArray();
    }

    if (!rootObject.contains("tasks")) {
        rootObject["tasks"] = QJsonArray();
    }

    return true;
}

bool Database::saveData()
{
    rootObject["nextAppId"] = nextAppId;
    rootObject["nextCollectionId"] = nextCollectionId;
    rootObject["nextRemoteDesktopId"] = nextRemoteDesktopId;
    rootObject["nextSnapshotId"] = nextSnapshotId;

    QJsonDocument doc(rootObject);
    QFile file(dataFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Cannot save data file: %s", qPrintable(file.errorString()));
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

QJsonObject Database::appToJson(const AppInfo &app)
{
    QJsonObject obj;
    obj["id"] = app.id;
    obj["name"] = app.name;
    obj["path"] = app.path;
    obj["arguments"] = app.arguments;
    obj["iconPath"] = app.iconPath;
    obj["category"] = app.category;
    obj["useCount"] = app.useCount;
    obj["isFavorite"] = app.isFavorite;
    obj["sortOrder"] = app.sortOrder;
    obj["type"] = static_cast<int>(app.type);
    obj["remoteDesktopId"] = app.remoteDesktopId;
    return obj;
}

AppInfo Database::jsonToApp(const QJsonObject &obj)
{
    AppInfo app;
    app.id = obj["id"].toInt();
    app.name = obj["name"].toString();
    app.path = obj["path"].toString();
    app.arguments = obj["arguments"].toString();
    app.iconPath = obj["iconPath"].toString();
    app.category = obj["category"].toString();
    app.useCount = obj["useCount"].toInt();
    app.isFavorite = obj["isFavorite"].toBool();
    app.sortOrder = obj["sortOrder"].toInt();

    int typeInt = obj["type"].toInt(0);
    if (typeInt < 0 || typeInt > 4) typeInt = 0;
    app.type = static_cast<AppType>(typeInt);

    app.remoteDesktopId = obj["remoteDesktopId"].toInt(-1);

    return app;
}

QJsonObject Database::collectionToJson(const AppCollection &collection)
{
    QJsonObject obj;
    obj["id"] = collection.id;
    obj["name"] = collection.name;
    obj["description"] = collection.description;
    obj["tag"] = collection.tag;
    obj["sortPriority"] = collection.sortPriority;
    
    QJsonArray appsArray = rootObject["apps"].toArray();
    QSet<int> validAppIds;
    for (const QJsonValue &val : appsArray) {
        QJsonObject appObj = val.toObject();
        validAppIds.insert(appObj["id"].toInt());
    }
    
    QJsonArray idArray;
    for (int id : collection.appIds) {
        if (validAppIds.contains(id)) {
            idArray.append(id);
        }
    }
    obj["appIds"] = idArray;
    return obj;
}

AppCollection Database::jsonToCollection(const QJsonObject &obj)
{
    AppCollection col;
    col.id = obj["id"].toInt();
    col.name = obj["name"].toString();
    col.description = obj["description"].toString();
    QString tag = obj["tag"].toString("未分类");
    if (tag == "上班") {
        col.tag = "工作";
    } else {
        col.tag = tag;
    }
    col.sortPriority = obj["sortPriority"].toInt(0);
    
    QJsonArray appsArray = rootObject["apps"].toArray();
    QSet<int> validAppIds;
    for (const QJsonValue &val : appsArray) {
        QJsonObject appObj = val.toObject();
        validAppIds.insert(appObj["id"].toInt());
    }
    
    QJsonArray idArray = obj["appIds"].toArray();
    for (const QJsonValue &val : idArray) {
        int appId = val.toInt();
        if (validAppIds.contains(appId)) {
            col.appIds.append(appId);
        }
    }
    return col;
}

bool Database::addApp(const AppInfo &app)
{
    AppInfo newApp = app;
    newApp.id = nextAppId++;
    
    QJsonArray appsArray = rootObject["apps"].toArray();
    appsArray.append(appToJson(newApp));
    rootObject["apps"] = appsArray;
    
    bool result = saveData();
    if (result) {
        emit appsChanged();
    }
    return result;
}

bool Database::updateApp(const AppInfo &app)
{
    QJsonArray appsArray = rootObject["apps"].toArray();
    
    for (int i = 0; i < appsArray.size(); ++i) {
        if (appsArray[i].toObject()["id"].toInt() == app.id) {
            appsArray[i] = appToJson(app);
            break;
        }
    }
    
    rootObject["apps"] = appsArray;
    
    bool result = saveData();
    if (result) {
        emit appsChanged();
    }
    return result;
}

bool Database::deleteApp(int id)
{
    QJsonArray appsArray = rootObject["apps"].toArray();
    
    for (int i = 0; i < appsArray.size(); ++i) {
        if (appsArray[i].toObject()["id"].toInt() == id) {
            appsArray.removeAt(i);
            break;
        }
    }
    
    rootObject["apps"] = appsArray;
    
    QJsonArray colsArray = rootObject["collections"].toArray();
    for (int i = 0; i < colsArray.size(); ++i) {
        QJsonObject colObj = colsArray[i].toObject();
        QJsonArray idArray = colObj["appIds"].toArray();
        QJsonArray newIdArray;
        for (const QJsonValue &val : idArray) {
            if (val.toInt() != id) {
                newIdArray.append(val);
            }
        }
        colObj["appIds"] = newIdArray;
        colsArray[i] = colObj;
    }
    rootObject["collections"] = colsArray;
    
    bool result = saveData();
    if (result) {
        emit appsChanged();
    }
    return result;
}

QList<AppInfo> Database::getAllApps()
{
    QList<AppInfo> apps;
    QJsonArray appsArray = rootObject["apps"].toArray();
    
    QList<QJsonObject> appObjects;
    for (const QJsonValue &val : appsArray) {
        appObjects.append(val.toObject());
    }
    
    std::sort(appObjects.begin(), appObjects.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a["sortOrder"].toInt() < b["sortOrder"].toInt();
    });
    
    for (const QJsonObject &obj : appObjects) {
        apps.append(jsonToApp(obj));
    }
    
    return apps;
}

QList<AppInfo> Database::getFavoriteApps()
{
    QList<AppInfo> apps;
    QJsonArray appsArray = rootObject["apps"].toArray();
    
    QList<QJsonObject> appObjects;
    for (const QJsonValue &val : appsArray) {
        QJsonObject obj = val.toObject();
        if (obj["isFavorite"].toBool()) {
            appObjects.append(obj);
        }
    }
    
    std::sort(appObjects.begin(), appObjects.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a["sortOrder"].toInt() < b["sortOrder"].toInt();
    });
    
    for (const QJsonObject &obj : appObjects) {
        apps.append(jsonToApp(obj));
    }
    
    return apps;
}

AppInfo Database::getAppById(int id)
{
    AppInfo app;
    app.id = -1;
    
    QJsonArray appsArray = rootObject["apps"].toArray();
    
    for (const QJsonValue &val : appsArray) {
        QJsonObject obj = val.toObject();
        if (obj["id"].toInt() == id) {
            app = jsonToApp(obj);
            break;
        }
    }
    
    return app;
}

int Database::getMaxSortOrder()
{
    int maxOrder = 0;
    QList<AppInfo> apps = getAllApps();
    
    for (const AppInfo &app : apps) {
        if (app.sortOrder > maxOrder) {
            maxOrder = app.sortOrder;
        }
    }
    
    return maxOrder;
}

bool Database::addCollection(const AppCollection &collection)
{
    AppCollection newCol = collection;
    newCol.id = nextCollectionId++;
    
    QJsonArray colsArray = rootObject["collections"].toArray();
    colsArray.append(collectionToJson(newCol));
    rootObject["collections"] = colsArray;
    
    return saveData();
}

bool Database::updateCollection(const AppCollection &collection)
{
    QJsonArray colsArray = rootObject["collections"].toArray();
    
    for (int i = 0; i < colsArray.size(); ++i) {
        QJsonObject obj = colsArray[i].toObject();
        if (obj["id"].toInt() == collection.id) {
            colsArray[i] = collectionToJson(collection);
            break;
        }
    }
    
    rootObject["collections"] = colsArray;
    
    return saveData();
}

bool Database::deleteCollection(int id)
{
    QJsonArray colsArray = rootObject["collections"].toArray();
    
    for (int i = 0; i < colsArray.size(); ++i) {
        QJsonObject obj = colsArray[i].toObject();
        if (obj["id"].toInt() == id) {
            colsArray.removeAt(i);
            break;
        }
    }
    
    rootObject["collections"] = colsArray;
    
    return saveData();
}

QList<AppCollection> Database::getAllCollections()
{
    QList<AppCollection> cols;
    QJsonArray colsArray = rootObject["collections"].toArray();
    
    for (const QJsonValue &val : colsArray) {
        cols.append(jsonToCollection(val.toObject()));
    }
    
    return cols;
}

AppCollection Database::getCollectionById(int id)
{
    AppCollection col;
    col.id = -1;
    
    QJsonArray colsArray = rootObject["collections"].toArray();
    
    for (const QJsonValue &val : colsArray) {
        QJsonObject obj = val.toObject();
        if (obj["id"].toInt() == id) {
            col = jsonToCollection(obj);
            break;
        }
    }
    
    return col;
}

bool Database::setAutoStart(bool enabled)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                       QSettings::NativeFormat);
    
    if (enabled) {
        settings.setValue("PonyWork", QCoreApplication::applicationFilePath().replace("/", "\\"));
    } else {
        settings.remove("PonyWork");
    }
    
    QJsonObject settingsObj = rootObject["settings"].toObject();
    settingsObj["auto_start"] = enabled ? "1" : "0";
    rootObject["settings"] = settingsObj;
    
    saveData();
    
    return true;
}

bool Database::getAutoStart()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                       QSettings::NativeFormat);
    return settings.contains("PonyWork");
}

bool Database::setMinimizeToTray(bool enabled)
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    settingsObj["minimize_to_tray"] = enabled ? "1" : "0";
    rootObject["settings"] = settingsObj;
    
    return saveData();
}

bool Database::getMinimizeToTray()
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    return settingsObj["minimize_to_tray"].toString("1") == "1";
}

bool Database::setShowClosePrompt(bool show)
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    settingsObj["show_close_prompt"] = show ? "1" : "0";
    rootObject["settings"] = settingsObj;
    
    return saveData();
}

bool Database::getShowClosePrompt()
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    return settingsObj["show_close_prompt"].toString("1") == "1";
}

bool Database::setAutoCheckUpdate(bool enabled)
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    settingsObj["auto_check_update"] = enabled ? "1" : "0";
    rootObject["settings"] = settingsObj;
    
    return saveData();
}

bool Database::getAutoCheckUpdate()
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    return settingsObj["auto_check_update"].toString("1") == "1";
}

bool Database::setShowBottomAppBar(bool show)
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    settingsObj["show_bottom_app_bar"] = show ? "1" : "0";
    rootObject["settings"] = settingsObj;
    
    return saveData();
}

bool Database::getShowBottomAppBar()
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    return settingsObj["show_bottom_app_bar"].toString("1") == "1";
}

bool Database::setShortcutKey(const QString &key)
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    settingsObj["shortcut_key"] = key;
    rootObject["settings"] = settingsObj;
    
    return saveData();
}

QString Database::getShortcutKey()
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    return settingsObj["shortcut_key"].toString("Ctrl+W");
}

bool Database::recordShortcutUsage(const QString &shortcut)
{
    QJsonArray statsArray = rootObject["shortcut_stats"].toArray();
    
    QDateTime now = QDateTime::currentDateTime();
    bool found = false;
    
    for (int i = 0; i < statsArray.size(); ++i) {
        QJsonObject statObj = statsArray[i].toObject();
        if (statObj["shortcut"].toString() == shortcut) {
            statObj["use_count"] = statObj["use_count"].toInt(0) + 1;
            statObj["last_used"] = now.toString(Qt::ISODate);
            statsArray[i] = statObj;
            found = true;
            break;
        }
    }
    
    if (!found) {
        QJsonObject newStat;
        newStat["shortcut"] = shortcut;
        newStat["use_count"] = 1;
        newStat["last_used"] = now.toString(Qt::ISODate);
        statsArray.append(newStat);
    }
    
    rootObject["shortcut_stats"] = statsArray;
    return saveData();
}

QList<ShortcutStat> Database::getShortcutStats()
{
    QList<ShortcutStat> stats;
    QJsonArray statsArray = rootObject["shortcut_stats"].toArray();
    
    for (const auto &value : statsArray) {
        QJsonObject statObj = value.toObject();
        ShortcutStat stat;
        stat.shortcut = statObj["shortcut"].toString();
        stat.useCount = statObj["use_count"].toInt(0);
        QString lastUsedStr = statObj["last_used"].toString();
        if (!lastUsedStr.isEmpty()) {
            stat.lastUsed = QDateTime::fromString(lastUsedStr, Qt::ISODate);
        }
        stats.append(stat);
    }
    
    // Sort by use count descending
    std::sort(stats.begin(), stats.end(), [](const ShortcutStat &a, const ShortcutStat &b) {
        return a.useCount > b.useCount;
    });
    
    return stats;
}

bool Database::clearShortcutStats()
{
    rootObject["shortcut_stats"] = QJsonArray();
    return saveData();
}

bool Database::setIgnoredVersion(const QString &version)
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    settingsObj["ignored_version"] = version;
    rootObject["settings"] = settingsObj;
    
    return saveData();
}

QString Database::getIgnoredVersion()
{
    QJsonObject settingsObj = rootObject["settings"].toObject();
    return settingsObj["ignored_version"].toString("");
}

QString Database::encryptPassword(const QString &password)
{
    QByteArray data = password.toUtf8();
    QByteArray key = "PonyWorkRemoteDesktopSecretKey2024";
    QByteArray result;
    
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ key[i % key.size()]);
    }
    
    return result.toBase64();
}

QString Database::decryptPassword(const QString &encrypted)
{
    QByteArray data = QByteArray::fromBase64(encrypted.toUtf8());
    QByteArray key = "PonyWorkRemoteDesktopSecretKey2024";
    QByteArray result;
    
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ key[i % key.size()]);
    }
    
    return QString::fromUtf8(result);
}

QJsonObject Database::remoteDesktopToJson(const RemoteDesktopConnection &connection)
{
    QJsonObject obj;
    obj["id"] = connection.id;
    obj["name"] = connection.name;
    obj["hostAddress"] = connection.hostAddress;
    obj["port"] = connection.port;
    obj["username"] = connection.username;
    obj["password"] = encryptPassword(connection.password);
    obj["domain"] = connection.domain;
    obj["displayName"] = connection.displayName;
    obj["screenWidth"] = connection.screenWidth;
    obj["screenHeight"] = connection.screenHeight;
    obj["fullScreen"] = connection.fullScreen;
    obj["useAllMonitors"] = connection.useAllMonitors;
    obj["enableAudio"] = connection.enableAudio;
    obj["enableClipboard"] = connection.enableClipboard;
    obj["enablePrinter"] = connection.enablePrinter;
    obj["enableDrive"] = connection.enableDrive;
    obj["notes"] = connection.notes;
    obj["category"] = connection.category;
    obj["sortOrder"] = connection.sortOrder;
    obj["isFavorite"] = connection.isFavorite;
    obj["createdTime"] = connection.createdTime.toString(Qt::ISODate);
    obj["lastUsedTime"] = connection.lastUsedTime.toString(Qt::ISODate);
    return obj;
}

RemoteDesktopConnection Database::jsonToRemoteDesktop(const QJsonObject &obj)
{
    RemoteDesktopConnection conn;
    conn.id = obj["id"].toInt();
    conn.name = obj["name"].toString();
    conn.hostAddress = obj["hostAddress"].toString();
    conn.port = obj["port"].toInt(3389);
    conn.username = obj["username"].toString();
    conn.password = decryptPassword(obj["password"].toString());
    conn.domain = obj["domain"].toString();
    conn.displayName = obj["displayName"].toString();
    conn.screenWidth = obj["screenWidth"].toInt(1920);
    conn.screenHeight = obj["screenHeight"].toInt(1080);
    conn.fullScreen = obj["fullScreen"].toBool(false);
    conn.useAllMonitors = obj["useAllMonitors"].toBool(false);
    conn.enableAudio = obj["enableAudio"].toBool(true);
    conn.enableClipboard = obj["enableClipboard"].toBool(true);
    conn.enablePrinter = obj["enablePrinter"].toBool(false);
    conn.enableDrive = obj["enableDrive"].toBool(false);
    conn.notes = obj["notes"].toString();
    conn.category = obj["category"].toString("未分类");
    conn.sortOrder = obj["sortOrder"].toInt(0);
    conn.isFavorite = obj["isFavorite"].toBool(false);
    conn.createdTime = QDateTime::fromString(obj["createdTime"].toString(), Qt::ISODate);
    conn.lastUsedTime = QDateTime::fromString(obj["lastUsedTime"].toString(), Qt::ISODate);
    return conn;
}

bool Database::addRemoteDesktop(const RemoteDesktopConnection &connection)
{
    RemoteDesktopConnection newConn = connection;
    newConn.id = nextRemoteDesktopId++;
    newConn.createdTime = QDateTime::currentDateTime();
    newConn.lastUsedTime = QDateTime::currentDateTime();
    
    QJsonArray rdsArray = rootObject["remoteDesktops"].toArray();
    
    if (newConn.sortOrder < 0 || newConn.sortOrder >= rdsArray.count()) {
        newConn.sortOrder = rdsArray.count();
    }
    
    rdsArray.append(remoteDesktopToJson(newConn));
    rootObject["remoteDesktops"] = rdsArray;
    
    return saveData();
}

bool Database::updateRemoteDesktop(const RemoteDesktopConnection &connection)
{
    QJsonArray rdsArray = rootObject["remoteDesktops"].toArray();
    
    for (int i = 0; i < rdsArray.size(); ++i) {
        QJsonObject obj = rdsArray[i].toObject();
        if (obj["id"].toInt() == connection.id) {
            rdsArray[i] = remoteDesktopToJson(connection);
            break;
        }
    }
    
    rootObject["remoteDesktops"] = rdsArray;
    
    return saveData();
}

bool Database::deleteRemoteDesktop(int id)
{
    QJsonArray rdsArray = rootObject["remoteDesktops"].toArray();
    
    for (int i = 0; i < rdsArray.size(); ++i) {
        QJsonObject obj = rdsArray[i].toObject();
        if (obj["id"].toInt() == id) {
            rdsArray.removeAt(i);
            break;
        }
    }
    
    rootObject["remoteDesktops"] = rdsArray;
    
    return saveData();
}

QList<RemoteDesktopConnection> Database::getAllRemoteDesktops()
{
    QList<RemoteDesktopConnection> connections;
    QJsonArray rdsArray = rootObject["remoteDesktops"].toArray();
    
    QList<QJsonObject> rdObjects;
    for (const QJsonValue &val : rdsArray) {
        rdObjects.append(val.toObject());
    }
    
    std::sort(rdObjects.begin(), rdObjects.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a["sortOrder"].toInt() < b["sortOrder"].toInt();
    });
    
    for (const QJsonObject &obj : rdObjects) {
        connections.append(jsonToRemoteDesktop(obj));
    }
    
    return connections;
}

QJsonObject Database::snapshotToJson(const SnapshotInfo &snapshot)
{
    QJsonObject obj;
    obj["id"] = snapshot.id;
    obj["name"] = snapshot.name;
    obj["path"] = snapshot.path;
    obj["description"] = snapshot.description;
    obj["type"] = static_cast<int>(snapshot.type);
    obj["thumbnailPath"] = snapshot.thumbnailPath;
    obj["createdTime"] = snapshot.createdTime.toString(Qt::ISODate);
    obj["lastAccessedTime"] = snapshot.lastAccessedTime.toString(Qt::ISODate);
    obj["folderStructure"] = snapshot.folderStructure;
    obj["fileTypeDistribution"] = snapshot.fileTypeDistribution;
    obj["websiteTitle"] = snapshot.websiteTitle;
    obj["websiteUrl"] = snapshot.websiteUrl;
    obj["documentTitle"] = snapshot.documentTitle;
    obj["documentAuthor"] = snapshot.documentAuthor;
    obj["documentModifiedDate"] = snapshot.documentModifiedDate;
    obj["fileCount"] = snapshot.fileCount;
    obj["totalSize"] = static_cast<qint64>(snapshot.totalSize);
    obj["tags"] = snapshot.tags;
    obj["isFavorite"] = snapshot.isFavorite;
    obj["sortOrder"] = snapshot.sortOrder;
    return obj;
}

SnapshotInfo Database::jsonToSnapshot(const QJsonObject &obj)
{
    SnapshotInfo snapshot;
    snapshot.id = obj["id"].toInt(-1);
    snapshot.name = obj["name"].toString();
    snapshot.path = obj["path"].toString();
    snapshot.description = obj["description"].toString();
    
    int typeInt = obj["type"].toInt(0);
    if (typeInt < 0 || typeInt > 2) typeInt = 0;
    snapshot.type = static_cast<SnapshotType>(typeInt);
    
    snapshot.thumbnailPath = obj["thumbnailPath"].toString();
    snapshot.createdTime = QDateTime::fromString(obj["createdTime"].toString(), Qt::ISODate);
    snapshot.lastAccessedTime = QDateTime::fromString(obj["lastAccessedTime"].toString(), Qt::ISODate);
    snapshot.folderStructure = obj["folderStructure"].toString();
    snapshot.fileTypeDistribution = obj["fileTypeDistribution"].toString();
    snapshot.websiteTitle = obj["websiteTitle"].toString();
    snapshot.websiteUrl = obj["websiteUrl"].toString();
    snapshot.documentTitle = obj["documentTitle"].toString();
    snapshot.documentAuthor = obj["documentAuthor"].toString();
    snapshot.documentModifiedDate = obj["documentModifiedDate"].toString();
    snapshot.fileCount = obj["fileCount"].toInt(0);
    snapshot.totalSize = static_cast<qint64>(obj["totalSize"].toDouble(0));
    snapshot.tags = obj["tags"].toString();
    snapshot.isFavorite = obj["isFavorite"].toBool(false);
    snapshot.sortOrder = obj["sortOrder"].toInt(0);
    
    return snapshot;
}

bool Database::addSnapshot(const SnapshotInfo &snapshot)
{
    SnapshotInfo newSnapshot = snapshot;
    newSnapshot.id = nextSnapshotId++;
    
    QJsonArray snapshotsArray = rootObject["snapshots"].toArray();
    snapshotsArray.append(snapshotToJson(newSnapshot));
    rootObject["snapshots"] = snapshotsArray;
    
    return saveData();
}

bool Database::updateSnapshot(const SnapshotInfo &snapshot)
{
    QJsonArray snapshotsArray = rootObject["snapshots"].toArray();
    
    for (int i = 0; i < snapshotsArray.size(); ++i) {
        QJsonObject obj = snapshotsArray[i].toObject();
        if (obj["id"].toInt() == snapshot.id) {
            snapshotsArray[i] = snapshotToJson(snapshot);
            break;
        }
    }
    
    rootObject["snapshots"] = snapshotsArray;
    
    return saveData();
}

bool Database::deleteSnapshot(int id)
{
    QJsonArray snapshotsArray = rootObject["snapshots"].toArray();
    
    for (int i = 0; i < snapshotsArray.size(); ++i) {
        QJsonObject obj = snapshotsArray[i].toObject();
        if (obj["id"].toInt() == id) {
            snapshotsArray.removeAt(i);
            break;
        }
    }
    
    rootObject["snapshots"] = snapshotsArray;
    
    return saveData();
}

QList<SnapshotInfo> Database::getAllSnapshots()
{
    QList<SnapshotInfo> snapshots;
    QJsonArray snapshotsArray = rootObject["snapshots"].toArray();
    
    QList<QJsonObject> snapshotObjects;
    for (const QJsonValue &val : snapshotsArray) {
        snapshotObjects.append(val.toObject());
    }
    
    std::sort(snapshotObjects.begin(), snapshotObjects.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a["sortOrder"].toInt() < b["sortOrder"].toInt();
    });
    
    for (const QJsonObject &obj : snapshotObjects) {
        snapshots.append(jsonToSnapshot(obj));
    }
    
    return snapshots;
}

QList<SnapshotInfo> Database::getSnapshotsByType(SnapshotType type)
{
    QList<SnapshotInfo> allSnapshots = getAllSnapshots();
    QList<SnapshotInfo> filtered;
    
    for (const SnapshotInfo &snapshot : allSnapshots) {
        if (snapshot.type == type) {
            filtered.append(snapshot);
        }
    }
    
    return filtered;
}

QList<SnapshotInfo> Database::getFavoriteSnapshots()
{
    QList<SnapshotInfo> allSnapshots = getAllSnapshots();
    QList<SnapshotInfo> favorites;
    
    for (const SnapshotInfo &snapshot : allSnapshots) {
        if (snapshot.isFavorite) {
            favorites.append(snapshot);
        }
    }
    
    return favorites;
}

SnapshotInfo Database::getSnapshotById(int id)
{
    SnapshotInfo snapshot;
    snapshot.id = -1;
    
    QJsonArray snapshotsArray = rootObject["snapshots"].toArray();
    
    for (const QJsonValue &val : snapshotsArray) {
        QJsonObject obj = val.toObject();
        if (obj["id"].toInt() == id) {
            snapshot = jsonToSnapshot(obj);
            break;
        }
    }
    
    return snapshot;
}

QList<SnapshotInfo> Database::searchSnapshots(const QString &keyword)
{
    QList<SnapshotInfo> snapshots;
    QList<SnapshotInfo> allSnapshots = getAllSnapshots();
    
    QString lowerKeyword = keyword.toLower();
    
    for (const SnapshotInfo &snapshot : allSnapshots) {
        if (snapshot.name.toLower().contains(lowerKeyword) ||
            snapshot.description.toLower().contains(lowerKeyword) ||
            snapshot.tags.toLower().contains(lowerKeyword) ||
            snapshot.path.toLower().contains(lowerKeyword)) {
            snapshots.append(snapshot);
        }
    }
    
    return snapshots;
}

QList<RemoteDesktopConnection> Database::getFavoriteRemoteDesktops()
{
    QList<RemoteDesktopConnection> connections;
    QJsonArray rdsArray = rootObject["remoteDesktops"].toArray();
    
    QList<QJsonObject> rdObjects;
    for (const QJsonValue &val : rdsArray) {
        QJsonObject obj = val.toObject();
        if (obj["isFavorite"].toBool()) {
            rdObjects.append(obj);
        }
    }
    
    std::sort(rdObjects.begin(), rdObjects.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a["sortOrder"].toInt() < b["sortOrder"].toInt();
    });
    
    for (const QJsonObject &obj : rdObjects) {
        connections.append(jsonToRemoteDesktop(obj));
    }
    
    return connections;
}

RemoteDesktopConnection Database::getRemoteDesktopById(int id)
{
    RemoteDesktopConnection conn;
    conn.id = -1;
    
    QJsonArray rdsArray = rootObject["remoteDesktops"].toArray();
    
    for (const QJsonValue &val : rdsArray) {
        QJsonObject obj = val.toObject();
        if (obj["id"].toInt() == id) {
            conn = jsonToRemoteDesktop(obj);
            break;
        }
    }
    
    return conn;
}

QList<RemoteDesktopConnection> Database::searchRemoteDesktops(const QString &keyword)
{
    QList<RemoteDesktopConnection> connections;
    QList<RemoteDesktopConnection> allConnections = getAllRemoteDesktops();
    
    QString lowerKeyword = keyword.toLower();
    
    for (const RemoteDesktopConnection &conn : allConnections) {
        if (conn.name.toLower().contains(lowerKeyword) ||
            conn.hostAddress.toLower().contains(lowerKeyword) ||
            conn.username.toLower().contains(lowerKeyword) ||
            conn.category.toLower().contains(lowerKeyword) ||
            conn.notes.toLower().contains(lowerKeyword)) {
            connections.append(conn);
        }
    }
    
    return connections;
}

QJsonObject Database::taskToJson(const Task &task)
{
    QJsonObject obj;
    obj["id"] = task.id;
    obj["title"] = task.title;
    obj["description"] = task.description;
    obj["categoryId"] = task.categoryId;
    obj["priority"] = static_cast<int>(task.priority);
    obj["status"] = static_cast<int>(task.status);
    obj["workDuration"] = task.workDuration;
    obj["finishTime"] = task.completionTime.toString(Qt::ISODate);

    QJsonArray tagsArray;
    for (const QString &tag : task.tags) {
        tagsArray.append(tag);
    }
    obj["tags"] = tagsArray;

    // 添加更新时间戳，用于增量同步
    obj["updatedAt"] = task.updatedAt.toString(Qt::ISODate);

    return obj;
}

Task Database::jsonToTask(const QJsonObject &obj)
{
    Task task;
    task.id = obj["id"].toString();
    task.title = obj["title"].toString();
    task.description = obj["description"].toString();
    task.categoryId = obj["categoryId"].toInt(-1);
    
    int priorityInt = obj["priority"].toInt(1);
    if (priorityInt < 0 || priorityInt > 2) priorityInt = 1;
    task.priority = static_cast<TaskPriority>(priorityInt);
    
    int statusInt = obj["status"].toInt(0);
    if (statusInt < 0 || statusInt > 3) statusInt = 0;
    task.status = static_cast<TaskStatus>(statusInt);
    
    task.workDuration = obj["workDuration"].toDouble(0.0);
    
    task.completionTime = QDateTime::fromString(obj["finishTime"].toString(), Qt::ISODate);
    
    QJsonArray tagsArray = obj["tags"].toArray();
    for (const QJsonValue &val : tagsArray) {
        task.tags.append(val.toString());
    }

    // 读取更新时间戳
    task.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
    if (!task.updatedAt.isValid()) {
        task.updatedAt = QDateTime::currentDateTime();
    }

    return task;
}

bool Database::addTask(const Task &task)
{
    Task newTask = task;
    newTask.id = generateTaskId();
    newTask.updatedAt = QDateTime::currentDateTime();

    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    tasksArray.append(taskToJson(newTask));
    taskRootObject["tasks"] = tasksArray;

    bool result = saveTaskData();
    if (result) {
        emit tasksChanged();
    }
    return result;
}

// 保留原有ID添加任务，用于同步
bool Database::addTaskWithId(const Task &task)
{
    Task newTask = task;
    newTask.updatedAt = QDateTime::currentDateTime();

    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    tasksArray.append(taskToJson(newTask));
    taskRootObject["tasks"] = tasksArray;

    bool result = saveTaskData();
    if (result) {
        emit tasksChanged();
    }
    return result;
}

bool Database::updateTask(const Task &task)
{
    Task updatedTask = task;
    updatedTask.updatedAt = QDateTime::currentDateTime();

    QJsonArray tasksArray = taskRootObject["tasks"].toArray();

    for (int i = 0; i < tasksArray.size(); ++i) {
        QJsonObject obj = tasksArray[i].toObject();
        if (obj["id"].toString() == task.id) {
            tasksArray[i] = taskToJson(updatedTask);
            break;
        }
    }

    taskRootObject["tasks"] = tasksArray;

    bool result = saveTaskData();
    if (result) {
        emit tasksChanged();
    }
    return result;
}

bool Database::deleteTask(const QString &id)
{
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (int i = 0; i < tasksArray.size(); ++i) {
        QJsonObject obj = tasksArray[i].toObject();
        if (obj["id"].toString() == id) {
            tasksArray.removeAt(i);
            break;
        }
    }
    
    taskRootObject["tasks"] = tasksArray;

    bool result = saveTaskData();
    if (result) {
        emit tasksChanged();
    }
    return result;
}

QList<Task> Database::getAllTasks()
{
    QList<Task> tasks;
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (const QJsonValue &val : tasksArray) {
        tasks.append(jsonToTask(val.toObject()));
    }
    
    std::sort(tasks.begin(), tasks.end(), [](const Task &a, const Task &b) {
        return a.id < b.id;
    });
    
    return tasks;
}

static QList<Task> filterTasksForDateInternal(const QList<Task> &allTasks, const QDate &viewDate)
{
    QList<Task> result;
    QDateTime dayStart(viewDate);
    QDateTime dayEnd(viewDate, QTime(23, 59, 59));

    for (const Task &task : allTasks) {
        if (task.id.length() < 8) {
            continue;
        }

        QString dateStr = task.id.left(8);
        QDate taskDate = QDate::fromString(dateStr, "yyyyMMdd");
        if (!taskDate.isValid()) {
            continue;
        }
        if (taskDate > viewDate) {
            continue;
        }

        // 对于已完成的任务，只显示在查看日期当天或之后完成的任务
        if (task.status == TaskStatus_Completed && task.completionTime.isValid()) {
            // 如果任务在查看日期之前就已完成，则不显示
            if (task.completionTime.date() < viewDate) {
                continue;
            }
        }

        result.append(task);
    }

    return result;
}

#ifdef QT_DEBUG
static void runTaskFilterTests()
{
    QList<Task> tasks;
    QDate viewDate(2026, 3, 4);
    QDate prevDate = viewDate.addDays(-1);
    QDate nextDate = viewDate.addDays(1);
    QDateTime dayStart(viewDate);
    
    Task prevIncomplete;
    prevIncomplete.id = prevDate.toString("yyyyMMdd") + "0001";
    prevIncomplete.status = TaskStatus_Todo;
    tasks.append(prevIncomplete);
    
    Task viewIncomplete;
    viewIncomplete.id = viewDate.toString("yyyyMMdd") + "0002";
    viewIncomplete.status = TaskStatus_InProgress;
    tasks.append(viewIncomplete);
    
    Task prevCompletedEarly;
    prevCompletedEarly.id = prevDate.toString("yyyyMMdd") + "0003";
    prevCompletedEarly.status = TaskStatus_Completed;
    prevCompletedEarly.completionTime = QDateTime(prevDate).addSecs(12 * 3600);
    tasks.append(prevCompletedEarly);
    
    Task prevCompletedAfter;
    prevCompletedAfter.id = prevDate.toString("yyyyMMdd") + "0004";
    prevCompletedAfter.status = TaskStatus_Completed;
    prevCompletedAfter.completionTime = QDateTime(viewDate).addSecs(3600);
    tasks.append(prevCompletedAfter);
    
    Task viewCompletedAtStart;
    viewCompletedAtStart.id = viewDate.toString("yyyyMMdd") + "0005";
    viewCompletedAtStart.status = TaskStatus_Completed;
    viewCompletedAtStart.completionTime = dayStart;
    tasks.append(viewCompletedAtStart);
    
    Task viewCompletedLater;
    viewCompletedLater.id = viewDate.toString("yyyyMMdd") + "0006";
    viewCompletedLater.status = TaskStatus_Completed;
    viewCompletedLater.completionTime = QDateTime(viewDate).addSecs(10 * 3600);
    tasks.append(viewCompletedLater);
    
    Task futureIncomplete;
    futureIncomplete.id = nextDate.toString("yyyyMMdd") + "0007";
    futureIncomplete.status = TaskStatus_Todo;
    tasks.append(futureIncomplete);
    
    Task completedNoTime;
    completedNoTime.id = viewDate.toString("yyyyMMdd") + "0008";
    completedNoTime.status = TaskStatus_Completed;
    tasks.append(completedNoTime);
    
    QList<Task> result = filterTasksForDateInternal(tasks, viewDate);
    
    bool hasPrevIncomplete = false;
    bool hasViewIncomplete = false;
    bool hasPrevCompletedAfter = false;
    bool hasViewCompletedLater = false;
    bool hasCompletedNoTime = false;
    
    bool hasPrevCompletedEarly = false;
    bool hasViewCompletedAtStart = false;
    bool hasFutureIncomplete = false;

    for (const Task &task : result) {
        if (task.id == prevIncomplete.id) hasPrevIncomplete = true;
        if (task.id == viewIncomplete.id) hasViewIncomplete = true;
        if (task.id == prevCompletedAfter.id) hasPrevCompletedAfter = true;
        if (task.id == viewCompletedLater.id) hasViewCompletedLater = true;
        if (task.id == completedNoTime.id) hasCompletedNoTime = true;
        if (task.id == prevCompletedEarly.id) hasPrevCompletedEarly = true;
        if (task.id == viewCompletedAtStart.id) hasViewCompletedAtStart = true;
        if (task.id == futureIncomplete.id) hasFutureIncomplete = true;
    }

    // 验证应该显示的任务
    Q_ASSERT(hasPrevIncomplete);
    Q_ASSERT(hasViewIncomplete);
    Q_ASSERT(hasPrevCompletedAfter);
    Q_ASSERT(hasViewCompletedLater);
    Q_ASSERT(hasCompletedNoTime);

    // 验证不应该显示的任务
    Q_ASSERT(!hasPrevCompletedEarly);  // 3月3日提前完成，不显示
    Q_ASSERT(!hasFutureIncomplete);    // 3月5日未完成，不显示

    // 3月4日0点完成的任务，根据新逻辑应该显示
    // 如果旧逻辑则不显示，现在新逻辑允许显示
}
#endif

QList<Task> Database::getTasksForDate(const QDate &date)
{
    QList<Task> allTasks = getAllTasks();
    return filterTasksForDateInternal(allTasks, date);
}

QList<Task> Database::getTasksByStatus(TaskStatus status)
{
    QList<Task> tasks;
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        if (obj["status"].toInt() == static_cast<int>(status)) {
            tasks.append(jsonToTask(obj));
        }
    }
    
    return tasks;
}

QList<Task> Database::getTasksByCategory(int categoryId)
{
    QList<Task> tasks;
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        if (obj["categoryId"].toInt() == categoryId) {
            tasks.append(jsonToTask(obj));
        }
    }
    
    return tasks;
}

QList<Task> Database::getTasksByDateRange(const QDateTime &startDate, const QDateTime &endDate)
{
    QList<Task> tasks;
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    QDate startDateObj = startDate.date();
    QDate endDateObj = endDate.date();
    
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        QString taskId = obj["id"].toString();
        
        if (taskId.length() >= 8) {
            QString dateStr = taskId.left(8);
            QDate taskDate = QDate::fromString(dateStr, "yyyyMMdd");
            
            if (taskDate.isValid() && taskDate >= startDateObj && taskDate <= endDateObj) {
                tasks.append(jsonToTask(obj));
            }
        }
    }
    
    return tasks;
}

QList<Task> Database::searchTasks(const QString &keyword)
{
    QList<Task> tasks;
    QList<Task> allTasks = getAllTasks();
    
    QString lowerKeyword = keyword.toLower();
    
    for (const Task &task : allTasks) {
        if (task.title.toLower().contains(lowerKeyword) ||
            task.description.toLower().contains(lowerKeyword)) {
            tasks.append(task);
        }
    }
    
    return tasks;
}

Task Database::getTaskById(const QString &id)
{
    Task task;
    task.id = "";
    
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        if (obj["id"].toString() == id) {
            task = jsonToTask(obj);
            break;
        }
    }
    
    return task;
}

bool Database::updateTaskStatus(const QString &id, TaskStatus status)
{
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (int i = 0; i < tasksArray.size(); ++i) {
        QJsonObject obj = tasksArray[i].toObject();
        if (obj["id"].toString() == id) {
            obj["status"] = static_cast<int>(status);
            if (status == TaskStatus_Completed) {
                obj["finishTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            } else {
                obj.remove("finishTime");
            }
            tasksArray[i] = obj;
            break;
        }
    }
    
    taskRootObject["tasks"] = tasksArray;

    bool result = saveTaskData();
    if (result) {
        emit tasksChanged();
    }
    return result;
}

bool Database::updateTaskDuration(const QString &id, double duration)
{
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (int i = 0; i < tasksArray.size(); ++i) {
        QJsonObject obj = tasksArray[i].toObject();
        if (obj["id"].toString() == id) {
            obj["workDuration"] = duration;
            tasksArray[i] = obj;
            break;
        }
    }
    
    taskRootObject["tasks"] = tasksArray;

    bool result = saveTaskData();
    if (result) {
        emit tasksChanged();
    }
    return result;
}

QList<Category> Database::getBuiltinCategories()
{
    QList<Category> categories;

    // 工作
    Category cat1;
    cat1.id = 1;
    cat1.name = "工作";
    cat1.description = "";
    cat1.parentId = -1;
    cat1.color = "#3498db";
    cat1.sortOrder = 0;
    categories.append(cat1);

    // 会议
    Category cat2;
    cat2.id = 2;
    cat2.name = "会议";
    cat2.description = "";
    cat2.parentId = -1;
    cat2.color = "#f39c12";
    cat2.sortOrder = 1;
    categories.append(cat2);

    // 个人
    Category cat3;
    cat3.id = 3;
    cat3.name = "个人";
    cat3.description = "";
    cat3.parentId = -1;
    cat3.color = "#9b59b6";
    cat3.sortOrder = 2;
    categories.append(cat3);

    // 其他
    Category cat4;
    cat4.id = 4;
    cat4.name = "其他";
    cat4.description = "";
    cat4.parentId = -1;
    cat4.color = "#95a5a6";
    cat4.sortOrder = 3;
    categories.append(cat4);

    return categories;
}

QHash<QString, double> Database::getCategoryWorkHours(const QDateTime &startDate, const QDateTime &endDate)
{
    QHash<QString, double> result;
    QList<Category> categories = getBuiltinCategories();
    for (const Category &cat : categories) {
        result[cat.name] = 0.0;
    }
    
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        QDateTime finishTime = QDateTime::fromString(obj["finishTime"].toString(), Qt::ISODate);
        if (finishTime.isValid() && finishTime >= startDate && finishTime <= endDate) {
            int categoryId = obj["categoryId"].toInt(-1);
            // 从内置分类中查找
            for (const Category &cat : getBuiltinCategories()) {
                if (cat.id == categoryId) {
                    result[cat.name] += obj["workDuration"].toDouble(0.0);
                    break;
                }
            }
        }
    }

    return result;
}

QHash<QString, int> Database::getCategoryTaskCount(const QDateTime &startDate, const QDateTime &endDate)
{
    QHash<QString, int> result;
    QList<Category> categories = getBuiltinCategories();
    for (const Category &cat : categories) {
        result[cat.name] = 0;
    }
    
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        QDateTime finishTime = QDateTime::fromString(obj["finishTime"].toString(), Qt::ISODate);
        if (finishTime.isValid() && finishTime >= startDate && finishTime <= endDate) {
            int categoryId = obj["categoryId"].toInt(-1);
            // 从内置分类中查找
            for (const Category &cat : getBuiltinCategories()) {
                if (cat.id == categoryId) {
                    result[cat.name]++;
                    break;
                }
            }
        }
    }

    return result;
}

double Database::getTotalWorkHours(const QDateTime &startDate, const QDateTime &endDate)
{
    double total = 0.0;
    
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        QDateTime finishTime = QDateTime::fromString(obj["finishTime"].toString(), Qt::ISODate);
        if (finishTime.isValid() && finishTime >= startDate && finishTime <= endDate) {
            total += obj["workDuration"].toDouble(0.0);
        }
    }
    
    return total;
}

int Database::getTotalTaskCount(const QDateTime &startDate, const QDateTime &endDate)
{
    int count = 0;
    
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        QDateTime finishTime = QDateTime::fromString(obj["finishTime"].toString(), Qt::ISODate);
        if (finishTime.isValid() && finishTime >= startDate && finishTime <= endDate) {
            count++;
        }
    }
    
    return count;
}

QString Database::generateTaskId()
{
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    int sequenceNumber = getDailyTaskCount(dateStr) + 1;
    
    QString taskId;
    do {
        QString sequenceStr = QString::number(sequenceNumber).rightJustified(4, '0');
        taskId = dateStr + sequenceStr;
        sequenceNumber++;
    } while (taskExists(taskId));
    
    return taskId;
}

bool Database::taskExists(const QString &taskId)
{
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();
    
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        if (obj["id"].toString() == taskId) {
            return true;
        }
    }
    
    return false;
}

int Database::getDailyTaskCount(const QString &dateStr)
{
    int count = 0;
    QJsonArray tasksArray = taskRootObject["tasks"].toArray();

    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        QString taskId = obj["id"].toString();
        if (taskId.startsWith(dateStr)) {
            count++;
        }
    }

    return count;
}

void Database::setCurrentUser(int userId)
{
    qDebug() << "[Database] setCurrentUser:" << currentUserId << "->" << userId;

    if (currentUserId == userId) {
        qDebug() << "[Database] User unchanged, skipping";
        return;
    }

    // 保存当前用户数据
    if (!taskFilePath.isEmpty()) {
        saveTaskData();
    }

    // 更新当前用户ID
    currentUserId = userId;

    // 更新任务文件路径
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (userId > 0) {
        taskFilePath = dataPath + QString("/user_%1/tasks.json").arg(userId);
        QDir dir(QFileInfo(taskFilePath).absolutePath());
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        qDebug() << "[Database] Logged in, using user task file:" << taskFilePath;
    } else {
        taskFilePath = dataPath + "/tasks.json";
        qDebug() << "[Database] Logged out, using default task file:" << taskFilePath;
    }

    // 清空当前数据并重新加载
    taskRootObject = QJsonObject();
    taskRootObject["tasks"] = QJsonArray();
    loadTaskData();

    // 发出信号通知UI刷新
    emit tasksChanged();
}

bool Database::loadTaskData()
{
    QFile file(taskFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        // 文件不存在，初始化为空数据
        taskRootObject = QJsonObject();
        taskRootObject["tasks"] = QJsonArray();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        taskRootObject = QJsonObject();
        taskRootObject["tasks"] = QJsonArray();
        return false;
    }

    taskRootObject = doc.object();

    if (!taskRootObject.contains("tasks")) {
        taskRootObject["tasks"] = QJsonArray();
    }

    return true;
}

bool Database::saveTaskData()
{
    QJsonDocument doc(taskRootObject);
    QFile file(taskFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Cannot save task file: %s", qPrintable(file.errorString()));
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool Database::migrateTaskData()
{
    // 分类现在是内置的，不再迁移
    // 只检查是否需要将 tasks 从 data.json 迁移到 tasks.json
    QFileInfo taskFileInfo(taskFilePath);
    if (taskFileInfo.exists()) {
        return false; // 迁移已完成
    }

    // 将任务数据迁移到 tasks.json
    QJsonArray tasksArray = rootObject["tasks"].toArray();

    taskRootObject = QJsonObject();
    taskRootObject["tasks"] = tasksArray;

    // 保存任务文件
    saveTaskData();

    // 从 data.json 中移除 tasks
    rootObject["tasks"] = QJsonArray();

    // 保存 data.json
    saveData();

    return true;
}
