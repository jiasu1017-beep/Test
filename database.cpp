#include "database.h"
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <algorithm>

Database::Database(QObject *parent) : QObject(parent)
{
    nextAppId = 1;
    nextCollectionId = 1;
    nextRemoteDesktopId = 1;
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
    
    if (!loadData()) {
        rootObject = QJsonObject();
        rootObject["apps"] = QJsonArray();
        rootObject["collections"] = QJsonArray();
        rootObject["remoteDesktops"] = QJsonArray();
        rootObject["settings"] = QJsonObject();
        rootObject["nextAppId"] = 1;
        rootObject["nextCollectionId"] = 1;
        rootObject["nextRemoteDesktopId"] = 1;
        saveData();
    }
    
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
    
    return true;
}

bool Database::saveData()
{
    rootObject["nextAppId"] = nextAppId;
    rootObject["nextCollectionId"] = nextCollectionId;
    rootObject["nextRemoteDesktopId"] = nextRemoteDesktopId;
    
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
    obj["isRemoteDesktop"] = app.isRemoteDesktop;
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
    app.isRemoteDesktop = obj["isRemoteDesktop"].toBool(false);
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
    
    return saveData();
}

bool Database::updateApp(const AppInfo &app)
{
    QJsonArray appsArray = rootObject["apps"].toArray();
    
    for (int i = 0; i < appsArray.size(); ++i) {
        QJsonObject obj = appsArray[i].toObject();
        if (obj["id"].toInt() == app.id) {
            appsArray[i] = appToJson(app);
            break;
        }
    }
    
    rootObject["apps"] = appsArray;
    
    return saveData();
}

bool Database::deleteApp(int id)
{
    QJsonArray appsArray = rootObject["apps"].toArray();
    
    for (int i = 0; i < appsArray.size(); ++i) {
        QJsonObject obj = appsArray[i].toObject();
        if (obj["id"].toInt() == id) {
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
    
    return saveData();
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
