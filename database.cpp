#include "database.h"
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QCoreApplication>

Database::Database(QObject *parent) : QObject(parent)
{
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::init()
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dbPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath + "/officeassistant.db");
    
    if (!db.open()) {
        qWarning("Cannot open database: %s", qPrintable(db.lastError().text()));
        return false;
    }
    
    return createTables();
}

bool Database::createTables()
{
    QSqlQuery query;
    
    if (!query.exec("CREATE TABLE IF NOT EXISTS apps ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "name TEXT NOT NULL,"
                   "path TEXT NOT NULL,"
                   "arguments TEXT,"
                   "icon_path TEXT,"
                   "category TEXT,"
                   "use_count INTEGER DEFAULT 0,"
                   "is_favorite INTEGER DEFAULT 0,"
                   "sort_order INTEGER DEFAULT 0)")) {
        qWarning("Error creating apps table: %s", qPrintable(query.lastError().text()));
        return false;
    }
    
    query.exec("PRAGMA table_info(apps)");
    bool hasArgumentsColumn = false;
    bool hasSortOrderColumn = false;
    while (query.next()) {
        if (query.value(1).toString() == "arguments") {
            hasArgumentsColumn = true;
        }
        if (query.value(1).toString() == "sort_order") {
            hasSortOrderColumn = true;
        }
    }
    if (!hasArgumentsColumn) {
        query.exec("ALTER TABLE apps ADD COLUMN arguments TEXT");
    }
    if (!hasSortOrderColumn) {
        query.exec("ALTER TABLE apps ADD COLUMN sort_order INTEGER DEFAULT 0");
    }
    
    if (!query.exec("CREATE TABLE IF NOT EXISTS collections ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "name TEXT NOT NULL,"
                   "app_ids TEXT)")) {
        qWarning("Error creating collections table: %s", qPrintable(query.lastError().text()));
        return false;
    }
    
    if (!query.exec("CREATE TABLE IF NOT EXISTS settings ("
                   "key TEXT PRIMARY KEY,"
                   "value TEXT)")) {
        qWarning("Error creating settings table: %s", qPrintable(query.lastError().text()));
        return false;
    }
    
    return true;
}

bool Database::addApp(const AppInfo &app)
{
    QSqlQuery query;
    query.prepare("INSERT INTO apps (name, path, arguments, icon_path, category, use_count, is_favorite, sort_order) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(app.name);
    query.addBindValue(app.path);
    query.addBindValue(app.arguments);
    query.addBindValue(app.iconPath);
    query.addBindValue(app.category);
    query.addBindValue(app.useCount);
    query.addBindValue(app.isFavorite ? 1 : 0);
    query.addBindValue(app.sortOrder);
    
    return query.exec();
}

bool Database::updateApp(const AppInfo &app)
{
    QSqlQuery query;
    query.prepare("UPDATE apps SET name=?, path=?, arguments=?, icon_path=?, category=?, use_count=?, is_favorite=?, sort_order=? "
                  "WHERE id=?");
    query.addBindValue(app.name);
    query.addBindValue(app.path);
    query.addBindValue(app.arguments);
    query.addBindValue(app.iconPath);
    query.addBindValue(app.category);
    query.addBindValue(app.useCount);
    query.addBindValue(app.isFavorite ? 1 : 0);
    query.addBindValue(app.sortOrder);
    query.addBindValue(app.id);
    
    return query.exec();
}

bool Database::deleteApp(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM apps WHERE id=?");
    query.addBindValue(id);
    return query.exec();
}

QList<AppInfo> Database::getAllApps()
{
    QList<AppInfo> apps;
    QSqlQuery query("SELECT * FROM apps ORDER BY sort_order ASC, id ASC");
    
    while (query.next()) {
        AppInfo app;
        app.id = query.value(0).toInt();
        app.name = query.value(1).toString();
        app.path = query.value(2).toString();
        app.arguments = query.value(3).toString();
        app.iconPath = query.value(4).toString();
        app.category = query.value(5).toString();
        app.useCount = query.value(6).toInt();
        app.isFavorite = query.value(7).toInt() == 1;
        app.sortOrder = query.value(8).toInt();
        apps.append(app);
    }
    
    return apps;
}

QList<AppInfo> Database::getFavoriteApps()
{
    QList<AppInfo> apps;
    QSqlQuery query("SELECT * FROM apps WHERE is_favorite=1 ORDER BY sort_order ASC, id ASC");
    
    while (query.next()) {
        AppInfo app;
        app.id = query.value(0).toInt();
        app.name = query.value(1).toString();
        app.path = query.value(2).toString();
        app.arguments = query.value(3).toString();
        app.iconPath = query.value(4).toString();
        app.category = query.value(5).toString();
        app.useCount = query.value(6).toInt();
        app.isFavorite = query.value(7).toInt() == 1;
        app.sortOrder = query.value(8).toInt();
        apps.append(app);
    }
    
    return apps;
}

AppInfo Database::getAppById(int id)
{
    AppInfo app;
    QSqlQuery query;
    query.prepare("SELECT * FROM apps WHERE id=?");
    query.addBindValue(id);
    
    if (query.exec() && query.next()) {
        app.id = query.value(0).toInt();
        app.name = query.value(1).toString();
        app.path = query.value(2).toString();
        app.arguments = query.value(3).toString();
        app.iconPath = query.value(4).toString();
        app.category = query.value(5).toString();
        app.useCount = query.value(6).toInt();
        app.isFavorite = query.value(7).toInt() == 1;
        app.sortOrder = query.value(8).toInt();
    }
    
    return app;
}

bool Database::addCollection(const AppCollection &collection)
{
    QSqlQuery query;
    QString appIdsStr;
    for (int id : collection.appIds) {
        appIdsStr += QString::number(id) + ",";
    }
    if (!appIdsStr.isEmpty()) {
        appIdsStr.chop(1);
    }
    
    query.prepare("INSERT INTO collections (name, app_ids) VALUES (?, ?)");
    query.addBindValue(collection.name);
    query.addBindValue(appIdsStr);
    
    return query.exec();
}

bool Database::updateCollection(const AppCollection &collection)
{
    QSqlQuery query;
    QString appIdsStr;
    for (int id : collection.appIds) {
        appIdsStr += QString::number(id) + ",";
    }
    if (!appIdsStr.isEmpty()) {
        appIdsStr.chop(1);
    }
    
    query.prepare("UPDATE collections SET name=?, app_ids=? WHERE id=?");
    query.addBindValue(collection.name);
    query.addBindValue(appIdsStr);
    query.addBindValue(collection.id);
    
    return query.exec();
}

bool Database::deleteCollection(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM collections WHERE id=?");
    query.addBindValue(id);
    return query.exec();
}

QList<AppCollection> Database::getAllCollections()
{
    QList<AppCollection> collections;
    QSqlQuery query("SELECT * FROM collections");
    
    while (query.next()) {
        AppCollection col;
        col.id = query.value(0).toInt();
        col.name = query.value(1).toString();
        QString appIdsStr = query.value(2).toString();
        if (!appIdsStr.isEmpty()) {
            QStringList idList = appIdsStr.split(",");
            for (QString idStr : idList) {
                col.appIds.append(idStr.toInt());
            }
        }
        collections.append(col);
    }
    
    return collections;
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
    
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)");
    query.addBindValue("auto_start");
    query.addBindValue(enabled ? "1" : "0");
    query.exec();
    
    return true;
}

bool Database::getAutoStart()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                       QSettings::NativeFormat);
    return settings.contains("OfficeAssistant");
}
