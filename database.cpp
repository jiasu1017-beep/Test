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
        rootObject["settings"] = QJsonObject();
        rootObject["nextAppId"] = 1;
        rootObject["nextCollectionId"] = 1;
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
    
    return true;
}

bool Database::saveData()
{
    rootObject["nextAppId"] = nextAppId;
    rootObject["nextCollectionId"] = nextCollectionId;
    
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
    return app;
}

QJsonObject Database::collectionToJson(const AppCollection &collection)
{
    QJsonObject obj;
    obj["id"] = collection.id;
    obj["name"] = collection.name;
    
    QJsonArray idArray;
    for (int id : collection.appIds) {
        idArray.append(id);
    }
    obj["appIds"] = idArray;
    return obj;
}

AppCollection Database::jsonToCollection(const QJsonObject &obj)
{
    AppCollection col;
    col.id = obj["id"].toInt();
    col.name = obj["name"].toString();
    
    QJsonArray idArray = obj["appIds"].toArray();
    for (const QJsonValue &val : idArray) {
        col.appIds.append(val.toInt());
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

bool Database::setAutoStart(bool enabled)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                       QSettings::NativeFormat);
    
    if (enabled) {
        settings.setValue("OfficeAssistant", QCoreApplication::applicationFilePath().replace("/", "\\"));
    } else {
        settings.remove("OfficeAssistant");
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
    return settings.contains("OfficeAssistant");
}
